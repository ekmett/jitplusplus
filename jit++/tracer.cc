#include <string.h>          // memset
#include <jit++/tracer.h>    // tracer::*
#include <jit++/internal.h>  // DEFINE_*

DEFINE_uint64(jitpp_default_stack_size,16384,"the default stack size for tracing. If too small then deep recursion or sparse allocas will fail to trace.");

namespace jitpp { 
    size_t tracer::default_stack_size() { 
	return FLAGS_jitpp_default_stack_size; 
    }

    tracer::tracer(size_t stack_size) : m_stack_size(stack_size) {
	// m_stack = new uint8_t[stack_size];
	for (int i=0;i<16;++i) m_reg[i] = 0xbad;
    }

    tracer::~tracer() { 
	// delete[] m_stack;
    }
} // namespace jitpp
