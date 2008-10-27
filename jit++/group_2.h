#ifndef INCLUDED_JITPP_GROUP_2_H
#define INCLUDED_JITPP_GROUP_2_H

#include <jit++/interpreter_internal.h>

namespace jitpp { 

    using namespace jitpp::flags;
    template <typename T> struct group_2 { 
    private:
        static const int size = sizeof(T)*8;
        static const int mask = size == 64 ? 0x3f : 0x1f;
    	static inline int64_t sal(interpreter & i, int64_t x, int count) { 
            return i.handle_rflags(sal_flags,x << count,x,count);
        }
        static inline int64_t sar(interpreter & i, int64_t x, int count) { 
            return i.handle_rflags(sar_flags,x >> count,x,count);
        }
        static inline uint64_t shr(interpreter & i, uint64_t x, int count) { 
            return i.handle_rflags(shr_flags,x >> count,x,count);
        }
        static inline uint64_t rol(interpreter & i, uint64_t x, int count) { 
	    return i.handle_rflags(rol_flags, (x << count) | (x >> (size - count)),x, count);
        }
        static inline uint64_t ror(interpreter & i, uint64_t x, int count) { 
	    return i.handle_rflags(ror_flags, (x << (size - count)) | (x >> count),x, count);
        }
	static inline uint64_t rcl(interpreter & i, uint64_t x, int count) { 
	    uint64_t carry_bit = i.cf() ? 1 : 0;
	    uint64_t result = (x << count) | carry_bit << (count - 1);
	    if (count != 1) result |= (x >> (size + 1 - count));
	    bool carry_result = (x >> (size - count)) & 1;
	    i.cf(carry_result);
	    i.of(carry_result ^ (result < 0));
	    return result;
	}
	static inline uint64_t rcr(interpreter & i, uint64_t x, int count) { 
	    uint64_t carry_bit = i.cf() ? 1 : 0;
	    uint64_t result = (x >> count) | (carry_bit << (size - count));
	    if (count != 1)
		result |= (x << (size + 1 - count));
	    bool carry_result = (x >> (count - 1)) & 1;
	    i.cf(carry_result);
	    i.of(((result << 1) & result) < 0);
	    return result;
	}

        static inline bool sal_cf(const context & ctx) {
            return ((ctx.op1 >> (size - ctx.op2)) & 1) != 0;
        }
        static inline bool sal_of(const context & ctx) { 
            return (ctx.op1 ^ ctx.result) < 0;
        }
	static inline bool sar_cf(const context & ctx) { 
	    return ((ctx.op1 >> (ctx.op2 - 1)) & 1) != 0;
	}
	static inline bool ror_cf(const context & ctx) {
            return (ctx.result < 0);
	}
	static inline bool shr_of(const context & ctx) { 
	    return ((ctx.result << 1) ^ ctx.result) < 0;
	}

    public:
	// honorary member :)
	static inline uint64_t shrd(interpreter & i, uint64_t x, uint64_t y, int count) {
	    return i.handle_rflags(shrd_flags, (y << (size - count)) | (x >> count), x, count);
        }
	static void interpret(interpreter & i, int count) { 
	    count &= mask;
            if (!count) return;
            switch (i.reg) {
            case 0: E<T>(i,rol(i,E<T>(i),count)); return; // ROL E?,??
            case 1: E<T>(i,ror(i,E<T>(i),count)); return; // ROR E?,??
            case 2: E<T>(i,rcl(i,E<T>(i),count)); return; // RCL E?,??
            case 3: E<T>(i,rcr(i,E<T>(i),count)); return; // RCR E?,??
            case 4: E<T>(i,sal(i,E<T>(i),count)); return; // SHL E?,??
            case 5: E<T>(i,shr(i,E<T>(i),count)); return; // SHR E?,??
            case 6: E<T>(i,sal(i,E<T>(i),count)); return; // SAL E?,?? = SHL E?,??
            case 7: E<T>(i,sar(i,E<T>(i),count)); return; // SAR E?,??
            }
	    logic_error();
	}
        static const flags::handler sal_flags, rol_flags, shr_flags, ror_flags, sar_flags, shrd_flags;
    };


    template <typename T> const flags::handler group_2<T>::sal_flags  = { sal_cf, flags::bad_flag, sal_of, 	     OSZPC,  AF };
    template <typename T> const flags::handler group_2<T>::rol_flags  = { sal_cf, flags::bad_flag, sal_of, 	     OF|CF,  0 };
    template <typename T> const flags::handler group_2<T>::sar_flags  = { sar_cf, flags::bad_flag, flags::bad_flag,  CF,     OF }; 
    template <typename T> const flags::handler group_2<T>::shr_flags  = { sar_cf, flags::bad_flag, shr_of,	     OSZPC,  AF };
    template <typename T> const flags::handler group_2<T>::ror_flags  = { ror_cf, flags::bad_flag, shr_of,           OF|CF,  0 };
    template <typename T> const flags::handler group_2<T>::shrd_flags = { sar_cf, flags::bad_flag, shr_of,           OSZPC, AF };

    // shlq shrq isn't technically in group 
} // namespace jitpp;

#endif

