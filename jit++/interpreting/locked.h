#include <jit++/common.h>
#include <jit++/interpreting/base.h>

namespace jitpp { 
  namespace interpreting {
    class locked_interpreter : public virtual interpreter_base { 
    private:
        template <typename T> inline void lock_inc(T * ptr);
        template <typename T> inline void lock_dec(T * ptr);
        template <typename T> inline T lock_cmpxchg(T * p, T o, T n);
    public:
        template <typename os> void interpret_locked_opcode();
    };

    template <> inline void locked_interpreter::lock_inc<>(int8_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; incb %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	set_flags(flags::OSZAP, flags);
    }
    template <> inline void locked_interpreter::lock_inc<>(int16_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; incw %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	set_flags(flags::OSZAP, flags);
    }
    template <> inline void locked_interpreter::lock_inc<>(int32_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; incl %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	set_flags(flags::OSZAP, flags);
    }
    template <> inline void locked_interpreter::lock_inc<>(int64_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; incq %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
        set_flags(flags::OSZAP, flags);
    }
 
    template <> inline void locked_interpreter::lock_dec<>(int8_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; decb %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	set_flags(flags::OSZAP, flags);
    }
    template <> inline void locked_interpreter::lock_dec<>(int16_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; decw %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	set_flags(flags::OSZAP, flags);
    }
    template <> inline void locked_interpreter::lock_dec<>(int32_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; decl %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
        set_flags(flags::OSZAP, flags);
    }
    template <> inline void locked_interpreter::lock_dec<>(int64_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; decq %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	set_flags(flags::OSZAP, flags);
    }


    template <> inline int8_t locked_interpreter::lock_cmpxchg<>(int8_t * p, int8_t o, int8_t n) {
        int8_t r;
        __asm__ __volatile__("lock; cmpxchgb %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        zf(r == o);
        return r;
    }
    template <> inline int16_t locked_interpreter::lock_cmpxchg<>(int16_t * p, int16_t o, int16_t n) {
        int16_t r;
        __asm__ __volatile__("lock; cmpxchgw %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        zf(r == o);
        return r;
    }
    template <> inline int32_t locked_interpreter::lock_cmpxchg<>(int32_t * p, int32_t o, int32_t n) {
        int32_t r;
        __asm__ __volatile__("lock; cmpxchgl %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        zf(r == o);
        return r;
    }
    template <> inline int64_t locked_interpreter::lock_cmpxchg<>(int64_t * p, int64_t o, int64_t n) {
        int64_t r;
        __asm__ __volatile__("lock; cmpxchgq %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        zf(r == o);
        return r;
    }
    
    template <typename os> void locked_interpreter::interpret_locked_opcode() { 
        typedef int8_t b;
        typedef int16_t w;
        typedef typename os::v v;
        typedef typename os::z z;
    
        if (mod == 3) illegal(); 
    
        int64_t addr = mem();
        v * vp = reinterpret_cast<v*>(addr); // aliases
        b * bp = reinterpret_cast<b*>(addr);
        w * wp = reinterpret_cast<w*>(addr);
        z * zp = reinterpret_cast<z*>(addr);
    
        switch (code) { 
        case 0x1b0: // LOCK CMPXCHG Mb,Gb
            set_reg<b>(0,lock_cmpxchg<b>(bp,get_reg<b>(0),G<b>())); 
            return;
        case 0x1b1: // LOCK CMPXCHG Mv,Gv
            set_reg<v>(0,lock_cmpxchg<v>(vp,get_reg<v>(0),G<v>())); 
            return;
        case 0xfe:
            switch (reg) { 
            case 0: lock_inc<b>(bp); return; // LOCK INC Mb
            case 1: lock_dec<b>(bp); return; // LOCK DEC Mb
            default: illegal();
            }
        case 0xff:
            switch (reg) { 
            case 0: lock_inc<v>(vp); return; // LOCK INC Mv
            case 1: lock_dec<v>(vp); return; // LOCK DEC Mv
            default: illegal();
            }
        default: unsupported();
        }
    }
 
} } // namespace jitpp::interpreting
