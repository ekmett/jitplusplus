#ifndef INCLUDED_JITPP_INTERPRETING_GROUP_3_H
#define INCLUDED_JITPP_INTERPRETING_GROUP_3_H

#include <jit++/common.h>
#include <jit++/interpreting/base.h>
#include <jit++/interpreting/flags.h>
#include <jit++/interpreting/group_1.h> // AND

namespace jitpp { 
  namespace interpreting { 
    using namespace jitpp::interpreting::flags;

    template <typename T>
    struct group_3_traits { 
        static inline bool neg_cf(const context & ctx) {
            return ctx.result != 0;
        }
        static inline bool neg_af(const context & ctx) {
            return (ctx.result & 0xf) != 0;
        }
        static inline bool neg_of(const context & ctx) {
            return ctx.result == 1ULL << (sizeof(T)*8 - 1);
        }
	static const flags::handler neg_flags; 
    };
    template <typename T> const flags::handler group_3_traits<T>::neg_flags = { neg_cf, neg_af, neg_of, flags::OSZAPC, 0 };


    class group_3 : private virtual group_1 {
    private:
	template <typename T> inline int64_t neg(int64_t x) {
            return handle_rflags(group_3_traits<T>::neg_flags,-x);
        }
	template <typename T> inline void mul();
	template <typename T> inline void imul();
	template <typename T> inline void div();
	template <typename T> inline void idiv();

    public:
	template <typename T>
	inline void interpret_group_3() { 
            switch (reg) {
            case 0 ... 1: and_(E<T>(),imm); return; // TEST E?, I?*
	    case 2: E<T>(~E<T>()); return; // NOT E?
            case 3: E<T>(neg<T>(E<T>())); return; // NEG E?
            case 4: mul<T>(); return;  // MUL E?
            case 5: imul<T>(); return; // IMUL E?
            case 6: div<T>(); return;  // DIV E?
            case 7: idiv<T>(); return; // DIV E?
            default: unsupported();
            }
	}
    };
    // DIV E?
    //
    template <> inline void group_3::div<int64_t>() {
	int64_t rax = get_reg<int64_t>(0);
	int64_t rdx = get_reg<int64_t>(2);
	int64_t divisor = E<int64_t>();
	__asm__("divq %2" : "=a"(rax),"=d"(rdx) : "q"(divisor), "a"(rax), "d"(rdx));
	set_reg<int64_t>(0,rax);
	set_reg<int64_t>(2,rdx);
    }

    template <> inline void group_3::div<int32_t>() {
	int32_t eax = get_reg<int32_t>(0);
	int32_t edx = get_reg<int32_t>(2);
	int32_t divisor = E<int32_t>();
	__asm__("divl %2" : "=a"(eax),"=d"(edx) : "q"(divisor), "a"(eax), "d"(edx));
	set_reg<int32_t>(0,eax);
	set_reg<int32_t>(2,edx);
    }

    template <> inline void group_3::div<int16_t>() {
	int16_t ax = get_reg<int16_t>(0);
	int16_t dx = get_reg<int16_t>(2);
	int16_t divisor = E<int16_t>();
	__asm__("divw %2" : "=a"(ax),"=d"(dx) : "q"(divisor), "a"(ax), "d"(dx));
	set_reg<int16_t>(0,ax);
	set_reg<int16_t>(2,dx);
    }

    template <> inline void group_3::div<int8_t>() {
	int16_t ax = get_reg<int64_t>(0);
	int8_t divisor = E<int64_t>();
	__asm__("divb %1" : "=a"(ax) : "q"(divisor), "a"(ax));
	set_reg<int16_t>(0,ax);
    }

    // IDIV E?

    template <> inline void group_3::idiv<int64_t>() {
	int64_t rax = get_reg<int64_t>(0);
	int64_t rdx = get_reg<int64_t>(2);
	int64_t idivisor = E<int64_t>();
	__asm__("idivq %2" : "=a"(rax),"=d"(rdx) : "q"(idivisor), "a"(rax), "d"(rdx));
	set_reg<int64_t>(0,rax);
	set_reg<int64_t>(2,rdx);
    }

    template <> inline void group_3::idiv<int32_t>() {
	int32_t eax = get_reg<int32_t>(0);
	int32_t edx = get_reg<int32_t>(2);
	int32_t idivisor = E<int32_t>();
	__asm__("idivl %2" : "=a"(eax),"=d"(edx) : "q"(idivisor), "a"(eax), "d"(edx));
	set_reg<int32_t>(0,eax);
	set_reg<int32_t>(2,edx);
    }

    template <> inline void group_3::idiv<int16_t>() {
	int16_t ax = get_reg<int16_t>(0);
	int16_t dx = get_reg<int16_t>(2);
	int16_t idivisor = E<int16_t>();
	__asm__("idivw %2" : "=a"(ax),"=d"(dx) : "q"(idivisor), "a"(ax), "d"(dx));
	set_reg<int16_t>(0,ax);
	set_reg<int16_t>(2,dx);
    }

    template <> inline void group_3::idiv<int8_t>() {
	int16_t ax = get_reg<int64_t>(0);
	int8_t idivisor = E<int64_t>();
	__asm__("idivb %1" : "=a"(ax) : "q"(idivisor), "a"(ax));
	set_reg<int16_t>(0,ax);
    }
    

    // MUL E?

    template <> inline void group_3::mul<int64_t>() {
	int64_t rax = get_reg<int64_t>(0);
	int64_t rdx;
	int64_t multiplier = E<int64_t>();
	int8_t o, c;
	__asm__("mulq %4\n\tseto %2\n\tsetc %3" : "=a"(rax),"=d"(rdx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(rax));
	set_reg<int64_t>(0,rax);
	set_reg<int64_t>(2,rdx);
	of(o); cf(c); 
    }

    template <> inline void group_3::mul<int32_t>() {
	int32_t eax = get_reg<int32_t>(0);
	int32_t edx;
	int32_t multiplier = E<int32_t>();
	int8_t o, c;
	__asm__("mul %4\n\tseto %2\n\tsetc %3" : "=a"(eax), "=d"(edx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(eax));
	set_reg<int32_t>(0,eax);
	set_reg<int32_t>(2,edx);
	of(o); cf(c); 
    }

    template <> inline void group_3::mul<int16_t>() {
	int16_t ax = get_reg<int16_t>(0);
	int16_t dx;
	int16_t multiplier = E<int16_t>();
	int8_t o, c;
	__asm__("mulw %4\n\tseto %2\n\tsetc %3" : "=a"(ax), "=d"(dx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(ax));
	set_reg<int16_t>(0,ax);
	set_reg<int16_t>(2,dx);
	of(o); cf(c); 
    }

    template <> inline void group_3::mul<int8_t>() {
	int8_t al = get_reg<int8_t>(0);
	int16_t ax;
	int8_t multiplier = E<int8_t>();
	int8_t o, c;
	__asm__("mulb %3\n\tseto %1\n\tsetc %2" : "=a"(ax), "=q"(o), "=q"(c) : "q"(multiplier), "a"(al));
	set_reg<int16_t>(0,ax);
	of(o); cf(c); 
    }

    // IMUL E?

    template <> inline void group_3::imul<int64_t>() {
	int64_t rax = get_reg<int64_t>(0);
	int64_t rdx;
	int64_t multiplier = E<int64_t>();
	int8_t o, c;
	__asm__("imulq %4\n\tseto %2\n\tsetc %3" : "=a"(rax),"=d"(rdx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(rax));
	set_reg<int64_t>(0,rax);
	set_reg<int64_t>(2,rdx);
	of(o); cf(c); 
    }

    template <> inline void group_3::imul<int32_t>() {
	int32_t eax = get_reg<int32_t>(0);
	int32_t edx;
	int32_t multiplier = E<int32_t>();
	int8_t o, c;
	__asm__("imul %4\n\tseto %2\n\tsetc %3" : "=a"(eax), "=d"(edx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(eax));
	set_reg<int32_t>(0,eax);
	set_reg<int32_t>(2,edx);
	of(o); cf(c); 
    }

    template <> inline void group_3::imul<int16_t>() {
	int16_t ax = get_reg<int16_t>(0);
	int16_t dx;
	int16_t multiplier = E<int16_t>();
	int8_t o, c;
	__asm__("imulw %4\n\tseto %2\n\tsetc %3" : "=a"(ax), "=d"(dx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(ax));
	set_reg<int16_t>(0,ax);
	set_reg<int16_t>(2,dx);
	of(o); cf(c); 
    }

    template <> inline void group_3::imul<int8_t>() {
	int8_t al = get_reg<int8_t>(0);
	int16_t ax;
	int8_t multiplier = E<int8_t>();
	int8_t o, c;
	__asm__("imulb %3\n\tseto %1\n\tsetc %2" : "=a"(ax), "=q"(o), "=q"(c) : "q"(multiplier), "a"(al));
	set_reg<int16_t>(0,ax);
	of(o); cf(c); 
    }
  } // namespace interpreting
} // namespace jitpp

#endif
