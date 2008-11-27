#ifndef INCLUDED_JITPP_INTERPRETING_GROUP_6_H
#define INCLUDED_JITPP_INTERPRETING_GROUP_6_H

#include <jit++/common.h>
#include <jit++/interpreting/base.h>

namespace jitpp { 
  namespace interpreting { 
    class group_6 : public virtual interpreter_base {
    public:
	inline uint16_t str() { 
	    uint16_t result;
	    asm ("str %0" : "=q"(result) :: "memory");
	    return result;
	}
	inline void verr(int16_t selector) { 
	    int8_t zero;
	    asm ("verr %1; setz %0" : "=q"(zero) : "q"(selector) : "memory");
	    zf(zero != 0);
	}
	inline void verw(int16_t selector) { 
	    int8_t zero;
	    asm ("verw %1; setz %0" : "=q"(zero) : "q"(selector) : "memory");
	    zf(zero != 0);
	}
	template <typename os>
	inline void interpret_group_6() { 
	    // memory operand
	    switch (reg) { 
	    case 0: uninterpretable(); // SLDT Mw/Rv
	    case 1: // STR Mw/Rv
	        if (mod == 3) R<typename os::v>(str()); // zero extended
		else M<int16_t>(str());
	    case 2: uninterpretable(); // LLDT Mw/Rv
	    case 3: uninterpretable(); // LTR Mw/Rv
	    case 4: verr(E<int16_t>()); // VERR Mw/Rv
	    case 5: verw(E<int16_t>()); // VERW Mw/Rv
	    case 6: uninterpretable(); // JMPE Ev (Itanium only)
	    case 7: illegal(); 
	    default: logic_error();
	    }
	} // interpret_group_6
    }; // group_6
  } // interpreting
} // jitpp

#endif
