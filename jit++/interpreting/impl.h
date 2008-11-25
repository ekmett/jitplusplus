#ifndef INCLUDED_JITPP_INTERPRETING_IMPL_H
#define INCLUDED_JITPP_INTERPRETING_IMPL_H

#include <jit++/common.h>
#include <jit++/interpreting/locked.h>
#include <jit++/interpreting/opcode.h>

namespace jitpp { 
    class interpreter_impl 
      : public jitpp::interpreting::locked_interpreter, 
	public jitpp::interpreting::opcode_interpreter { 
    public:
        void run();
    };
} // namespace jitpp

#endif
