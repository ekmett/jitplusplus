#include <jit++/tracer.h>    // tracer::*
#include <jit++/internal.h>  // DEFINE_*

DEFINE_uint64(jitpp_default_stack_size,102400,"the default stack size for tracing fibers. must be larger than the size of /proc/self/maps!");

namespace jitpp { 
    size_t tracer::default_stack_size() { 
	return FLAGS_jitpp_default_stack_size; 
    }

    tracer::tracer(size_t stack_size) : m_stack_size(stack_size) {
	m_stack = new uint8_t[stack_size];
    }

    tracer::~tracer() { 
	delete[] m_stack;
    }
} // namespace jitpp
