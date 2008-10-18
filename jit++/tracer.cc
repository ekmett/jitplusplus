#include <ucontext.h>         // *context
#include <asm/prctl.h>        // SYS_arch_prctl
#include <sys/syscall.h>      // syscall

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
	VLOG(3) << "Fetched segment bases. fs = " << reinterpret_cast<uint64_t>(m_fs_base) << " gs = " << reinterpret_cast<uint64_t>(m_gs_base);
	swapcontext(&m_context,&m_meta_context);
    }
    void tracer::run_tracer(tracer * f) throw() { 
	VLOG(1) << "Entering trace";
	gregset_t & gregs = f->m_context.uc_mcontext.gregs;
	reg64 * reg = f->m_reg;
	// transform the registers into a format suitable for mod r/m indexing
	using namespace modrm;
	reg[rax] = gregs[REG_RAX];
	reg[rcx] = gregs[REG_RCX];
	reg[rdx] = gregs[REG_RDX];
	reg[rbx] = gregs[REG_RBX];
	reg[rsp] = gregs[REG_RSP];
	reg[rbp] = gregs[REG_RBP];
	reg[rsi] = gregs[REG_RSI];
	reg[rdi] = gregs[REG_RDI];
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
	gregs[REG_RAX] = reg[rax];
	gregs[REG_RCX] = reg[rcx];
	gregs[REG_RDX] = reg[rdx];
	gregs[REG_RBX] = reg[rbx];
	gregs[REG_RSP] = reg[rsp];
	gregs[REG_RBP] = reg[rbp];
	gregs[REG_RSI] = reg[rsi];
	gregs[REG_RDI] = reg[rdi];
	gregs[REG_R8]  = reg[8];
	gregs[REG_R9]  = reg[9];
	gregs[REG_R10] = reg[10];
	gregs[REG_R11] = reg[11];
	gregs[REG_R12] = reg[12];
	gregs[REG_R13] = reg[13];
	gregs[REG_R14] = reg[14];
	gregs[REG_R15] = reg[15];
	VLOG(1) << "Leaving trace";
	setcontext(&f->m_context); 
	// resume execution at our new location
    }
} // namespace jitpp
