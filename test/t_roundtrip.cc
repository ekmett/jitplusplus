// SPDX-FileCopyrightText: 2026 Edward Kmett <ekmett@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR Apache-2.0

#include <gflags/gflags.h>
import jitpp;
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <type_traits>

static_assert(std::is_standard_layout_v<jitpp::interpreter>,
              "assembly state must have a standard layout");
static_assert(!std::is_polymorphic_v<jitpp::interpreter>,
              "the concrete interpreter needs no virtual dispatch");

DECLARE_uint64(jitpp_steps);

extern "C" uint64_t roundtrip(jitpp::interpreter *, bool *);
extern "C" char roundtrip_stopped[], roundtrip_unsupported[];
extern "C" char roundtrip_step_limit[];

static void require(bool condition, const char * message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

int main(int argc, char ** argv) {
    jitpp::application app(argc, argv);
    jitpp::interpreter interpreter;
    for (unsigned iteration = 0; iteration != 2; ++iteration) {
        FLAGS_jitpp_steps = -1;
        uint64_t flags = roundtrip(&interpreter, &interpreter.m_stopped);
        require(interpreter.rip() == reinterpret_cast<int64_t>(roundtrip_stopped),
                "arithmetic and stop must execute under interpretation");
        require(interpreter.rax() == 0, "interpreted addition result");
        require((flags & 0x8d5) == 0x55, "arithmetic flags survive native resumption");
    }
    // Stop after the addition, before the explicit stop store.
    FLAGS_jitpp_steps = 2;
    uint64_t flags = roundtrip(&interpreter, &interpreter.m_stopped);
    require(interpreter.rip() == reinterpret_cast<int64_t>(roundtrip_step_limit),
            "step limit resumes immediately after the second instruction");
    require(interpreter.rax() == 0, "step-limited interpreted addition result");
    require((flags & 0x8d5) == 0x55, "step-limit exit preserves flags");

    // With a separate stop flag, CPUID is the first unsupported instruction.
    bool ignored_stop = false;
    FLAGS_jitpp_steps = -1;
    roundtrip(&interpreter, &ignored_stop);
    require(interpreter.rip() == reinterpret_cast<int64_t>(roundtrip_unsupported),
            "unsupported instruction resumes at its original address");
    require(ignored_stop, "interpreted store reaches native memory");

    // Cross the former inheritance branches while sharing registers and memory.
    const unsigned char groups[] = {
        0xb8, 7, 0, 0, 0,        // mov $7, %eax
        0x83, 0xc0, 5,           // group 1: add $5, %eax -> 12
        0xc1, 0xe0, 1,           // group 2: shl $1, %eax -> 24
        0xf7, 0xd0,              // group 3: not %eax -> 0xffffffe7
        0xfe, 0xc0,              // group 4: inc %al -> 0xffffffe8
        0x48, 0xff, 0xc8,        // group 5: dec %rax -> 0xffffffe7
        0x48, 0x92,              // misc: xchg %rax, %rdx
        0xb8, 7, 0, 0, 0,        // mov $7, %eax
        0xf0, 0x0f, 0xb1, 0x1e,  // locked: cmpxchg %ebx, (%rsi)
        0xc6, 0x07, 1            // movb $1, (%rdi): stop
    };
    int32_t memory = 7;
    interpreter.rbx() = 9;
    interpreter.rdx() = 42;
    interpreter.rsi() = reinterpret_cast<int64_t>(&memory);
    interpreter.rdi() = reinterpret_cast<int64_t>(&interpreter.m_stopped);
    interpreter.rip() = reinterpret_cast<int64_t>(groups);
    interpreter.rflags(2);
    interpreter.run();
    require(interpreter.rip() == reinterpret_cast<int64_t>(groups + sizeof(groups)),
            "all instruction groups finish without fallback");
    require(interpreter.rdx() == 0xffffffe7LL && interpreter.rax() == 7 && memory == 9,
            "instruction groups share register and memory state");

    // Check low-byte parity against the CPU, including ignored high bits.
    for (unsigned byte = 0; byte != 256; ++byte) {
        const auto value = static_cast<uint8_t>(byte);
        uint8_t even;
        asm("testb %1, %1; setp %0" : "=qm"(even) : "q"(value) : "cc");
        interpreter.and_(0x100 | byte, 0x100 | byte);
        require(interpreter.pf() == static_cast<bool>(even), "parity matches native flags");
    }

    // RET's immediate counts bytes, independently of its return-address width.
    const unsigned char ret[] = {0xc2, 0x10, 0x00};
    int64_t stack[4] = {reinterpret_cast<int64_t>(groups)};
    interpreter.rsp() = reinterpret_cast<int64_t>(stack);
    interpreter.rip() = reinterpret_cast<int64_t>(ret);
    FLAGS_jitpp_steps = 1;
    interpreter.run();
    require(interpreter.rip() == stack[0] &&
            interpreter.rsp() == reinterpret_cast<int64_t>(stack) + 8 + 16,
            "RET imm16 pops the return address and releases the encoded byte count");
    std::puts("PASS: interpreted arithmetic, flags, repeated entry, step limit, native fallback");
}
