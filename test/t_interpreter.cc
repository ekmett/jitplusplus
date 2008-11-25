#include <jit++.h>
#include <jit++/common.h>
#include <jit++/interpreter.h>

#include <stdio.h>

using namespace jitpp;

jitpp::interpreter t;


int main(int argc, char ** argv) { 
    jitpp::application(argc,argv);
    VLOG(1) << "options set";
    VLOG(1) << "tracer constructed";
    printf("0123456789\n");
    fflush(stdout);
    t.start();
    printf("0123456789\n");
    fflush(stdout);
    for (int i=0;i<10;++i) { 
	puts("hi\n");
    fflush(stdout);
    }
    t.stop();
    puts("rejoined\n");
    fflush(stdout);
    return 0;
}
