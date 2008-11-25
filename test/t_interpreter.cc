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
    printf("Hello World 1\n");
    fflush(stdout);
    t.start();
    for (int i=0;i<10;++i) { 
	puts("hi\n");
	fflush(stdout);
    }
    t.stop();
    puts("rejoined\n");
    return 0;
}
