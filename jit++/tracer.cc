#include <string.h>          // memset
#include <jit++/tracer.h>    // tracer::*
#include <jit++/internal.h>  // DEFINE_*
#include <pthread.h>

DEFINE_uint64(jitpp_default_stack_size,16384,"the default stack size for tracing. If too small then deep recursion or sparse allocas will fail to trace.");

namespace { 
    void * call_tracer_run(void * t) {
	static_cast<jitpp::tracer *>(t)->run();
	return 0; // this could return the function pointer to execute on subsequent runs
    }
}

namespace jitpp { 
    size_t tracer::default_stack_size() { 
	return FLAGS_jitpp_default_stack_size; 
    }

    tracer::tracer(size_t stack_size) : m_stack_size(stack_size) {
	for (int i=0;i<16;++i) m_reg[i] = 0xbad;
    }

    tracer::~tracer() {}

    // executed far down the stack of the to-be-interpreted thread
    void tracer::stub() { 
	pthread_t thread;
	pthread_attr_t attr;
	size_t stack_size = std::max(8192,PTHREAD_STACK_MIN);

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr,stack_size);
    retry:
	int result = pthread_create(&thread, &attr, &call_tracer_run, this);
	switch (result) { 
	case EAGAIN: goto retry; // transient fault
	case EINVAL: 
	    LOG(DFATAL) << "stack size attribute invalid (stack_size = " << stack_size << ")";
	    break; // don't jit
	case EPERM:  
	    LOG(DFATAL) << "unable to create pthread (EPERM)";
	    break; // don't jit
	default:
	    LOG(DFATAL) << "unexpected error (" << result << ")";
	    break;
	    // fall through
	case 0:      
	    pthread_join(thread,0);
	    break;
        }
	pthread_attr_destroy(&attr);
    }
    
} // namespace jitpp
