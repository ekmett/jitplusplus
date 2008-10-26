#include <jit++.h>
#include <jit++/common.h>
#include <jit++/interpreter.h>


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

int main(int argc, char ** argv) { 
    jitpp::application(argc,argv);
    VLOG(1) << "options set";
    TRACER t;
    VLOG(1) << "tracer constructed";
    printf("Hello World 1\n");
    t.start();
    printf("Hello World 2\n");
    return 0;
}
