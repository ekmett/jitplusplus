#ifndef INCLUDED_JITPP_INTERPRETING_GROUP_5_H
#define INCLUDED_JITPP_INTERPRETING_GROUP_5_H

#include <jit++/common.h>
#include <jit++/interpreting/base.h>
#include <jit++/interpreting/group_4.h>

namespace jitpp { 
  namespace interpreting { 
    class group_5 : private virtual group_4, public virtual interpreter_base {
    public:
	template <typename T>
	inline void interpret_group_5() { 
            switch (reg) {
            case 0: E<T>(inc<T>(E<T>())); return; // INC Ev
            case 1: E<T>(dec<T>(E<T>())); return; // DEC Ev
            case 2: push<T>(rip()); rip() = E<T>(); return; // CALL Ev
            case 3: unsupported();
            case 4: rip() = E<T>(); return; // JMP Ev
            case 5: unsupported();
            case 6: push<T>(E<T>()); return; // PUSH Ev
            case 7: illegal();
            default: unsupported();
            }
	} // interpret_group_5
    }; // group 5
  } // interpreting
} // jitpp

#endif
