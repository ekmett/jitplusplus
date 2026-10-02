#ifndef INCLUDED_JITPP_INTERPRETING_GROUP_3_H
#define INCLUDED_JITPP_INTERPRETING_GROUP_3_H

#include <jit++/interpreter.h>

namespace jitpp {

    // DIV E?
    //
    template <> inline void interpreter::div<int64_t>() {
    int64_t rax = get_reg<int64_t>(0);
    int64_t rdx = get_reg<int64_t>(2);
    int64_t divisor = E<int64_t>();
    __asm__("divq %2" : "=a"(rax),"=d"(rdx) : "q"(divisor), "a"(rax), "d"(rdx));
    set_reg<int64_t>(0,rax);
    set_reg<int64_t>(2,rdx);
    }

    template <> inline void interpreter::div<int32_t>() {
    int32_t eax = get_reg<int32_t>(0);
    int32_t edx = get_reg<int32_t>(2);
    int32_t divisor = E<int32_t>();
    __asm__("divl %2" : "=a"(eax),"=d"(edx) : "q"(divisor), "a"(eax), "d"(edx));
    set_reg<int32_t>(0,eax);
    set_reg<int32_t>(2,edx);
    }

    template <> inline void interpreter::div<int16_t>() {
    int16_t ax = get_reg<int16_t>(0);
    int16_t dx = get_reg<int16_t>(2);
    int16_t divisor = E<int16_t>();
    __asm__("divw %2" : "=a"(ax),"=d"(dx) : "q"(divisor), "a"(ax), "d"(dx));
    set_reg<int16_t>(0,ax);
    set_reg<int16_t>(2,dx);
    }

    template <> inline void interpreter::div<int8_t>() {
    int16_t ax = get_reg<int64_t>(0);
    int8_t divisor = E<int64_t>();
    __asm__("divb %1" : "=a"(ax) : "q"(divisor), "a"(ax));
    set_reg<int16_t>(0,ax);
    }

    // IDIV E?

    template <> inline void interpreter::idiv<int64_t>() {
    int64_t rax = get_reg<int64_t>(0);
    int64_t rdx = get_reg<int64_t>(2);
    int64_t idivisor = E<int64_t>();
    __asm__("idivq %2" : "=a"(rax),"=d"(rdx) : "q"(idivisor), "a"(rax), "d"(rdx));
    set_reg<int64_t>(0,rax);
    set_reg<int64_t>(2,rdx);
    }

    template <> inline void interpreter::idiv<int32_t>() {
    int32_t eax = get_reg<int32_t>(0);
    int32_t edx = get_reg<int32_t>(2);
    int32_t idivisor = E<int32_t>();
    __asm__("idivl %2" : "=a"(eax),"=d"(edx) : "q"(idivisor), "a"(eax), "d"(edx));
    set_reg<int32_t>(0,eax);
    set_reg<int32_t>(2,edx);
    }

    template <> inline void interpreter::idiv<int16_t>() {
    int16_t ax = get_reg<int16_t>(0);
    int16_t dx = get_reg<int16_t>(2);
    int16_t idivisor = E<int16_t>();
    __asm__("idivw %2" : "=a"(ax),"=d"(dx) : "q"(idivisor), "a"(ax), "d"(dx));
    set_reg<int16_t>(0,ax);
    set_reg<int16_t>(2,dx);
    }

    template <> inline void interpreter::idiv<int8_t>() {
    int16_t ax = get_reg<int64_t>(0);
    int8_t idivisor = E<int64_t>();
    __asm__("idivb %1" : "=a"(ax) : "q"(idivisor), "a"(ax));
    set_reg<int16_t>(0,ax);
    }


    // MUL E?

    template <> inline void interpreter::mul<int64_t>() {
    int64_t rax = get_reg<int64_t>(0);
    int64_t rdx;
    int64_t multiplier = E<int64_t>();
    int8_t o, c;
    __asm__("mulq %4\n\tseto %2\n\tsetc %3" : "=a"(rax),"=d"(rdx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(rax));
    set_reg<int64_t>(0,rax);
    set_reg<int64_t>(2,rdx);
    of(o); cf(c);
    }

    template <> inline void interpreter::mul<int32_t>() {
    int32_t eax = get_reg<int32_t>(0);
    int32_t edx;
    int32_t multiplier = E<int32_t>();
    int8_t o, c;
    __asm__("mul %4\n\tseto %2\n\tsetc %3" : "=a"(eax), "=d"(edx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(eax));
    set_reg<int32_t>(0,eax);
    set_reg<int32_t>(2,edx);
    of(o); cf(c);
    }

    template <> inline void interpreter::mul<int16_t>() {
    int16_t ax = get_reg<int16_t>(0);
    int16_t dx;
    int16_t multiplier = E<int16_t>();
    int8_t o, c;
    __asm__("mulw %4\n\tseto %2\n\tsetc %3" : "=a"(ax), "=d"(dx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(ax));
    set_reg<int16_t>(0,ax);
    set_reg<int16_t>(2,dx);
    of(o); cf(c);
    }

    template <> inline void interpreter::mul<int8_t>() {
    int8_t al = get_reg<int8_t>(0);
    int16_t ax;
    int8_t multiplier = E<int8_t>();
    int8_t o, c;
    __asm__("mulb %3\n\tseto %1\n\tsetc %2" : "=a"(ax), "=q"(o), "=q"(c) : "q"(multiplier), "a"(al));
    set_reg<int16_t>(0,ax);
    of(o); cf(c);
    }

    // IMUL E?

    template <> inline void interpreter::imul<int64_t>() {
    int64_t rax = get_reg<int64_t>(0);
    int64_t rdx;
    int64_t multiplier = E<int64_t>();
    int8_t o, c;
    __asm__("imulq %4\n\tseto %2\n\tsetc %3" : "=a"(rax),"=d"(rdx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(rax));
    set_reg<int64_t>(0,rax);
    set_reg<int64_t>(2,rdx);
    of(o); cf(c);
    }

    template <> inline void interpreter::imul<int32_t>() {
    int32_t eax = get_reg<int32_t>(0);
    int32_t edx;
    int32_t multiplier = E<int32_t>();
    int8_t o, c;
    __asm__("imul %4\n\tseto %2\n\tsetc %3" : "=a"(eax), "=d"(edx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(eax));
    set_reg<int32_t>(0,eax);
    set_reg<int32_t>(2,edx);
    of(o); cf(c);
    }

    template <> inline void interpreter::imul<int16_t>() {
    int16_t ax = get_reg<int16_t>(0);
    int16_t dx;
    int16_t multiplier = E<int16_t>();
    int8_t o, c;
    __asm__("imulw %4\n\tseto %2\n\tsetc %3" : "=a"(ax), "=d"(dx), "=q"(o), "=q"(c) : "q"(multiplier), "a"(ax));
    set_reg<int16_t>(0,ax);
    set_reg<int16_t>(2,dx);
    of(o); cf(c);
    }

    template <> inline void interpreter::imul<int8_t>() {
    int8_t al = get_reg<int8_t>(0);
    int16_t ax;
    int8_t multiplier = E<int8_t>();
    int8_t o, c;
    __asm__("imulb %3\n\tseto %1\n\tsetc %2" : "=a"(ax), "=q"(o), "=q"(c) : "q"(multiplier), "a"(al));
    set_reg<int16_t>(0,ax);
    of(o); cf(c);
    }

} // namespace jitpp

#endif // INCLUDED_JITPP_INTERPRETING_GROUP_3_H
