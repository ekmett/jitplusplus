#ifndef INCLUDED_JITNEY_INTERPRETER_H
#define INCLUDED_JITNEY_INTERPRETER_H

#include <jitney/tracer.h>

namespace jitney { 
    class interpreter : public tracer { 
    public:
	interpreter() : tracer() {}
	void run();
    private:
	void interpret();
    };
}

#endif
