#include <jit++/common.h>
#include <jit++/exceptions.h>
#include <jit++/interpreting/base.h>
#include <jit++/interpreting/impl.h>

DEFINE_uint64(jitpp_steps,-1,"the number of steps to trace before quitting");

namespace jitpp { 

using namespace jitpp::interpreting;

void interpreter_impl::run() { 
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
                case 1: interpret_locked_opcode<os16>(); break;
                case 2: interpret_locked_opcode<os32>(); break;
                case 3: interpret_locked_opcode<os64>(); break;
                default: logic_error(); break;
                }
            } else {
                switch (log_v) {
                case 1: interpret_opcode<os16>(); break;
                case 2: interpret_opcode<os32>(); break;
                case 3: interpret_opcode<os64>(); break;
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
