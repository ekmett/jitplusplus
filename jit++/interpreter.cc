#include <jit++/common.h>
#include <jit++/interpreter.h>
#include <jit++/exceptions.h>
#include <asm/prctl.h>        // SYS_arch_prctl
#include <sys/syscall.h>      // syscall
#include <udis86.h>

namespace { 
    const char * os64_reg_names[16] = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
    const char * os32_reg_names[16] = {"eax","ecx","edx","ebx","esp","ebp","esi","edi","r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"};
    const char * os16_reg_names[16] = {"ax","cx","dx","bx","sp","bp","si","di","r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w"};
    const char * byte_reg_names_rex[16] = {"al","cl","dl","bl","spl","bpl","sil","dil","r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"};
    const char * byte_reg_names_norex[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};
}

namespace jitpp { 
    const char * os64::reg_name(int r) { return os64_reg_names[r]; }
    const char * os32::reg_name(int r) { return os32_reg_names[r]; }
    const char * os16::reg_name(int r) { return os16_reg_names[r]; }
    const char * byte_reg_name (int r, bool has_rex) { 
        return has_rex ? byte_reg_names_rex[r] : byte_reg_names_norex[r];
    }

    int64_t interpreter::mem(bool add_segment_base) const {
        int64_t addr = op.disp;
	// VLOG(5) << "disp " << std::hex << addr;

	if (add_segment_base) { 
            if (op.seg_prefix == 0x64)
                addr += fs_base();
            else if (op.seg_prefix == 0x65)
                addr += gs_base();
	    // VLOG(5) << "addr w/ segment base " << std::hex << addr;
	}

	if (op.address_size_is_64()) { 
	    // VLOG(5) << "64 bit address";
            if (op.has_sib()) {
	        // VLOG(1) << "with sib";
                if (op.base != 5) { 
                    addr += reg<int64_t>(op.base);
	            // VLOG(1) << "addr w/ base " << os64::reg_name(op.base) << " = " << std::hex << addr;
		}
                if (op.index != 4) {
                    addr += reg<int64_t>(op.index) << op.log_scale;
	            // VLOG(1) << "addr w/ " << os64::reg_name(op.index) << " * " << op.scale() << " = " << std::hex << addr;
		}
            } else if (op.is_rip_relative()) {
		addr += rip();
	        // VLOG(1) << "addr w/ rip (" << std::hex << rip() << ") = " << std::hex << addr;
	    } else addr += reg<int64_t>(op.rm);
	} else { 
	    // VLOG(1) << "32 bit address";
	    if (op.has_sib()) { 
                if (op.base != 5) { 
                    addr += reg<int32_t>(op.base);
		}
                if (op.index != 4) {
                    addr += reg<int32_t>(op.index) << op.log_scale;
		}
	    } else if (op.is_rip_relative()) {
		addr += eip();
	    } else addr += reg<int32_t>(op.rm);
	}
	// VLOG(1) << "addr = " << std::hex << addr;
        return addr;
    }


// operand sizes to template interpret_operand below

int64_t interpreter::fs_base() const {
    if (unlikely(!m_fs_base_known)) {
        syscall(SYS_arch_prctl,ARCH_GET_FS,&m_fs_base);
        m_fs_base_known = true;
    }
    return m_fs_base;
}

int64_t interpreter::gs_base() const {
    if (unlikely(!m_gs_base_known)) {
        syscall(SYS_arch_prctl,ARCH_GET_FS,&m_gs_base);
        m_gs_base_known = true;
    }
    return m_gs_base;
}

void interpreter::print_regs() {
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
	    << (zf() ? " ZF" : "");
}

void interpreter::print_opcode(int64_t rip, int expected) { 
    using namespace std;
    ud_t ud;
    ud_init(&ud);
    ud_set_mode(&ud,64);
    ud_set_syntax(&ud,UD_SYN_ATT);
    ud_set_input_buffer(&ud,reinterpret_cast<uint8_t*>(rip),15);
    ud_set_pc(&ud,rip);
    size_t bytes = ud_disassemble(&ud);
    VLOG(0)
	    << noshowbase << setfill('0') << hex << setw(16) << ud_insn_off(&ud) 
            << ": " << setfill(' ') << setw(32) << ud_insn_hex(&ud)
	    << " " << ud_insn_asm(&ud);
    LOG_IF(WARNING, (bytes != expected) && (expected != 0))
	    << "udis86 and jit++ disagree on opcode size! (udis86: " << bytes << " vs. jit++: " << expected << ")";
}

// assumes x86-64 long mode 
// TODO: check for correct handling of 66h on e0-e3,70-7f,eb,e9,ff/4,e8,ff/2,c3,c2
void interpreter::run() { 
    m_fs_base_known = m_gs_base_known = false;
    while (true) { 
	if (VLOG_IS_ON(2))
	    print_regs();

	int64_t old_rip = rip();
	try { 
	    rip() = op.parse(old_rip);
	    if (VLOG_IS_ON(1)) 
	        print_opcode(old_rip,rip() - old_rip);

            if (op.has_lock_prefix()) { 
                switch (op.log_v) { 
                case 1: interpret_locked_opcode_16(); break;
                case 2: interpret_locked_opcode_32(); break;
                case 3: interpret_locked_opcode_64(); break;
                default: logic_error(); break;
                }
            } else {
                switch (op.log_v) {
                case 1: interpret_opcode_16(); break;
                case 2: interpret_opcode_32(); break;
                case 3: interpret_opcode_64(); break;
                default: logic_error(); break;
                }
            }
	} catch (interpreter_exception & e) { 
	    if (e.fatal()) LOG(FATAL) << e.what();
	    else LOG(WARNING) << e.what();
	    m_rip = old_rip; // restore rip to before the parsing attempt
	    rflags();        // force commonuation of any lazy rflags
	    // show the current intruction if we aren't logging it already
	    print_regs();
	    print_opcode(m_rip);
	    return;	     // stop interpreting and return
	}
    }
}

void interpreter::illegal() { 
    throw invalid_opcode_exception();
}
void interpreter::unsupported() { 
    throw unsupported_opcode_exception();
}
void interpreter::uninterpretable() { 
    throw uninterpretable_opcode_exception();
}
void interpreter::logic_error() { 
    throw interpreter_logic_error();
}

 
} // namespace jitpp

