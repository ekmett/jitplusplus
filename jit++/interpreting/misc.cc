#include <jit++/interpreting/misc.h>

namespace jitpp { 
    namespace interpreting { 
        void misc::syscall_() { 
            int64_t rax = this->rax();
            VLOG(1) << "executing syscall " << std::hex << rax;
            if (rax == 0xe7) uninterpretable(); // bail out on exit syscall
            int64_t rbx = this->rbx();
            int64_t rdx = this->rdx();
            int64_t rsi = this->rsi();
            int64_t rdi = this->rdi();
            register int64_t r8 asm("r8") = this->r8();
            register int64_t r9 asm("r9") = this->r9(); // how to specify these constraints directly?
            register int64_t r11 asm("r11");
    
            // should be valid for linux x86-64 abi
            __asm__ __volatile__ (
                "syscall"
                : "=a"(rax), "=b"(rbx), "=r"(r11)
                : "a"(rax), "b"(rbx), "d"(rdx), "r"(r9), "R"(rsi), "D"(rdi), "r"(r8)
                : "memory", "cc", "cx"
            );
    
            this->rax() = rax;
            this->rbx() = rbx;
            this->r11() = r11;
        }
    }
}
