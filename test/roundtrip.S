// SPDX-FileCopyrightText: 2026 Edward Kmett <ekmett@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR Apache-2.0

    .text
    .globl roundtrip
    .type roundtrip, @function
roundtrip:
    pushq %rbx
    pushq %r12
    subq $8, %rsp
    movq %rsi, %r12
    call _ZN5jitpp11interpreter5startEv@PLT
    movl $-1, %eax
    addl $1, %eax
    .globl roundtrip_step_limit
roundtrip_step_limit:
    movb $1, (%r12)
    .globl roundtrip_stopped
roundtrip_stopped:
    pushfq
    popq %rax
    pushq %rax
    xorl %eax, %eax
    .globl roundtrip_unsupported
roundtrip_unsupported:
    cpuid
    popq %rax
    addq $8, %rsp
    popq %r12
    popq %rbx
    ret
    .size roundtrip, .-roundtrip
    .section .note.GNU-stack,"",@progbits
