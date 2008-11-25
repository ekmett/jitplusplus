#ifndef INCLUDED_JITPP_INTERPRETING_GROUP_2_H
#define INCLUDED_JITPP_INTERPRETING_GROUP_2_H

#include <jit++/interpreting/base.h>
#include <jit++/interpreting/flags.h>

namespace jitpp { 
  namespace interpreting { 
    using namespace jitpp::interpreting::flags;

    template <typename T> struct group_2_traits { 
        static const int size = sizeof(T)*8;
        static const int mask = size == 64 ? 0x3f : 0x1f;
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
        static const flags::handler sal_flags, rol_flags, shr_flags, ror_flags, sar_flags, shrd_flags;
    };

    template <typename T> const flags::handler group_2_traits<T>::sal_flags  
	= { sal_cf, flags::bad_flag, sal_of, OSZPC,  AF };
    template <typename T> const flags::handler group_2_traits<T>::rol_flags  
	= { sal_cf, flags::bad_flag, sal_of, OF|CF,  0 };
    template <typename T> const flags::handler group_2_traits<T>::sar_flags  
	= { sar_cf, flags::bad_flag, flags::bad_flag,  CF, OF }; 
    template <typename T> const flags::handler group_2_traits<T>::shr_flags  
	= { sar_cf, flags::bad_flag, shr_of, OSZPC,  AF };
    template <typename T> const flags::handler group_2_traits<T>::ror_flags  
	= { ror_cf, flags::bad_flag, shr_of, OF|CF,  0 };
    template <typename T> const flags::handler group_2_traits<T>::shrd_flags 
	= { sar_cf, flags::bad_flag, shr_of, OSZPC, AF };

    class group_2 : public virtual interpreter_base {
    private:
    public:
        template <typename T>
    	inline int64_t sal(int64_t x, int count) { 
            return handle_rflags(group_2_traits<T>::sal_flags,x << count,x,count);
        }
        template <typename T>
        inline int64_t sar(int64_t x, int count) { 
            return handle_rflags(group_2_traits<T>::sar_flags,x >> count,x,count);
        }
        template <typename T>
        inline uint64_t shr(uint64_t x, int count) { 
            return handle_rflags(group_2_traits<T>::shr_flags,x >> count,x,count);
        }
        template <typename T>
        inline uint64_t rol(uint64_t x, int count) { 
	    return handle_rflags(group_2_traits<T>::rol_flags, (x << count) | (x >> (group_2_traits<T>::size - count)),x, count);
        }
        template <typename T>
        inline uint64_t ror(uint64_t x, int count) { 
	    return handle_rflags(group_2_traits<T>::ror_flags, (x << (group_2_traits<T>::size - count)) | (x >> count),x, count);
        }
        template <typename T>
	inline uint64_t rcl(uint64_t x, int count) { 
	    uint64_t carry_bit = cf() ? 1 : 0;
	    uint64_t result = (x << count) | carry_bit << (count - 1);
	    if (count != 1) result |= (x >> (group_2_traits<T>::size + 1 - count));
	    bool carry_result = (x >> (group_2_traits<T>::size - count)) & 1;
	    cf(carry_result);
	    of(carry_result ^ (result < 0));
	    return result;
	}
        template <typename T>
	inline uint64_t rcr(uint64_t x, int count) { 
	    uint64_t carry_bit = cf() ? 1 : 0;
	    uint64_t result = (x >> count) | (carry_bit << (group_2_traits<T>::size - count));
	    if (count != 1)
		result |= (x << (group_2_traits<T>::size + 1 - count));
	    bool carry_result = (x >> (count - 1)) & 1;
	    cf(carry_result);
	    of(((result << 1) & result) < 0);
	    return result;
	}

	template <typename T>
	inline uint64_t shrd(uint64_t x, uint64_t y, int count) {
	    return handle_rflags(group_2_traits<T>::shrd_flags, (y << (group_2_traits<T>::size - count)) | (x >> count), x, count);
        }

        template <typename T>
	void interpret_group_2(int count) { 
	    count &= group_2_traits<T>::mask;
            if (!count) return;
            switch (reg) {
            case 0: E<T>(rol<T>(E<T>(),count)); return; // ROL E?,??
            case 1: E<T>(ror<T>(E<T>(),count)); return; // ROR E?,??
            case 2: E<T>(rcl<T>(E<T>(),count)); return; // RCL E?,??
            case 3: E<T>(rcr<T>(E<T>(),count)); return; // RCR E?,??
            case 4: E<T>(sal<T>(E<T>(),count)); return; // SHL E?,??
            case 5: E<T>(shr<T>(E<T>(),count)); return; // SHR E?,??
            case 6: E<T>(sal<T>(E<T>(),count)); return; // SAL E?,?? = SHL E?,??
            case 7: E<T>(sar<T>(E<T>(),count)); return; // SAR E?,??
            }
	    logic_error();
	}
    }; // class group_2
  } // namespace interpreting
} // namespace jitpp;

#endif

