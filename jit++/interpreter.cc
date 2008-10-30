
#include <asm/prctl.h>        // SYS_arch_prctl
#include <sys/syscall.h>      // syscall

#include <dlfcn.h>

#include <udis86.h>

#include <jit++/common.h>
#include <jit++/interpreter_internal.h>
#include <jit++/exceptions.h>

DEFINE_uint64(jitpp_steps,-1,"the number of steps to trace before quitting");

    

namespace jitpp { 
    int64_t interpreter::repetitions() const {
        if (has_repxx_prefix()) return 1;
        else if (address_size_is_64()) return ecx();
        else return rcx();
    }

    int64_t interpreter::mem(bool add_segment_base) const {
        int64_t addr = disp;
	// VLOG(5) << "disp " << std::hex << addr;

	if (add_segment_base) 
	    addr += seg_base();

	if (address_size_is_64()) { 
	    // VLOG(5) << "64 bit address";
            if (has_sib()) {
	        // VLOG(1) << "with sib";
                if (((base & 7) != 5) || (mod != 0)) { 
                    addr += get_reg<int64_t>(*this,base);
	            // VLOG(1) << "addr w/ base " << os64::reg_name(base) << " = " << std::hex << addr;
		}
                if (index != 4) {
                    addr += get_reg<int64_t>(*this,index) << log_scale;
	            // VLOG(1) << "addr w/ " << os64::reg_name(index) << " * " << scale() << " = " << std::hex << addr;
		}
            } else if (is_rip_relative()) {
		addr += rip();
	        // VLOG(1) << "addr w/ rip (" << std::hex << rip() << ") = " << std::hex << addr;
	    } else addr += get_reg<int64_t>(*this,rm);
	} else { 
	    // VLOG(1) << "32 bit address";
	    if (has_sib()) { 
                if (((base & 7) != 5) || mod != 0) { 
                    addr += get_reg<int32_t>(*this,base);
		}
                if (index != 4) {
                    addr += get_reg<int32_t>(*this,index) << log_scale;
		}
	    } else if (is_rip_relative()) {
		addr += eip();
	    } else addr += get_reg<int32_t>(*this,rm);
	}
	// VLOG(1) << "addr = " << std::hex << addr;
        return addr;
    }
int64_t interpreter::seg_base() const { 
    if (seg_prefix == 0x64) return fs_base();
    else if (seg_prefix == 0x65) return gs_base();
    else return 0;
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
	    << (zf() ? " ZF" : "")
	    << (df() ? " DF" : "");
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

void print_address(int64_t addr) { 
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(addr),&info)) { 
	VLOG(2) << info.dli_sname << "+" << (addr - reinterpret_cast<int64_t>(info.dli_saddr)) << " from " << info.dli_fname;
    } else {
        VLOG(2) << "unresolved address " << std::hex << addr;
    }
}

// assumes x86-64 long mode 
// TODO: check for correct handling of 66h on e0-e3,70-7f,eb,e9,ff/4,e8,ff/2,c3,c2
void interpreter::run() { 
    m_fs_base_known = m_gs_base_known = false;
    m_stopped = false;
    int64_t steps = FLAGS_jitpp_steps;
    while (steps-- && !m_stopped) { 
	if (VLOG_IS_ON(2))
	    print_regs();

	int64_t old_rip = rip();
	try { 
	    rip() = parse(old_rip);
	    if (VLOG_IS_ON(1)) {
		print_address(rip());
	        print_opcode(old_rip,rip() - old_rip);
	    }

            if (has_lock_prefix()) { 
                switch (log_v) { 
                case 1: interpret_locked_opcode_16(*this); break;
                case 2: interpret_locked_opcode_32(*this); break;
                case 3: interpret_locked_opcode_64(*this); break;
                default: logic_error(); break;
                }
            } else {
                switch (log_v) {
                case 1: interpret_opcode_16(*this); break;
                case 2: interpret_opcode_32(*this); break;
                case 3: interpret_opcode_64(*this); break;
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
    if (!m_stopped) { 
        VLOG(1) << "Completed " << FLAGS_jitpp_steps << " steps.";
        print_regs();
        print_opcode(m_rip);
    }
}

} // namespace jitpp

