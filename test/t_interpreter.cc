#include <jit++.h>
#include <jit++/common.h>
#include <jit++/interpreter.h>

#include <stdio.h>

using namespace jitpp;

class mock_tracer : public tracer { 
public:
    mock_tracer() : tracer() {}
    void run() throw() { 
        VLOG(3) << "running tracer";
    }
};
// #define TRACER mock_tracer 
#define TRACER interpreter
TRACER t;


int main(int argc, char ** argv) { 
    jitpp::application(argc,argv);
    VLOG(1) << "options set";
    VLOG(1) << "tracer constructed";
    printf("Hello World 1\n");
    fflush(stdout);
    t.start();
#ifdef SCENARIO_1
    printf("Hello World 2\n");
#endif
    for (int i=0;i<10;++i) { 
	puts("hi\n");
	fflush(stdout);
    }
    t.stop();
    puts("rejoined\n");
    return 0;
}
