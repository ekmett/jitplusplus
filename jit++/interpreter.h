#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

#include <jit++/tracer.h>

namespace jitpp { 
    class interpreter : public tracer { 
    public:
	interpreter() : tracer() {}
	inline void operator ()() { start(); } 
    private:
	void run();
	void interpret();
    };
}

#endif
