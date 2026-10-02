# Source and definition ownership

`jitpp` is the sole module export provider. It exports `application` and
`interpreter`; supporting headers are textual implementation inputs. Ordinary
translation units use `.cc`, assembly uses `.S`, and the interface uses `.ccm`.

| Source | Responsibility |
| --- | --- |
| `../jitpp.ccm` | Public module and global-fragment exports |
| `interpreter.h` | Concrete interpreter state and inline instruction methods |
| `interpreter.cc` | Construction and worker-thread entry |
| `interpreting/tracer_start.S` | Native register capture and resumption |
| `interpreting/decoder.cc` | Encoding metadata and instruction parsing |
| `interpreting/run.cc` | Decode/execute loop and fallback |
| `interpreting/opcode.h`, `interpreting/locked.h` | Instruction dispatch |
| `interpreting/base.*` | Register/memory access and diagnostics |
| `interpreting/flags.*`, `interpreting/traits.h` | Lazy flags and operand traits |
| `application.*` | Runtime logging and option initialization |

## Module ownership and assembly

The interface includes the interpreter declaration in its global module
fragment, then exports namespace using-declarations. This follows `native.isa`'s
provider pattern: declarations retain their global ownership and C++ linkage.
Assembly can reference `interpreter::start()` and `interpreter::stub()` directly.
There is no C bridge or separately allocated implementation object.

The concrete class is standard-layout. Static assertions pin the offsets used
by assembly for registers, instruction pointer, flags, floating-point state and
stack bookkeeping. Layout changes must update the assembly and these assertions
together. System and dependency headers also remain in the global fragment.

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

The [decoder guide](../docs/decoding.md) describes the metadata boundary.
