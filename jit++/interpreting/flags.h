#ifndef INCLUDED_JITPP_INTERPRETING_FLAGS_H
#define INCLUDED_JITPP_INTERPRETING_FLAGS_H

#include <jit++/common.h> // for debug purposes only

namespace jitpp {
  namespace interpreting {
    namespace flags {

        static const int32_t CF = 0x0001;
        static const int32_t PF = 0x0004;
        static const int32_t AF = 0x0010;
        static const int32_t ZF = 0x0040;
        static const int32_t SF = 0x0080;
        static const int32_t TF = 0x0100;
        static const int64_t IF = 0x0200;
        static const int32_t DF = 0x0400;
        static const int32_t OF = 0x0800;

        static const int32_t OSZAPC = OF | SF | ZF | AF | PF | CF;
        static const int32_t OSZAP = OF | SF | ZF | AF | PF;
        static const int64_t OSZPC = OF | SF | ZF | PF | CF;

        // plain old data
        struct context {
            int64_t op1, op2, result;
        };

        // plain old data
        struct handler {
            bool(*m_cf)(const context &);
            bool(*m_af)(const context &);
            bool(*m_of)(const context &);
            const int32_t m_handled_flags;
            const int32_t m_cleared_flags;

            inline bool cf(const context & ctx) const { return m_cf(ctx); }
            inline bool af(const context & ctx) const { return m_af(ctx); }
            inline bool of(const context & ctx) const { return m_of(ctx); }
            inline int32_t handled_flags() const { return m_handled_flags; }
        inline int32_t cleared_flags() const { return m_cleared_flags; }
        };

        extern const uint8_t parity_lut[256];
        inline bool calculate_parity(uint8_t b) { return parity_lut[b] != 0; }

        template <typename T> inline bool test_cc(T & i, uint8_t cc) {
            switch (cc) {
            case 0x0: return i.of();
            case 0x1: return !i.of();
            case 0x2: return i.cf();
            case 0x3: return !i.cf();
            case 0x4: return i.zf();
            case 0x5: return !i.zf();
            case 0x6: return i.cf() || i.zf();
            case 0x7: return !(i.cf() || i.zf());
            case 0x8: return i.sf();
            case 0x9: return !i.sf();
            case 0xa: return i.pf();
            case 0xb: return !i.pf();
            case 0xc: return i.sf() != i.of();
            case 0xd: return i.sf() == i.of();
            case 0xe: return i.zf() || (i.sf() != i.of());
            case 0xf: return !i.zf() && (i.sf() == i.of());
            }
        }

    bool bad_flag(const context &);
    } // namespace flags;
  } // namespace interpreting
} // namespace jitpp

#endif
