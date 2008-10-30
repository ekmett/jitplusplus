#ifndef INCLUDED_JITPP_GROUP_3_H
#define INCLUDED_JITPP_GROUP_3_H

#include <jit++/common.h>
#include <jit++/interpreter_internal.h>
#include <jit++/group_1.h> // AND

namespace jitpp { 
    using namespace jitpp::flags;

    struct group_3_base { 
        static inline bool neg_cf(const context & ctx) {
            return ctx.result != 0;
        }
        static inline bool neg_af(const context & ctx) {
            return (ctx.result & 0xf) != 0;
        }
    };

    template <typename T> struct group_3 : group_3_base {
    private:
        static inline int64_t neg(interpreter & i, int64_t x) {
            return i.handle_rflags(neg_flags,-x);
        }
        static inline void mul(interpreter & i);
        static inline void imul(interpreter & i);
        static inline void div(interpreter & i);
        static inline void idiv(interpreter & i);

        static inline bool neg_of(const context & ctx) {
            return ctx.result == 1ULL << (sizeof(T)*8 - 1);
        }

	static const flags::handler neg_flags; 
    public:
	static inline void interpret(interpreter & i) { 
            switch (i.reg) {
            case 0: // TEST E?,I?
            case 1: group_1<T>::and_(i,E<T>(i),i.imm); return; // TEST E?, I?*
	    case 2: E<T>(i,~E<T>(i)); return; // NOT E?
            case 3: E<T>(i,neg(i,E<T>(i))); return; // NEG E?
            case 4: mul(i); return;  // MUL E?
            case 5: imul(i); return; // IMUL E?
            case 6: div(i); return;  // DIV E?
            case 7: idiv(i); return; // DIV E?
            default: break;
            }
            unsupported();
	}
    };

    // NEG flags
    template <typename T> const flags::handler group_3<T>::neg_flags = { neg_cf, neg_af, neg_of, flags::OSZAPC, 0 };


    // DIV E?

    template <> inline void group_3<int64_t>::div(interpreter & i) {
	int64_t rax = get_reg<int64_t>(i,0);
	int64_t rdx = get_reg<int64_t>(i,2);
	int64_t divisor = E<int64_t>(i);
	__asm__("divq %2" : "=a"(rax),"=d"(rdx) : "q"(divisor), "a"(rax), "d"(rdx));
	set_reg<int64_t>(i,0,rax);
	set_reg<int64_t>(i,2,rdx);
    }

    template <> inline void group_3<int32_t>::div(interpreter & i) {
	int32_t eax = get_reg<int32_t>(i,0);
	int32_t edx = get_reg<int32_t>(i,2);
	int32_t divisor = E<int32_t>(i);
	__asm__("divl %2" : "=a"(eax),"=d"(edx) : "q"(divisor), "a"(eax), "d"(edx));
	set_reg<int32_t>(i,0,eax);
	set_reg<int32_t>(i,2,edx);
    }

    template <> inline void group_3<int16_t>::div(interpreter & i) {
	int16_t ax = get_reg<int16_t>(i,0);
	int16_t dx = get_reg<int16_t>(i,2);
	int16_t divisor = E<int16_t>(i);
	__asm__("divw %2" : "=a"(ax),"=d"(dx) : "q"(divisor), "a"(ax), "d"(dx));
	set_reg<int16_t>(i,0,ax);
	set_reg<int16_t>(i,2,dx);
    }

    template <> inline void group_3<int8_t>::div(interpreter & i) {
	int16_t ax = get_reg<int64_t>(i,0);
	int8_t divisor = E<int64_t>(i);
	__asm__("divb %1" : "=a"(ax) : "q"(divisor), "a"(ax));
	set_reg<int16_t>(i,0,ax);
    }

    // IDIV E?

    template <> inline void group_3<int64_t>::idiv(interpreter & i) {
	int64_t rax = get_reg<int64_t>(i,0);
	int64_t rdx = get_reg<int64_t>(i,2);
	int64_t idivisor = E<int64_t>(i);
	__asm__("idivq %2" : "=a"(rax),"=d"(rdx) : "q"(idivisor), "a"(rax), "d"(rdx));
	set_reg<int64_t>(i,0,rax);
	set_reg<int64_t>(i,2,rdx);
    }

    template <> inline void group_3<int32_t>::idiv(interpreter & i) {
	int32_t eax = get_reg<int32_t>(i,0);
	int32_t edx = get_reg<int32_t>(i,2);
	int32_t idivisor = E<int32_t>(i);
	__asm__("idivl %2" : "=a"(eax),"=d"(edx) : "q"(idivisor), "a"(eax), "d"(edx));
	set_reg<int32_t>(i,0,eax);
	set_reg<int32_t>(i,2,edx);
    }

    template <> inline void group_3<int16_t>::idiv(interpreter & i) {
	int16_t ax = get_reg<int16_t>(i,0);
	int16_t dx = get_reg<int16_t>(i,2);
	int16_t idivisor = E<int16_t>(i);
	__asm__("idivw %2" : "=a"(ax),"=d"(dx) : "q"(idivisor), "a"(ax), "d"(dx));
	set_reg<int16_t>(i,0,ax);
	set_reg<int16_t>(i,2,dx);
    }

    template <> inline void group_3<int8_t>::idiv(interpreter & i) {
	int16_t ax = get_reg<int64_t>(i,0);
	int8_t idivisor = E<int64_t>(i);
	__asm__("idivb %1" : "=a"(ax) : "q"(idivisor), "a"(ax));
	set_reg<int16_t>(i,0,ax);
    }
    

    // MUL E?

    template <> inline void group_3<int64_t>::mul(interpreter & i) {
	int64_t rax = get_reg<int64_t>(i,0);
	int64_t rdx;
	int64_t multiplier = E<int64_t>(i);
	int8_t o, c;
	__asm__("mulq %4\n\tseto %2\n\tsetc %3" : "=a"(rax),"=d"(rdx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(rax));
	set_reg<int64_t>(i,0,rax);
	set_reg<int64_t>(i,2,rdx);
	i.of(o); i.cf(c); 
    }

    template <> inline void group_3<int32_t>::mul(interpreter & i) {
	int32_t eax = get_reg<int32_t>(i,0);
	int32_t edx;
	int32_t multiplier = E<int32_t>(i);
	int8_t o, c;
	__asm__("mul %4\n\tseto %2\n\tsetc %3" : "=a"(eax), "=d"(edx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(eax));
	set_reg<int32_t>(i,0,eax);
	set_reg<int32_t>(i,2,edx);
	i.of(o); i.cf(c); 
    }

    template <> inline void group_3<int16_t>::mul(interpreter & i) {
	int16_t ax = get_reg<int16_t>(i,0);
	int16_t dx;
	int16_t multiplier = E<int16_t>(i);
	int8_t o, c;
	__asm__("mulw %4\n\tseto %2\n\tsetc %3" : "=a"(ax), "=d"(dx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(ax));
	set_reg<int16_t>(i,0,ax);
	set_reg<int16_t>(i,2,dx);
	i.of(o); i.cf(c); 
    }

    template <> inline void group_3<int8_t>::mul(interpreter & i) {
	int8_t al = get_reg<int8_t>(i,0);
	int16_t ax;
	int8_t multiplier = E<int8_t>(i);
	int8_t o, c;
	__asm__("mulb %3\n\tseto %1\n\tsetc %2" : "=a"(ax), "=q"(o), "=q"(c) : "q"(multiplier), "a"(al));
	set_reg<int16_t>(i,0,ax);
	i.of(o); i.cf(c); 
    }

    // IMUL E?

    template <> inline void group_3<int64_t>::imul(interpreter & i) {
	int64_t rax = get_reg<int64_t>(i,0);
	int64_t rdx;
	int64_t multiplier = E<int64_t>(i);
	int8_t o, c;
	__asm__("imulq %4\n\tseto %2\n\tsetc %3" : "=a"(rax),"=d"(rdx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(rax));
	set_reg<int64_t>(i,0,rax);
	set_reg<int64_t>(i,2,rdx);
	i.of(o); i.cf(c); 
    }

    template <> inline void group_3<int32_t>::imul(interpreter & i) {
	int32_t eax = get_reg<int32_t>(i,0);
	int32_t edx;
	int32_t multiplier = E<int32_t>(i);
	int8_t o, c;
	__asm__("imul %4\n\tseto %2\n\tsetc %3" : "=a"(eax), "=d"(edx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(eax));
	set_reg<int32_t>(i,0,eax);
	set_reg<int32_t>(i,2,edx);
	i.of(o); i.cf(c); 
    }

    template <> inline void group_3<int16_t>::imul(interpreter & i) {
	int16_t ax = get_reg<int16_t>(i,0);
	int16_t dx;
	int16_t multiplier = E<int16_t>(i);
	int8_t o, c;
	__asm__("imulw %4\n\tseto %2\n\tsetc %3" : "=a"(ax), "=d"(dx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(ax));
	set_reg<int16_t>(i,0,ax);
	set_reg<int16_t>(i,2,dx);
	i.of(o); i.cf(c); 
    }

    template <> inline void group_3<int8_t>::imul(interpreter & i) {
	int8_t al = get_reg<int8_t>(i,0);
	int16_t ax;
	int8_t multiplier = E<int8_t>(i);
	int8_t o, c;
	__asm__("imulb %3\n\tseto %1\n\tsetc %2" : "=a"(ax), "=q"(o), "=q"(c) : "q"(multiplier), "a"(al));
	set_reg<int16_t>(i,0,ax);
	i.of(o); i.cf(c); 
    }
} // namespace jitpp

#endif
