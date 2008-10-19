#include <ucontext.h>         // *context
#include <asm/prctl.h>        // SYS_arch_prctl
#include <sys/syscall.h>      // syscall
#include <iostream>

#include <jit++/tracer.h>    // tracer::*
#include <jit++/exception.h> // unsupported_opcode_exception
#include <jit++/internal.h>  // LOG

DEFINE_uint64(jitpp_default_stack_size,102400,"the default stack size for tracing fibers. must be larger than the size of /proc/self/maps!");

namespace jitpp { 
    size_t tracer::default_stack_size() { 
	return FLAGS_jitpp_default_stack_size; 
    }

    tracer::tracer(size_t stack_size) {
	if (stack_size == 0) stack_size = default_stack_size(); 
	getcontext(&m_meta_context);
	// build a stack for the jit fiber
	m_meta_context.uc_stack.ss_sp = new char[stack_size];
	m_meta_context.uc_stack.ss_size = stack_size;
	m_meta_context.uc_link = &m_context;
	// prepare that context to call into run on this object
	makecontext(&m_meta_context, reinterpret_cast<void(*)()>(&tracer::run_tracer), 1, this);

	// HACK: the x86-64 implementation of makecontext can only bootstrap with 32 bit arguments
	// this passes a full pointer through to the first argument using the x86-64 ABI
	m_meta_context.uc_mcontext.gregs[REG_RDI] = reinterpret_cast<int64_t>(this);
    }
    tracer::~tracer() {
	// clean up the stack
	delete[] reinterpret_cast<char*>(m_meta_context.uc_stack.ss_sp);
    }
    void tracer::start() throw() { 
        // store the 64 bit base pointer for fs and gs so we have TLS & can abuse gs in userspace if we so desire
	syscall(SYS_arch_prctl,ARCH_GET_FS,&m_fs_base);
        syscall(SYS_arch_prctl,ARCH_GET_GS,&m_gs_base);
	VLOG(4) << "Fetched segment bases. fs = " << std::hex << m_fs_base << " gs = " << std::hex << m_gs_base;
	swapcontext(&m_context,&m_meta_context);
    }
    void tracer::run_tracer(tracer * f) throw() { 
	VLOG(3) << "Entering trace";
	gregset_t & gregs = f->m_context.uc_mcontext.gregs;
	int64_t * reg = f->m_reg;
	// transform the registers into a format suitable for mod r/m indexing
	reg[reg_rax] = gregs[REG_RAX];
	reg[reg_rcx] = gregs[REG_RCX];
	reg[reg_rdx] = gregs[REG_RDX];
	reg[reg_rbx] = gregs[REG_RBX];
	reg[reg_rsp] = gregs[REG_RSP];
	reg[reg_rbp] = gregs[REG_RBP];
	reg[reg_rsi] = gregs[REG_RSI];
	reg[reg_rdi] = gregs[REG_RDI];
	reg[8]  = gregs[REG_R8];
	reg[9]  = gregs[REG_R9];
	reg[10] = gregs[REG_R10];
	reg[11] = gregs[REG_R11];
	reg[12] = gregs[REG_R12];
	reg[13] = gregs[REG_R13];
	reg[14] = gregs[REG_R14];
	reg[15] = gregs[REG_R15];
	try { 
	    f->run();
	} catch (std::exception & e) {
	    LOG(DFATAL) << "Unhandled exception in tracer: " << e.what();
	} catch (...) {
	    LOG(DFATAL) << "Unhandled unknown exception in tracer.";
        }
	// transform them back into the context
	gregs[REG_RAX] = reg[reg_rax];
	gregs[REG_RCX] = reg[reg_rcx];
	gregs[REG_RDX] = reg[reg_rdx];
	gregs[REG_RBX] = reg[reg_rbx];
	gregs[REG_RSP] = reg[reg_rsp];
	gregs[REG_RBP] = reg[reg_rbp];
	gregs[REG_RSI] = reg[reg_rsi];
	gregs[REG_RDI] = reg[reg_rdi];
	gregs[REG_R8]  = reg[8];
	gregs[REG_R9]  = reg[9];
	gregs[REG_R10] = reg[10];
	gregs[REG_R11] = reg[11];
	gregs[REG_R12] = reg[12];
	gregs[REG_R13] = reg[13];
	gregs[REG_R14] = reg[14];
	gregs[REG_R15] = reg[15];
	VLOG(3) << "Leaving trace";
	setcontext(&f->m_context); 
	// resume execution at our new location
    }
} // namespace jitpp
