#include <asm/prctl.h>        // SYS_arch_prctl
#include <sys/syscall.h>      // syscall

#include <jit++/tracer.h>    // tracer::*
#include <jit++/exception.h> // unsupported_opcode_exception
#include <jit++/internal.h>  // DEFINE_*

DEFINE_uint64(jitpp_default_stack_size,102400,"the default stack size for tracing fibers. must be larger than the size of /proc/self/maps!");

namespace jitpp { 
    size_t tracer::default_stack_size() throw() { 
	return FLAGS_jitpp_default_stack_size; 
    }

    tracer::tracer(size_t stack_size) : m_stack_size(stack_size) {
	m_stack = new uint8_t[stack_size];
    }

    tracer::~tracer() { 
	delete[] m_stack;
    }

    int64_t tracer::fs_base() const throw() { 
 	if (unlikely(!m_fs_base_known)) {
	    syscall(SYS_arch_prctl,ARCH_GET_FS,&m_fs_base);
	    m_fs_base_known = true;
	}
	return m_fs_base;
    }
    int64_t tracer::gs_base() const throw() { 
 	if (unlikely(!m_gs_base_known)) {
	    syscall(SYS_arch_prctl,ARCH_GET_FS,&m_gs_base);
	    m_gs_base_known = true;
	}
	return m_gs_base;
    }

    void tracer::run_tracer(tracer & t) throw() {
	t.m_fs_base_known = t.m_gs_base_known = false;
	t.run();
    }

    void tracer::start() throw() {
	enter_interpreter(*this,m_stack,m_stack_size,reinterpret_cast<void(*)(context &)>(&run_tracer));
    }
} // namespace jitpp
