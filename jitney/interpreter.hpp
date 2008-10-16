#ifndef INCLUDED_JITNEY_INTERPRETER

#include <jitney/tracer.hpp>

namespace jitney { 
    class interpreter : public tracer { 
    public:
	interpreter() : tracer() {}
	void run();
    };
}

#endif
