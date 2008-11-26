#include <jit++/interpreting/misc.h>

namespace jitpp { 
    namespace interpreting { 
	// make a syscall. register usage here conforms to the linux x86-64 ABI
        void misc::syscall_() { 
            int64_t RAX = rax();
            VLOG(1) << "executing syscall " << std::hex << RAX;
            if (RAX == 0xe7) uninterpretable(); // stop interpreting for exit syscall
            int64_t RBX = rbx();
            int64_t RDX = rdx();
            int64_t RSI = rsi();
            int64_t RDI = rdi();

	    // HACK: is there a better way to specify these constraints directly?
            register int64_t R8 asm("r8") = r8();
            register int64_t R9 asm("r9") = r9(); 
            register int64_t R11 asm("r11");
    
            __asm__ __volatile__ (
                "syscall"
                : "=a"(RAX), "=b"(RBX), "=r"(R11)
                : "a"(RAX), "b"(RBX), "d"(RDX), "r"(R9), "R"(RSI), "D"(RDI), "r"(R8)
                : "memory", "cc", "cx"
            );
    
            rax() = RAX;
            rbx() = RBX;
            r11() = R11;
        }
	// write all modified cache lines back to main memory and flush caches
        void misc::wbinvd() { 
	    __asm__ __volatile__ ("wbinvd" ::: "memory");
        }
	// invalidate internal caches
        void misc::invd() { 
	    __asm__ __volatile__ ("invd" ::: "memory");
	}
	// write EDX:EAX into the model-specific register indicated by ECX
        void misc::wrmsr() { 
	    int64_t RAX = rax();
	    int64_t RCX = rcx();
	    int64_t RDX = rdx();
	    __asm__ __volatile__("wrmsr" :: "a"(RAX), "c"(RCX), "d"(RDX) : "memory");
	}
	// read model-specific register indicated by ECX into EDX:EAX
	void misc::rdmsr() {
	    int64_t RCX = rax();
	    int64_t RAX, RDX;
	    __asm__ __volatile__("rdmsr" : "=a"(RAX), "=d"(RDX) : "c"(RCX) : "memory");
	    rax() = RAX;
	    rdx() = RDX;
	}
	// read performance counter indicated by ECX into EDX:EAX
	void misc::rdpmc() {
	    int64_t RCX = rax();
	    int64_t RAX, RDX;
	    __asm__ __volatile__("rdmsr" : "=a"(RAX), "=d"(RDX) : "c"(RCX) : "memory");
	    rax() = RAX;
	    rdx() = RDX;
	}
	// output TSC into EDX:EAX
	void misc::rdtsc() { 
	    int64_t RAX, RDX;
	    __asm__ __volatile__("rdtsc" : "=a"(RAX), "=d"(RDX) :: "memory");
	    rax() = RAX;
	    rdx() = RDX;
	}
        int64_t misc::lsl(int16_t descriptor) {
            int64_t result;
            int8_t  zero;
            __asm__ ("lsl %2,%0; setz %1" : "=q"(result), "=q"(zero) : "q"(descriptor) : "memory", "cc");
            zf(zero != 0);
            return result;
        }
        int64_t misc::lar(int16_t descriptor) { 
            int64_t result;
            int8_t  zero;
            __asm__ ("lar %2,%0; setz %1" : "=q"(result), "=q"(zero) : "q"(descriptor) : "memory", "cc");
            zf(zero != 0);
            return result;
        }

    }
}
