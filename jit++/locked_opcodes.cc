#include <jit++/common.h>
#include <jit++/interpreter_internal.h>

namespace jitpp { 

    template <typename T> static inline void lock_inc(interpreter & i, T * ptr);
    template <> static inline void lock_inc<>(interpreter & i, int8_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; incb %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	i.set_flags(flags::OSZAP, flags);
    }
    template <> static inline void lock_inc<>(interpreter & i, int16_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; incw %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	i.set_flags(flags::OSZAP, flags);
    }
    template <> static inline void lock_inc<>(interpreter & i, int32_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; incl %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	i.set_flags(flags::OSZAP, flags);
    }
    template <> static inline void lock_inc<>(interpreter & i, int64_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; incq %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	i.set_flags(flags::OSZAP, flags);
    }
 

    template <typename T> static inline void lock_dec(interpreter & i, T * ptr);
    template <> static inline void lock_dec<>(interpreter & i, int8_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; decb %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	i.set_flags(flags::OSZAP, flags);
    }
    template <> static inline void lock_dec<>(interpreter & i, int16_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; decw %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	i.set_flags(flags::OSZAP, flags);
    }
    template <> static inline void lock_dec<>(interpreter & i, int32_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; decl %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	i.set_flags(flags::OSZAP, flags);
    }
    template <> static inline void lock_dec<>(interpreter & i, int64_t * ptr) { 
	int64_t flags;
	__asm__ __volatile__("lock; decq %0\n\tpushfq\n\tpopq %1" : "=m"(*ptr), "=qm"(flags) : "m"(*ptr));
	i.set_flags(flags::OSZAP, flags);
    }

    template <typename T> static inline T lock_cmpxchg(interpreter & i, T * p, T o, T n);

    template <> static inline int8_t lock_cmpxchg<>(interpreter & i, int8_t * p, int8_t o, int8_t n) {
        int8_t r;
        __asm__ __volatile__("lock; cmpxchgb %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        i.zf(r == o);
        return r;
    }
    template <> static inline int16_t lock_cmpxchg<>(interpreter & i, int16_t * p, int16_t o, int16_t n) {
        int16_t r;
        __asm__ __volatile__("lock; cmpxchgw %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        i.zf(r == o);
        return r;
    }
    template <> static inline int32_t lock_cmpxchg<>(interpreter & i, int32_t * p, int32_t o, int32_t n) {
        int32_t r;
        __asm__ __volatile__("lock; cmpxchgl %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        i.zf(r == o);
        return r;
    }
    template <> static inline int64_t lock_cmpxchg<>(interpreter & i, int64_t * p, int64_t o, int64_t n) {
        int64_t r;
        __asm__ __volatile__("lock; cmpxchgq %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        i.zf(r == o);
        return r;
    }

template <typename os> static void interpret_locked_opcode(interpreter & i) { 
    typedef int8_t b;
    typedef int16_t w;
    typedef typename os::v v;
    typedef typename os::z z;

    if (i.op.mod == 3) 
	illegal(); 

    int64_t addr = i.mem();
    v * vp = reinterpret_cast<v*>(addr); // aliases
    b * bp = reinterpret_cast<b*>(addr);
    w * wp = reinterpret_cast<w*>(addr);
    z * zp = reinterpret_cast<z*>(addr);

    switch (i.op.code) { 
    case 0x1b0: // LOCK CMPXCHG Mb,Gb
	reg<b>(i,0,lock_cmpxchg<b>(i,bp,reg<b>(i,0),G<b>(i))); 
	return;
    case 0x1b1: // LOCK CMPXCHG Mv,Gv
	reg<v>(i,0,lock_cmpxchg<v>(i,vp,reg<v>(i,0),G<v>(i))); 
	return;
    case 0xfe:
	switch (i.op.reg) { 
        case 0: lock_inc<b>(i,bp); return; // LOCK INC Mb
        case 1: lock_dec<b>(i,bp); return; // LOCK DEC Mb
	default: illegal();
	}
    case 0xff:
	switch (i.op.reg) { 
        case 0: lock_inc<v>(i,vp); return; // LOCK INC Mv
        case 1: lock_dec<v>(i,vp); return; // LOCK DEC Mv
	default: illegal();
	}
    default: unsupported();
    }
}
void interpret_locked_opcode_16(interpreter & i) { 
	interpret_locked_opcode<os16>(i);
}
void interpret_locked_opcode_32(interpreter & i) { 
	interpret_locked_opcode<os32>(i);
}
void interpret_locked_opcode_64(interpreter & i) { 
	interpret_locked_opcode<os64>(i);
}

} // namespace jitpp
