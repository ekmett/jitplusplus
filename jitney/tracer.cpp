#include <jitney/tracer.h>
#include <asm/prctl.h>
#include <sys/syscall.h>

namespace jitney { 
    tracer::tracer(size_t stack_size) { 
	getcontext(&m_meta_context);
	// build a stack for the jit fiber
	m_meta_context.uc_stack.ss_sp = new char[stack_size];
	m_meta_context.uc_stack.ss_size = stack_size;
	m_meta_context.uc_link = &uc;
	// prepare that context to call into run on this object
	makecontext(&m_meta_context, &tracer::run_tracer, 1, this);

	// HACK: the x86-64 implementation of makecontext can only bootstrap with 32 bit arguments
	// this passes a full pointer through to the first argument using the x86-64 ABI
	m_meta_context.uc_mcontext.gregs[REG_RDI] = reinterpret_cast<int64_t>(this);
    }
    tracer::~tracer() { 
	// clean up the stack
	delete[] m_meta_context.uc_stack.ss_sp;
    }
    void tracer::start() { 
        // store the 64 bit base pointer for fs and gs so we have TLS & can abuse gs in userspace if we so desire
	syscall(SYS_arch_prctl,ARCH_GET_FS,&m_fs_base);
        syscall(SYS_arch_prctl,ARCH_GET_GS,&m_gs_base);
	swapcontext(&uc,&m_meta_context);
    }
    void tracer::run_tracer(tracer * f) { 
	gregset_t & gregs = f->ucp.uc_mcontext.gregs;
	reg64 * reg = f->m_reg;
	// transform the registers into a format suitable for mod r/m indexing
	reg[modrm::rax] = gregs[REG_RAX];
	reg[modrm::rcx] = gregs[REG_RCX];
	reg[modrm::rdx] = gregs[REG_RDX];
	reg[modrm::rbx] = gregs[REG_RBX];
	reg[modrm::rsp] = gregs[REG_RSP];
	reg[modrm::rbp] = gregs[REG_RBP];
	reg[modrm::rsi] = gregs[REG_RSI];
	reg[modrm::rdi] = gregs[REG_RDI];
	reg[8]  = gregs[REG_R8];
	reg[9]  = gregs[REG_R9];
	reg[10] = gregs[REG_R10];
	reg[11] = gregs[REG_R11];
	reg[12] = gregs[REG_R12];
	reg[13] = gregs[REG_R13];
	reg[14] = gregs[REG_R14];
	reg[15] = gregs[REG_R15];
	f->run();
	// transform them back into the context
	gregs[REG_RAX] = reg[modrm::rax];
	gregs[REG_RCX] = reg[modrm::rcx];
	gregs[REG_RDX] = reg[modrm::rdx];
	gregs[REG_RBX] = reg[modrm::rbx];
	gregs[REG_RSP] = reg[modrm::rsp];
	gregs[REG_RBP] = reg[modrm::rbp];
	gregs[REG_RSI] = reg[modrm::rsi];
	gregs[REG_RDI] = reg[modrm::rdi];
	gregs[REG_R8]  = reg[8];
	gregs[REG_R9]  = reg[9];
	gregs[REG_R10] = reg[10];
	gregs[REG_R11] = reg[11];
	gregs[REG_R12] = reg[12];
	gregs[REG_R13] = reg[13];
	gregs[REG_R14] = reg[14];
	gregs[REG_R15] = reg[15];
	// resume execution at our new location
	setcontext(&uc); // never returns
    }

    // abuse udis86 to print out the current instruction
    size_t tracer::print_instruction() { 
	ud_t ud;
	ud_init(&ud);
	ud_set_input_buffer(&ud,rip(),15);
	ud_set_mode(&ud,64);
	ud_set_pc(&ud,rip());
	ud_set_syntax(&ud,UD_SYN_ATT);
	size_t bytes = ud_disassemble(&ud);
	// we can also emit relevant register contents since this is mid trace
	printf("%16lx: %-45:45s %s\n", (uint64_t)ud_insn_off(&ud),ud_insn_hex(&ud),ud_insn_asm(&ud));
	return bytes;
    }
} // namespace jitney

#endif
