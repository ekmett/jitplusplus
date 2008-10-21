#define REG_OFFSET_RAX    0x10
#define REG_OFFSET_RCX    0x18
#define REG_OFFSET_RDX    0x20
#define REG_OFFSET_RBX    0x28
#define REG_OFFSET_RSP    0x30
#define REG_OFFSET_RBP    0x38
#define REG_OFFSET_RSI    0x40
#define REG_OFFSET_RDI    0x48
#define REG_OFFSET_R8     0x50
#define REG_OFFSET_R9     0x58
#define REG_OFFSET_R10    0x60
#define REG_OFFSET_R11    0x68
#define REG_OFFSET_R12    0x70
#define REG_OFFSET_R13    0x78
#define REG_OFFSET_R14    0x80
#define REG_OFFSET_R15    0x88
#define REG_OFFSET_RIP    0x90
#define REG_OFFSET_RFLAGS 0x98
#define FXREGS_OFFSET     0xa0
#define STACK_OFFSET      0x2a0
#define STACK_SIZE_OFFSET 0x2a8

/* argument %rdi points to the tracer object */

    .text
    .align 16
    .globl _ZN5jitpp6tracer5startEv
    .type  _ZN5jitpp6tracer5startEv, @function
    /* namespace jitpp { class tracer { virtual void run() = 0; ...; void NOTHROW start(); }; }  */
_ZN5jitpp6tracer5startEv:
    movq %r12, REG_OFFSET_R12(%rdi)
    movq %r13, REG_OFFSET_R13(%rdi)
    movq %r14, REG_OFFSET_R14(%rdi)
    movq %r15, REG_OFFSET_R15(%rdi)
    movq %rbp, REG_OFFSET_RBP(%rdi)
    movq %rbx, REG_OFFSET_RBX(%rdi)

    /* stmxcsr (FXREGS_OFFSET + 24)(%rdi); fstcw FXREGS_OFFSET(%rdi); then clear the reserved bits? */
    fxsave FXREGS_OFFSET(%rdi)
    
    /* save off the stack pointer minus the overhead for this call */
    leaq 8(%rsp), %rax
    movq %rax, REG_OFFSET_RSP(%rdi)

    /* save instruction pointer that this call would return to */
    mov 0(%rsp), %rax
    movq %rax, REG_OFFSET_RIP(%rdi)

    /* save the rflags we were called with */
    pushfq
    popq %rax
    movq %rax, REG_OFFSET_RFLAGS(%rdi)

    /* swich to the new stack */
    movq STACK_OFFSET(%rdi), %rax
    addq STACK_SIZE_OFFSET(%rdi), %rax
    movq %rax, %rsp

    /* callee retains 'this' for us in r15 */
    movq %rdi, %r15
    movq (%rdi), %r13 /* virtual run() */
    call *(%r13)
    mov %r15, %rdi
    /* fall through */

    .globl _ZN5jitpp6tracer4stopEv
    .type  _ZN5jitpp6tracer4stopEv, @function
_ZN5jitpp6tracer4stopEv:    
    fxrstor FXREGS_OFFSET(%rdi)

    movq %ss, %rax
    pushq %rax 	                   /* target ss */
    pushq REG_OFFSET_RSP(%rdi)     /* target rsp */
    pushq REG_OFFSET_RFLAGS(%rdi)  /* target rflags */
    movq %cs, %rax
    pushq %rax                     /* target cs */
    pushq REG_OFFSET_RIP(%rdi)     /* target rip */

    movq REG_OFFSET_RAX(%rdi), %rax
    movq REG_OFFSET_RCX(%rdi), %rcx
    movq REG_OFFSET_RDX(%rdi), %rdx
    movq REG_OFFSET_RBX(%rdi), %rbx
    /* skip rsp */
    movq REG_OFFSET_RBP(%rdi), %rbp
    /* skip rdi */
    movq REG_OFFSET_RSI(%rdi), %rsi
    movq REG_OFFSET_R8(%rdi),  %r8
    movq REG_OFFSET_R9(%rdi),  %r9
    movq REG_OFFSET_R10(%rdi), %r10
    movq REG_OFFSET_R11(%rdi), %r11
    movq REG_OFFSET_R12(%rdi), %r12
    movq REG_OFFSET_R13(%rdi), %r13
    movq REG_OFFSET_R14(%rdi), %r14
    movq REG_OFFSET_R15(%rdi), %r15

    movq REG_OFFSET_RDI(%rdi), %rdi
    iretq
    /* cs:rip, ss:rsp, and rflags restored by iret */

.section .note.GNU-stack,"",%progbits
