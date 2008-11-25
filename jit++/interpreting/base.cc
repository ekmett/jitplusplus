#include <asm/prctl.h>   // SYS_arch_prctl
#include <sys/syscall.h> // syscall

#include <dlfcn.h>  // dladdr
#include <udis86.h> // ud_t, ud_*

#include <jit++/common.h>
#include <jit++/exceptions.h>
#include <jit++/interpreting/base.h>

namespace jitpp { namespace interpreting { 

    static const char * os64_reg_names[16] = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
    static const char * os32_reg_names[16] = {"eax","ecx","edx","ebx","esp","ebp","esi","edi","r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"};
    static const char * os16_reg_names[16] = {"ax","cx","dx","bx","sp","bp","si","di","r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w"};
    static const char * byte_reg_names_rex[16] = {"al","cl","dl","bl","spl","bpl","sil","dil","r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"};
    static const char * byte_reg_names_norex[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};

    const char * os64::reg_name(int r) { return os64_reg_names[r]; }
    const char * os32::reg_name(int r) { return os32_reg_names[r]; }
    const char * os16::reg_name(int r) { return os16_reg_names[r]; }
    const char * byte_reg_name (int r, bool has_rex) { 
        return has_rex ? byte_reg_names_rex[r] : byte_reg_names_norex[r];
    }

    void illegal() { throw invalid_opcode_exception(); }
    void unsupported() { throw unsupported_opcode_exception(); }
    void uninterpretable() { throw uninterpretable_opcode_exception(); }
    void logic_error() { throw interpreter_logic_error(); }

    int64_t interpreter_base::repetitions() const {
        if (has_repxx_prefix()) return 1;
        else if (address_size_is_64()) return ecx();
        else return rcx();
    }

    int64_t interpreter_base::mem(bool add_segment_base) const {
        int64_t addr = disp;
	// VLOG(5) << "disp " << std::hex << addr;

	if (add_segment_base) 
	    addr += seg_base();

	if (address_size_is_64()) { 
	    // VLOG(5) << "64 bit address";
            if (has_sib()) {
	        // VLOG(1) << "with sib";
                if (((base & 7) != 5) || (mod != 0)) { 
                    addr += get_reg<int64_t>(base);
	            // VLOG(1) << "addr w/ base " << os64::reg_name(base) << " = " << std::hex << addr;
		}
                if (index != 4) {
                    addr += get_reg<int64_t>(index) << log_scale;
	            // VLOG(1) << "addr w/ " << os64::reg_name(index) << " * " << scale() << " = " << std::hex << addr;
		}
            } else if (is_rip_relative()) {
		addr += rip();
	        // VLOG(1) << "addr w/ rip (" << std::hex << rip() << ") = " << std::hex << addr;
	    } else addr += get_reg<int64_t>(rm);
	} else { 
	    // VLOG(1) << "32 bit address";
	    if (has_sib()) { 
                if (((base & 7) != 5) || mod != 0) { 
                    addr += get_reg<int32_t>(base);
		}
                if (index != 4) {
                    addr += get_reg<int32_t>(index) << log_scale;
		}
	    } else if (is_rip_relative()) {
		addr += eip();
	    } else addr += get_reg<int32_t>(rm);
	}
	// VLOG(1) << "addr = " << std::hex << addr;
        return addr;
    }
int64_t interpreter_base::seg_base() const { 
    if (seg_prefix == 0x64) return fs_base();
    else if (seg_prefix == 0x65) return gs_base();
    else return 0;
}


// operand sizes to template interpret_operand below

int64_t interpreter_base::fs_base() const {
    if (unlikely(!m_fs_base_known)) {
        syscall(SYS_arch_prctl,ARCH_GET_FS,&m_fs_base);
        m_fs_base_known = true;
    }
    return m_fs_base;
}

int64_t interpreter_base::gs_base() const {
    if (unlikely(!m_gs_base_known)) {
        syscall(SYS_arch_prctl,ARCH_GET_FS,&m_gs_base);
        m_gs_base_known = true;
    }
    return m_gs_base;
}

void interpreter_base::print_regs() {
    using namespace std;
    for (int i=0;i<16;i++)
	VLOG(0) << os64::reg_name(i) << " " << hex << m_reg[i];

    VLOG(0) << "rip " << hex << m_rip;
    VLOG(0) << "rflags" 
	    << (cf() ? " CF" : "") 
	    << (sf() ? " SF" : "")
	    << (of() ? " OF" : "")
	    << (pf() ? " PF" : "")
	    << (af() ? " AF" : "")
	    << (zf() ? " ZF" : "")
	    << (df() ? " DF" : "");
}

void interpreter_base::print_opcode(int64_t rip, int expected) { 
    using namespace std;
    ud_t ud;
    ud_init(&ud);
    ud_set_mode(&ud,64);
    ud_set_syntax(&ud,UD_SYN_ATT);
    ud_set_input_buffer(&ud,reinterpret_cast<uint8_t*>(rip),15);
    ud_set_pc(&ud,rip);
    size_t bytes = ud_disassemble(&ud);
    char buf[24];
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(rip),&info)) { 
        snprintf(buf,24,"%s+%lx",info.dli_sname, rip - reinterpret_cast<int64_t>(info.dli_saddr));
    } else { 
	snprintf(buf,24,"%lx",rip);
    }
    VLOG(0)
	<< noshowbase << hex << setfill(' ') << setw(24) << buf
	<< ": " << setw(32) << left << ud_insn_asm(&ud) << " : " << setfill(' ') << ud_insn_hex(&ud);

    LOG_IF(WARNING, (bytes != expected) && (expected != 0))
	<< "udis86 and jit++ disagree on opcode size! (udis86: " << bytes << " vs. jit++: " << expected << ")";
}

void interpreter_base::print_address(int64_t addr) { 
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(addr),&info)) { 
	VLOG(2) << info.dli_sname << "+" << (addr - reinterpret_cast<int64_t>(info.dli_saddr)) << " from " << info.dli_fname;
    } else {
        VLOG(2) << "unresolved address " << std::hex << addr;
    }
}

} } // namespace jitpp::interpreting

