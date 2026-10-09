<!--
SPDX-FileCopyrightText: 2026 Edward Kmett <ekmett@gmail.com>
SPDX-License-Identifier: BSD-2-Clause OR Apache-2.0
-->

# Source and definition ownership

`jitpp` is the sole public module. Its primary interface re-exports two interface
partitions; application code still writes only `import jitpp;`. Declarations and
implementations live together in `.ccm` files, without a project header layer.

| Source | Responsibility |
| --- | --- |
| [../jitpp.ccm](../jitpp.ccm) | Public module, re-exporting the two API partitions |
| [interpreter.ccm](interpreter.ccm) | Saved state, decoding, instruction execution, flags, diagnostics, and worker entry |
| [application.ccm](application.ccm) | Runtime logging and option initialization |
| [interpreting/tracer_start.S](interpreting/tracer_start.S) | Native register capture and resumption |
| [memory.ccm](memory.ccm) | Historical memory-permission and locking support, in an unimported internal partition |

## Finding interpreter code

`interpreter.ccm` starts with the exception, flag, and operand-size helpers.
The concrete class follows, with method bodies in its definition. Its sections
cover entry and execution, saved state, lazy flags, decoding, register/memory
access, instruction groups, miscellaneous instructions, and dispatch.

The decoder's state, prefix helpers, recipe constants, `encoding_lut`, and
`parse()` form one contiguous section. The [decoder guide](../docs/decoding.md)
describes the table and its validation boundary. Template specializations live
beside their primary declarations rather than in separate implementation headers.

## C++ conventions

The build uses C++26 mode on the supported Clang toolchain. Prefer `using`
aliases, `constexpr` data, defaulted/deleted special members, `nullptr`, and
standard attributes. Put static template data directly in its owning class.
Use standard library facilities when they remove custom machinery: parity uses
`std::popcount`, and lock ownership transfers with `std::exchange`.

Keep compile-time construction local to the data it builds, as with the
`consteval` decoder-table initializer. The raw saved-state arrays remain tied to
the assembly layout; their offsets are checked at compile time.

## Module ownership and assembly

The interpreter and its supporting definitions are written directly in the
global module fragment, above `export module jitpp:interpreter;`. The partition
then exports the class with `using ::jitpp::interpreter;`, following the global
ownership/provider pattern used by `native`.

This preserves the C++ symbols referenced by assembly: `interpreter::start()`
and `interpreter::stub()`. `stub()` has an out-of-class definition in the same
file so the compiler emits the symbol called by assembly. There is no C bridge
or separately allocated implementation object. System and dependency headers
also remain in the global fragment.

The concrete class is standard-layout. Static assertions pin the offsets used
by assembly for registers, instruction pointer, flags, floating-point state and
stack bookkeeping. Layout changes must update the assembly and these assertions
together.

`application` needs no assembly linkage and is defined directly in its named
module partition. The old memory-permission subsystem is not part of the exported
API or interpreter execution path. Its separate, unimported archive object keeps
its historical singleton initialization out of ordinary module consumers.

## Execution

`start()` saves the caller's state and invokes the worker through `stub()`.
`run()` parses an instruction, advances the saved RIP, and dispatches according
to the decoded operand size. Register and memory operations share the same
interpreter object. `stop()` ends the loop.

An unsupported instruction throws an interpreter exception. The loop restores
RIP to the start of that instruction and materializes lazy flags before the
assembly exit resumes native execution. A successful stop also materializes
flags. Instruction implementations must avoid partial state changes before
requesting fallback.
