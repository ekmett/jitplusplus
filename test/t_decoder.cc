#include <array>
#include <cstdint>
#include <cstdio>
#include <udis86.h>
import jitpp;

// Probe encoding shapes, not execution semantics. Invalid encodings in the
// development oracle are skipped; this is not a claim of complete ISA coverage.
int main(int argc, char **argv) {
    jitpp::application app(argc, argv);
    jitpp::interpreter machine;
    unsigned checked = 0, failures = 0, reported = 0;
    std::array<bool, 512> reported_opcode{};
    for (unsigned prefix : {0u, 0x66u, 0x67u, 0x48u, 0xf2u, 0xf3u}) {
        for (unsigned opcode = 0; opcode < 512; ++opcode) {
            // These bytes introduce other opcode maps or prefixes.
            if (opcode < 256 && (opcode == 0x0f || opcode == 0x26 || opcode == 0x2e ||
                opcode == 0x36 || opcode == 0x3e || (opcode >= 0x40 && opcode <= 0x4f) ||
                opcode == 0x64 || opcode == 0x65 || opcode == 0x66 || opcode == 0x67 ||
                opcode == 0xf0 || opcode == 0xf2 || opcode == 0xf3)) continue;
            for (unsigned modrm = 0; modrm < 256; ++modrm) {
                std::array<uint8_t, 32> bytes{};
                unsigned n = 0;
                if (prefix) bytes[n++] = prefix;
                if (opcode >= 256) bytes[n++] = 0x0f;
                bytes[n++] = opcode;
                bytes[n] = modrm;
                ud_t oracle;
                ud_init(&oracle);
                ud_set_mode(&oracle, 64);
                ud_set_input_buffer(&oracle, bytes.data(), bytes.size());
                unsigned expected = ud_disassemble(&oracle);
                if (!expected || ud_insn_mnemonic(&oracle) == UD_Iinvalid) continue;
                auto start = reinterpret_cast<int64_t>(bytes.data());
                auto actual = machine.parse(start) - start;
                ++checked;
                if (actual != expected) {
                    ++failures;
                    if (!reported_opcode[opcode] && reported++ < 12)
                        std::fprintf(stderr, "prefix=%02x opcode=%03x modrm=%02x: length %lld, oracle %u\n",
                                     prefix, opcode, modrm, static_cast<long long>(actual), expected);
                    reported_opcode[opcode] = true;
                }
            }
        }
    }
    // The old oracle lacks SSE4a: check its two immediate bytes directly.
    for (uint8_t prefix : {0x66, 0xf2}) {
        std::array<uint8_t, 16> bytes{prefix, 0x0f, 0x78, 0xc0, 0x12, 0x34};
        auto start = reinterpret_cast<int64_t>(bytes.data());
        if (machine.parse(start) - start != 6 || !machine.has_imm() ||
            !machine.has_imm2() || machine.imm != 0x12 || machine.imm2 != 0x34) {
            std::fputs("SSE4a immediate layout mismatch\n", stderr);
            ++failures;
        }
    }
    std::printf("decoder: %u oracle encodings checked, %u mismatches\n", checked, failures);
    return failures != 0 || checked == 0;
}
