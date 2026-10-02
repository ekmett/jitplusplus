# Instruction encoding metadata

`interpreter::encoding_lut` in [interpreter.ccm](../jit++/interpreter.ccm)
lives beside the decoder state, prefix helpers, and `parse()`. It describes byte layout for the primary and `0F`
opcode maps in 64-bit mode. Its 512 entries use Clang's GNU array/range
initializers, with named recipes for opcode families. Unspecified entries are
zero; the runtime performs one indexed lookup. A `consteval` initializer lambda
returns the `constexpr std::array<uint8_t, 512>`, keeping the recipe names local
to the initializer rather than adding class members.

```cpp
[0x00 ... 0x03] = MR, [0x04] = B, [0x05] = Z, // ADD
[0x08 ... 0x0b] = MR, [0x0c] = B, [0x0d] = Z, // OR
[0x50 ... 0x5f] = D,                        // PUSH/POP registers
```

Ranges do not overlap. The C99-designator warning is suppressed locally for
this intentional Clang extension; initializer-override warnings remain enabled.

`MR` means ModR/M; `B` and `Z` select immediate forms; `D` selects the default
64-bit operand size; `H` selects an explicit irregular form in `parse()`.
Other named constants cover fixed imm16, operand-sized immediates, ENTER's
imm16/imm8 pair, and the extra opcode byte in `0F 38`/`0F 3A`.

The table describes byte consumption, not instruction validity or interpreter
support. A zero entry means no additional bytes are described; it does not mean
that an opcode is valid. `interpreter::interpret_opcode()` remains responsible for execution or fallback.

## Cases that need more than an opcode byte

| Form | Selector |
| --- | --- |
| `A0`–`A3` moffs | Address size selects a 32- or 64-bit address |
| `F6`/`F7` group 3 | ModR/M selects whether TEST has an immediate |
| `FF` group 5 | ModR/M selects the default operand width |
| `0F 78` | The `66` and `F2` SSE4a forms carry two immediate bytes |

Keep these explicit. A single opcode entry cannot encode all prefix and group
variants. The unused DREX metadata from the abandoned SSE5 encoding has been
removed. This table does not describe VEX, EVEX, XOP, or APX encodings.

## Populating and checking the table

Add an instruction family by its encoding rule, or a named exception for an
irregular form. Check the rule against the architecture's opcode maps and
instruction reference: [AMD volume 3](https://www.amd.com/content/dam/amd/en/documents/processor-tech-docs/programmer-references/24594.pdf)
covers general-purpose encodings, [AMD volume 4](https://www.amd.com/content/dam/amd/en/documents/processor-tech-docs/programmer-references/26568.pdf)
covers SSE4a's immediate forms, and the [Intel manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
cover Intel-specific definitions. Keep vendor differences explicit when they
matter; agreement with a decoder is supporting evidence, not the specification.

`test_decoder` generates primary/0F opcodes, every candidate ModR/M byte, and
six prefix settings, then compares the parsed length with udis86. Invalid forms
reported by the oracle are skipped. The pinned udis86 revision recognizes
624,399 probes in this sweep; the corrected table has no length mismatches.
Two direct checks cover the SSE4a immediate forms absent from that oracle.
There is no stored fixture corpus.

Run it through CTest or directly:

```sh
ctest --test-dir build -R decoder --output-on-failure
./build/test_decoder
```

The sweep does not establish complete decoding or correct execution. It uses
zero padding and single prefixes; repeated-prefix precedence, malformed/truncated
inputs, the architectural length limit, and newer encoding maps still require
work. Operand widths and effective-address semantics need checks beyond matching
instruction lengths. The round-trip suite covers the native transition and separately checks RET's
immediate byte count by running the interpreter directly.
