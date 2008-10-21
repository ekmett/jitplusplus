#include <jit++.h>
#include <jit++/internal.h>

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
    jitpp::options(argc,argv);
    VLOG(3) << "constructing tracer";
    TRACER t;
    VLOG(3) << "starting tracer";
    t.start();
    VLOG(3) << "interpreting";
    return 0;
}
