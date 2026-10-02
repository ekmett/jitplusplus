#ifndef INCLUDED_JITPP_INTERPRETING_TRAITS_H
#define INCLUDED_JITPP_INTERPRETING_TRAITS_H

#include <jit++/interpreting/flags.h>

namespace jitpp {
    namespace interpreting {
        struct os64 {
            typedef int64_t v;
            typedef int32_t z;
            typedef uint32_t uz;
            typedef int64_t qv;
            typedef int32_t smaller_size;
            static const int bits = 64;
            static const char *reg_name(int r);
        };

        struct os32 {
            typedef int32_t v;
            typedef int32_t z;
            typedef uint32_t uz;
            typedef int64_t qv;
            typedef int16_t smaller_size;
            static const int bits = 32;
            static const char *reg_name(int r);
        };

        struct os16 {
            typedef int16_t v;
            typedef int16_t z;
            typedef uint16_t uz;
            typedef int16_t qv;
            typedef int8_t smaller_size;
            static const int bits = 16;
            static const char *reg_name(int r);
        };

        template <typename T> uint64_t zero_extend(T v) { return v; }
        template <typename T> int64_t sign_extend(T v) { return v; }

        template <typename T> struct os;
        template <> struct os<int16_t> {
            typedef os16 value;
        };
        template <> struct os<int32_t> {
            typedef os32 value;
        };
        template <> struct os<int64_t> {
            typedef os64 value;
        };

        extern bool sbb_af(const flags::context &);
        extern bool sbb_of(const flags::context &);

        template <typename T> struct group_1_traits {
            static inline bool sbb_cf(const flags::context &ctx) {
                return ((uint64_t)ctx.op1 < (uint64_t)ctx.result ||
                        (static_cast<T>(ctx.op2) == static_cast<T>(0xffffffffffffffffULL)));
            }
            static const flags::handler sbb_flags;
        };
        template <typename T>
        const flags::handler group_1_traits<T>::sbb_flags = {sbb_cf, sbb_af, sbb_of, flags::OSZAPC,
                                                             0};

        using namespace jitpp::interpreting::flags;

        template <typename T> struct group_2_traits {
            static const int size = sizeof(T) * 8;
            static const int mask = size == 64 ? 0x3f : 0x1f;
            static inline bool sal_cf(const context &ctx) {
                return ((ctx.op1 >> (size - ctx.op2)) & 1) != 0;
            }
            static inline bool sal_of(const context &ctx) { return (ctx.op1 ^ ctx.result) < 0; }
            static inline bool sar_cf(const context &ctx) {
                return ((ctx.op1 >> (ctx.op2 - 1)) & 1) != 0;
            }
            static inline bool ror_cf(const context &ctx) { return (ctx.result < 0); }
            static inline bool shr_of(const context &ctx) {
                return ((ctx.result << 1) ^ ctx.result) < 0;
            }
            static const flags::handler sal_flags, rol_flags, shr_flags, ror_flags, sar_flags,
                shrd_flags;
        };

        template <typename T>
        const flags::handler group_2_traits<T>::sal_flags = {sal_cf, flags::bad_flag, sal_of, OSZPC,
                                                             AF};
        template <typename T>
        const flags::handler group_2_traits<T>::rol_flags = {sal_cf, flags::bad_flag, sal_of,
                                                             OF | CF, 0};
        template <typename T>
        const flags::handler group_2_traits<T>::sar_flags = {sar_cf, flags::bad_flag,
                                                             flags::bad_flag, CF, OF};
        template <typename T>
        const flags::handler group_2_traits<T>::shr_flags = {sar_cf, flags::bad_flag, shr_of, OSZPC,
                                                             AF};
        template <typename T>
        const flags::handler group_2_traits<T>::ror_flags = {ror_cf, flags::bad_flag, shr_of,
                                                             OF | CF, 0};
        template <typename T>
        const flags::handler group_2_traits<T>::shrd_flags = {sar_cf, flags::bad_flag, shr_of,
                                                              OSZPC, AF};

        using namespace jitpp::interpreting::flags;

        template <typename T> struct group_3_traits {
            static inline bool neg_cf(const context &ctx) { return ctx.result != 0; }
            static inline bool neg_af(const context &ctx) { return (ctx.result & 0xf) != 0; }
            static inline bool neg_of(const context &ctx) {
                return ctx.result == 1ULL << (sizeof(T) * 8 - 1);
            }
            static const flags::handler neg_flags;
        };
        template <typename T>
        const flags::handler group_3_traits<T>::neg_flags = {neg_cf, neg_af, neg_of, flags::OSZAPC,
                                                             0};

        using namespace jitpp::interpreting::flags;

        template <typename T> struct group_4_traits {
            static const int size = sizeof(T) * 8;
            static const int shift_mask = (size == 64) ? 0x3f : 0x1f;
            static inline bool dec_of(const context &ctx) {
                return ctx.result == (1ULL << (size - 1)) - 1;
            }
            static inline bool inc_of(const context &ctx) {
                return ctx.result == 1ULL << (size - 1);
            }
            static inline bool inc_af(const context &ctx) { return (ctx.result & 0xf) == 0; }
            static inline bool dec_af(const context &ctx) { return (ctx.result & 0xf) == 0xf; }
            static const handler inc_flags, dec_flags;
        };

        template <typename T>
        const handler group_4_traits<T>::inc_flags = {bad_flag, inc_af, inc_of, OSZAP, 0};
        template <typename T>
        const handler group_4_traits<T>::dec_flags = {bad_flag, dec_af, dec_of, OSZAP, 0};

        extern const char *byte_reg_name(int r, bool has_rex);
        extern __attribute__((noreturn)) void illegal();
        extern __attribute__((noreturn)) void unsupported();
        extern __attribute__((noreturn)) void uninterpretable();
        extern __attribute__((noreturn)) void logic_error();

    } // namespace interpreting
} // namespace jitpp

#endif // INCLUDED_JITPP_INTERPRETING_TRAITS_H
