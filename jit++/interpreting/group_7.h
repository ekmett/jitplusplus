#ifndef INCLUDED_JITPP_INTERPRETING_GROUP_7_H
#define INCLUDED_JITPP_INTERPRETING_GROUP_7_H

#include <jit++/common.h>
#include <jit++/interpreting/base.h>

namespace jitpp { 
  namespace interpreting { 
    class group_7 : public virtual interpreter_base {
    public:
        // output TSC into EDX:EAX and TSC_AUX into ECX
        void rdtscp() {
            int64_t RAX, RCX, RDX;
            __asm__ __volatile__("rdtsc" : "=a"(RAX), "=d"(RDX), "=c"(RCX) :: "memory");
            rax() = RAX;
            rcx() = RCX;
            rdx() = RDX;
        }
	// monitor a linear region of memory starting at RAX for changes
	inline void monitor() { 
	    int64_t RAX = rax();
	    int64_t RCX = rcx();
	    int64_t RDX = rdx();
	    __asm__ __volatile__("monitor" :: "a"(RAX), "c"(RCX), "d"(RDX) : "memory");
	}
	// wait for the changes we are monitoring to occur
	inline void mwait() { 
	    int64_t RAX = rax();
	    int64_t RCX = rcx();
	    __asm__ __volatile__("mwait" :: "a"(RAX), "c"(RCX) : "memory");
	}
	uint64_t smsw() { 
	    uint64_t result;
	    __asm__ ("smsw %0" : "=q"(result) :: "memory");
	    return result;
	}
	template <typename os>
	inline void interpret_group_7() { 
	    if (mod != 3) {
		// memory operand
		switch (reg) { 
		case 0: uninterpretable(); // SGDT Ms
	        case 1: uninterpretable(); // SIDT Ms
		case 2: uninterpretable(); // LDGT Ms
		case 3: uninterpretable(); // LIDT Ms
		case 4: M<uint16_t>(smsw()); // SMSW Mw -- this we CAN execute!
		case 5: illegal(); 
		case 6: uninterpretable(); // LMSW Mw
		case 7: uninterpretable(); // INVLPG Mb
		default: logic_error();
		}
	    } else { 
		switch (reg) { 
		case 0:
		    switch (rm) { 
		    case 1 ... 4: uninterpretable(); // VMCALL, VMLAUNCH, VMRESUME, VMXOFF
		    default: illegal();
		    }
		case 1:
		    switch (rm) { 
		    case 0: monitor(); return; // MONITOR
		    case 1: mwait(); return; // MWAIT
		    default: illegal();
		    }
		case 2:
		    switch (rm) { 
		    case 0 ... 1: unsupported(); // XGETBV, XSETBV
		    default: illegal();
		    }
		case 3: uninterpretable(); // VMRUN, VMMCALL, VMLOAD, VMSAVE, STGI, CLGI, SKINIT, INVLPGA
		case 4: R<typename os::v>(smsw()); return; // SMSW Rv
		case 5: illegal();
	        case 6: uninterpretable(); // LMSW Rw 
		case 7:
		    switch (rm) { 
		    case 0: uninterpretable(); // SWAPGS
		    case 1: rdtscp(); return;
		    default: illegal();
		    }
		}
	    }
	} // interpret_group_7

    }; // group 7
  } // interpreting
} // jitpp

#endif
