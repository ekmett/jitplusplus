#include <jit++/interpreting/base.h>

namespace jitpp { 
  namespace interpreting { 

    class misc : public virtual interpreter_base { 
    public:
        template <typename T> 
        inline void cmpxchg() {
            int8_t value = E<T>();
            if (zf(value == get_reg<T>(0))) E<T>(G<T>());
            else set_reg<T>(0,value);
        }
        template <typename T> 
        inline void xchg() { 
            T t = G<T>(); G<T>(E<T>()); E<T>(t);
        }
        template <typename T> 
        inline void xchg(int r) { 
            T t = get_reg<T>(r);
            set_reg<T>(r,get_reg<T>(0));
            set_reg<T>(0,t);
        }
    
        template <typename T> 
        inline void movs() { 
            int64_t base = seg_base();
            T * s = reinterpret_cast<T*>(base + rsi());
            T * d = reinterpret_cast<T*>(rdi());
            int count = repetitions();
            VLOG(1) 
                   << "movs:"
                << " copying " << count << " " << sizeof(T) << " byte chunks from " 
                << std::hex << (base + rsi()) << " to " << rdi() 
                << (df() ? " descending" : "");
            if (df()) 
                while (count-- != 0) *s-- = *d--;
            else 
                while (count-- != 0) *s++ = *d++;
            // TODO: initialize any flags here ?
            rsi() = reinterpret_cast<int64_t>(s) - base;
            rdi() = reinterpret_cast<int64_t>(d);
        }
    
        void syscall_();
	void wbinvd();
        void invd();
        void wrmsr();
	void rdmsr();
	void rdtsc();
	void rdpmc();
	int64_t lsl(int16_t descriptor);
	int64_t lar(int16_t descriptor);
    };
  }
}
