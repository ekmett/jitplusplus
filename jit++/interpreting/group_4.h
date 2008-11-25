#ifndef INCLUDED_JITPP_INTERPRETING_GROUP_4_H
#define INCLUDED_JITPP_INTERPRETING_GROUP_4_H

#include <jit++/common.h>
#include <jit++/interpreting/base.h>

namespace jitpp { 
  namespace interpreting { 
    using namespace jitpp::interpreting::flags;

    template <typename T> struct group_4_traits { 
        static const int size = sizeof(T)*8;
        static const int shift_mask = (size == 64) ? 0x3f : 0x1f;
        static inline bool dec_of(const context & ctx) {
            return ctx.result == (1ULL << (size - 1)) - 1;
        }
        static inline bool inc_of(const context & ctx) {
            return ctx.result == 1ULL << (size - 1);
        }
        static inline bool inc_af(const context & ctx) {
            return (ctx.result & 0xf)  == 0;
        }
        static inline bool dec_af(const context & ctx) {
            return (ctx.result & 0xf) == 0xf;
        }
        static const handler inc_flags, dec_flags;
    };

    template <typename T> const handler group_4_traits<T>::inc_flags = { bad_flag, inc_af, inc_of, OSZAP, 0 };
    template <typename T> const handler group_4_traits<T>::dec_flags = { bad_flag, dec_af, dec_of, OSZAP, 0 };

    class group_4 : public virtual interpreter_base { 
    public:
	template <typename T>
        inline int64_t inc(int64_t x) {
            return handle_rflags(group_4_traits<T>::inc_flags,x+1);
        }
	template <typename T>
        inline int64_t dec(int64_t x) {
            return handle_rflags(group_4_traits<T>::dec_flags,x-1);
        }
	template <typename T>
	inline void interpret_group_4() { 
            switch (reg) {
            case 0: E<T>(inc<T>(E<T>())); return;
            case 1: E<T>(dec<T>(E<T>())); return;
            default: illegal();
            }
	}
    };
  } // interpreting
} // jitpp

#endif
