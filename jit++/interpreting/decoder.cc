#include <iostream>
#include <jit++/common.h>
#include <jit++/interpreter.h>

namespace {
    template <typename T> T fetch(int64_t &i) {
        T result = *reinterpret_cast<const T *>(i);
        i += sizeof(result);
        return result;
    }
} // namespace

namespace jitpp {
    constinit const std::array<uint8_t, 512> interpreter::encoding_lut = [] {
        std::array<uint8_t, 512> table{};
        // Recipes describe bytes to consume, not whether execution is supported.
        constexpr auto M = encoding_has_modrm;
        constexpr auto B = encoding_Ib;
        constexpr auto Z = encoding_Iz;
        constexpr auto D = encoding_default_os_64;
        constexpr auto H = encoding_hard;
        auto range = [&](unsigned first, unsigned last, uint8_t recipe) {
            for (unsigned op = first; op <= last; ++op)
                table[op] = recipe;
        };
        auto set = [&](std::initializer_list<unsigned> ops, uint8_t recipe) {
            for (unsigned op : ops)
                table[op] = recipe;
        };

        // Primary map: arithmetic families, stack operations, branches and moves.
        for (unsigned op = 0; op <= 0x38; op += 8) {
            range(op, op + 3, M);
            table[op + 4] = B;
            table[op + 5] = Z;
        }
        range(0x50, 0x5f, D);
        table[0x63] = M;
        table[0x68] = D | Z;
        table[0x69] = M | Z;
        table[0x6a] = D | B;
        table[0x6b] = M | B;
        range(0x70, 0x7f, B);
        set({0x80, 0x82, 0x83}, M | B);
        table[0x81] = M | Z;
        range(0x84, 0x8e, M);
        table[0x8f] = M | D;
        set({0x9c, 0x9d}, D);
        range(0xa0, 0xa3, H); // moffs follows address size, not operand size
        table[0xa8] = B;
        table[0xa9] = Z;
        range(0xb0, 0xb7, B);
        range(0xb8, 0xbf, encoding_Iv);
        set({0xc0, 0xc1, 0xc6}, M | B);
        table[0xc2] = D | encoding_Iw;
        table[0xc3] = D;
        table[0xc7] = M | Z;
        table[0xc8] = D | encoding_IwIb;
        table[0xc9] = D;
        table[0xca] = encoding_Iw;
        table[0xcd] = B;
        range(0xd0, 0xd3, M);
        set({0xd4, 0xd5}, B);
        range(0xd8, 0xdf, M);
        range(0xe0, 0xe7, B);
        set({0xe8, 0xe9}, D | Z);
        table[0xeb] = B;
        set({0xf6, 0xf7}, M | H); // only TEST in group 3 has an immediate
        table[0xfe] = M;
        table[0xff] = M | D; // group 5 selects the default width in parse()

        // 0F map: system, SIMD and integer instruction families.
        range(0x100, 0x103, M);
        table[0x10d] = M;
        table[0x10f] = M | B; // 3DNow's trailing opcode byte
        range(0x110, 0x123, M);
        range(0x128, 0x12f, M);
        table[0x138] = M | encoding_extra_byte;
        table[0x13a] = M | encoding_extra_byte | B;
        range(0x140, 0x176, M);
        range(0x170, 0x173, M | B);
        table[0x178] = M | H; // SSE4a 66/F2 forms have two immediate bytes
        table[0x179] = M;
        range(0x17c, 0x17f, M);
        range(0x180, 0x18f, Z);
        range(0x190, 0x19f, M);
        set({0x1a3, 0x1a5, 0x1a6, 0x1a7, 0x1ab, 0x1ad, 0x1ae, 0x1af}, M);
        set({0x1a4, 0x1ac}, M | B);
        range(0x1b0, 0x1bf, M);
        table[0x1ba] = M | B;
        set({0x1c0, 0x1c1, 0x1c3, 0x1c7}, M);
        set({0x1c2, 0x1c4, 0x1c5, 0x1c6}, M | B);
        range(0x1d0, 0x1ff, M);
        return table;
    }();

    int64_t interpreter::parse(int64_t i) {
        parts = prefix = seg_prefix = sse_prefix = 0;
    refetch:
        code = fetch<uint8_t>(i);
        VLOG(4) << "opcode = " << std::hex << (int)code;

        switch (code) {
        case 0x0f: // 0f xx
            code = 0x100 | fetch<uint8_t>(i);
            break;
        case 0x26:
        case 0x2e: // es, cs
        case 0x36:
        case 0x3e: // ss, ds
        case 0x64:
        case 0x65: // fs, gs
            prefix &= prefix_legacy_mask;
            seg_prefix = code;
            goto refetch;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
        case 0x4c:
        case 0x4d:
        case 0x4e:
        case 0x4f: // rex
            prefix |= code & prefix_rex_mask;
            goto refetch;
        case 0x66: // os
            prefix &= 0xf0;
            if (!sse_prefix)
                sse_prefix = 1;
            prefix |= prefix_66h_mask;
            goto refetch;
        case 0x67: // as
            prefix &= prefix_legacy_mask;
            prefix |= prefix_67h_mask;
            goto refetch;
        case 0xf0: // lock
            prefix &= prefix_legacy_mask;
            prefix |= prefix_lock_mask;
            goto refetch;
        case 0xf2: // repne
        case 0xf3: // rep
            sse_prefix = code & 3;
            prefix &= prefix_legacy_mask;
            prefix |= prefix_repxx_mask;
            goto refetch;
        default:
            break;
        }
        uint8_t encoding_flags = encoding_lut[code];
        parts |= encoding_flags & (encoding_has_modrm | encoding_extra_byte);
        if (has_modrm()) {
            // read extra byte if needed
            if (unlikely(has_extra())) {
                extra = fetch<uint8_t>(i);
                VLOG(4) << "extra = " << std::hex << (int)extra;
            }
            // parse mod r/m
            uint8_t modrm = fetch<uint8_t>(i);
            mod = modrm >> 6;
            reg = rex_r((modrm >> 3) & 7);
            uint8_t base_or_rm = rm = rex_b(modrm & 7);

            VLOG(4) << "modrm = " << std::hex << (int)modrm << ", mod = " << (int)mod
                    << ", reg.r = " << (int)reg << ", rm.b = " << (int)rm;
            if ((mod != 3) && ((rm & 7) == 4)) {
                // read sib
                uint8_t sib = fetch<uint8_t>(i);
                VLOG(4) << "sib = " << std::hex << (int)sib;
                parts |= part_sib;
                // parse sib
                log_scale = sib >> 6;
                index = rex_x((sib >> 3) & 7);
                base_or_rm = base = rex_b(sib & 7);
                VLOG(4) << std::hex << "log_scale = " << (int)log_scale
                        << ", index.x = " << (int)index << ", base.b = " << (int)base;
            }
            // read displacement
            switch (mod) {
            case 0:
                if ((base_or_rm & 7) == 5) {
                    disp = fetch<int32_t>(i);
                    parts |= part_disp;
                } else
                    disp = 0;
                break;
            case 1:
                disp = fetch<int8_t>(i);
                parts |= part_disp;
                break;
            case 2:
                disp = fetch<int32_t>(i);
                parts |= part_disp;
                break;
            case 3:
                disp = 0;
                break;
            }
            if ((parts & part_disp) != 0)
                VLOG(4) << "disp = " << std::hex << (int)disp;
        }

        if (has_rex_w())
            log_v = 3;
        else if (has_os_prefix())
            log_v = 1;
        else if ((encoding_flags & encoding_default_os_64) != 0) {
            if (unlikely(code == 0xff))
                switch (reg) {
                case 2:
                case 4:
                case 6:
                    log_v = 3;
                    break; // CALL Ev, JMP Ev, PUSH Ev
                default:
                    log_v = 2;
                }
            else if (unlikely((code == 0x8f) && (reg != 0)))
                log_v = 2; // any 8f /n (n != 0) than POP Ev
            else
                log_v = 3;
        } else
            log_v = 2;

        VLOG(4) << "immediate form = " << (int)(encoding_flags & encoding_immediate_mask);
        switch (encoding_flags & encoding_immediate_mask) {
        case encoding_no_imm:
            imm = 0;
            break;
        case encoding_Ib:
            imm = fetch<int8_t>(i);
            parts |= part_imm;
            break;
        case encoding_Iz:
            imm = (log_v == 1) ? fetch<int16_t>(i) : fetch<int32_t>(i);
            parts |= part_imm;
            break;
        case encoding_Iw:
            imm = fetch<int16_t>(i);
            parts |= part_imm;
            break;
        case encoding_IwIb:
            imm = fetch<int16_t>(i);
            imm2 = fetch<int8_t>(i);
            parts |= (part_imm | part_imm2);
            break;
        case encoding_Iv:
            parts |= part_imm;
            switch (log_v) {
            case 1:
                imm = fetch<int16_t>(i);
                break;
            case 2:
                imm = fetch<int32_t>(i);
                break;
            case 3:
                imm = fetch<int64_t>(i);
                break;
            }
            break;
        case encoding_hard:
            switch (code) {
            case 0xa0:
            case 0xa1:
            case 0xa2:
            case 0xa3:
                imm = address_size_is_64() ? fetch<int64_t>(i) : fetch<uint32_t>(i);
                parts |= part_imm;
                break;
            case 0x178:
                imm = 0;
                if (sse_prefix == 1 || sse_prefix == 2) {
                    imm = fetch<uint8_t>(i);
                    imm2 = fetch<uint8_t>(i);
                    parts |= part_imm | part_imm2;
                }
                break;
            case 0xf6:
                if ((reg & 6) == 0) {
                    imm = fetch<int8_t>(i);
                    parts |= part_imm;
                } else
                    imm = 0;
                break;
            case 0xf7:
                if ((reg & 6) == 0) {
                    imm = (log_v == 1) ? fetch<int16_t>(i) : fetch<int32_t>(i); // Iz
                    parts |= part_imm;
                } else
                    imm = 0;
                break;
            default:
                LOG(DFATAL) << "hard immediate without case. opcode: " << std::hex << (int)code;
                code = 0x10b; // bail by invoking known bad opcode UD1
                imm = 0;
                break;
            }
            break;
        default:
            LOG(DFATAL) << "unknown immediate encoding "
                        << (int)(encoding_flags & encoding_immediate_mask);
            code = 0x10b;
            imm = 0;
            break;
        }
        if ((parts & part_imm) != 0)
            VLOG(4) << "imm = " << std::hex << (int)imm;
        return i;
    }
} // namespace jitpp
