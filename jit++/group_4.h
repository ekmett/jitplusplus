#ifndef INCLUDED_JITPP_GROUP_4_H
#define INCLUDED_JITPP_GROUP_4_H

#include <jit++/common.h>
#include <jit++/interpreter_internal.h>

namespace jitpp { 
    using namespace jitpp::flags;

    struct group_4_base { 
        static inline bool inc_af(const context & ctx) {
            return (ctx.result & 0xf)  == 0;
        }
        static inline bool dec_af(const context & ctx) {
            return (ctx.result & 0xf) == 0xf;
        }
    };

    template <typename T> struct group_4 : group_4_base { 
        static const int size = sizeof(T)*8;
        static const int shift_mask = (size == 64) ? 0x3f : 0x1f;

        static inline int64_t inc(interpreter & i, int64_t x) {
            return i.handle_rflags(inc_flags,x+1);
        }
        static inline int64_t dec(interpreter & i, int64_t x) {
            return i.handle_rflags(dec_flags,x-1);
        }
        static inline bool dec_of(const context & ctx) {
            return ctx.result == (1ULL << (size - 1)) - 1;
        }
        static inline bool inc_of(const context & ctx) {
            return ctx.result == 1ULL << (size - 1);
        }

        static const handler inc_flags, dec_flags;
    public:
	static inline void interpret(interpreter & i) { 
            switch (i.reg) {
            case 0: E<T>(i,inc(i,E<T>(i))); return;
            case 1: E<T>(i,dec(i,E<T>(i))); return;
            default: illegal();
            }
	}
    };

    template <typename T> const handler group_4<T>::inc_flags = { bad_flag, inc_af, inc_of, OSZAP, 0 };
    template <typename T> const handler group_4<T>::dec_flags = { bad_flag, dec_af, dec_of, OSZAP, 0 };

} // jitpp

#endif
