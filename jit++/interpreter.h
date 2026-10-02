#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

#include <jit++/common.h>
#include <jit++/interpreting/traits.h>
#include <cstddef>
#include <type_traits>

namespace jitpp {
    using namespace interpreting;
    using namespace interpreting::flags;

    union mmx_t {
        long double ld;
        float f[2];
        double df[1];
        int64_t q[1];
        int32_t d[2];
        int16_t w[4];
        int8_t b[8], reserved[8];
    } __attribute__((aligned(8)));

    union xmm_t {
        double df[2];
        float f[4];
        int64_t q[2];
        int32_t d[4];
        int16_t w[8];
        int8_t b[16];
    } __attribute__((aligned(16)));

    // One concrete owner of the saved machine state, decoder, flags, and opcodes.
    class alignas(16) interpreter final {
      public:
        // Fixed prefix consumed by tracer_start.S; no vtable.
        void stub();
        void run();

        int64_t m_reserved[2];    /* 0: retain the saved-register offsets */
        int64_t m_reg[16];        /* 16 */
        int64_t m_rip;            /* 144 */
        mutable int64_t m_rflags; /* 152 */
        // fxsave format start
        int16_t m_fx_fcw; /* 160 */
        int16_t m_fx_fsw;
        int16_t m_fx_ftw;
        int16_t m_fx_fop;
        int32_t m_fx_ip;
        int16_t m_fx_cs;
        int16_t m_fx_reserved_1;
        int32_t m_fx_dp;
        int16_t m_fx_ds;
        int16_t m_fx_reserved_2;
        int32_t m_fx_mxcsr;
        int32_t m_fx_reserved_3;
        mmx_t m_fx_mmx[8];
        xmm_t m_fx_xmm[16];
        int64_t m_fx_reserved_4[12];
        // end fxsave format
        uint8_t *m_stack;    /* 672 */
        size_t m_stack_size; /* 680 */
        /* end of fixed structure */
        bool m_stopped;

        interpreter(size_t stack_size = default_stack_size());
        ~interpreter();

        static size_t default_stack_size();

        /* single threaded, non re-entrant! */
        JITPP_NOTHROW void start();
        void stop() { m_stopped = true; }

        // hide copy and assignment
        interpreter(const interpreter &) = delete;
        interpreter &operator=(const interpreter &) = delete;

        // Lazy arithmetic flags.

        const handler *m_handler;
        context m_context;

        // m_rflags is mutable for lazy flag materialization.
        int64_t &base_rflags() const { return m_rflags; }

        inline bool is_lazy(int32_t mask) const { return (m_lazy_flags & mask) != 0; }
        inline bool has_flag(int32_t mask) const { return (base_rflags() & mask) != 0; }

        mutable int64_t m_lazy_flags;

        inline int64_t lazy_flags() const { return m_lazy_flags; }
        inline bool cf() const {
            return is_lazy(CF) ? force_lazy(CF, m_handler->cf(m_context)) : has_flag(CF);
        }
        inline bool pf() const {
            return is_lazy(PF) ? force_lazy(PF, calculate_parity(m_context.result)) : has_flag(PF);
        }
        inline bool af() const {
            return is_lazy(AF) ? force_lazy(AF, m_handler->af(m_context)) : has_flag(AF);
        }
        inline bool zf() const {
            return is_lazy(ZF) ? force_lazy(ZF, m_context.result == 0) : has_flag(ZF);
        }
        inline bool sf() const {
            return is_lazy(SF) ? force_lazy(SF, m_context.result < 0) : has_flag(ZF);
        }
        inline bool of() const {
            return is_lazy(OF) ? force_lazy(OF, m_handler->of(m_context)) : has_flag(OF);
        }
        inline bool tf() const { return has_flag(TF); }
        inline bool df() const { return has_flag(DF); }

        inline bool set_flag(int32_t mask, bool value = true) {
            m_lazy_flags &= ~mask;
            if (value)
                base_rflags() |= mask;
            else
                base_rflags() &= ~mask;
            return value;
        }

        inline bool set_flags(int32_t mask, int32_t value) {
            m_lazy_flags &= ~mask;
            base_rflags() = (base_rflags() & ~mask) | (value & mask);
        }

        inline bool cf(bool value) { return set_flag(CF, value); }
        inline bool pf(bool value) { return set_flag(PF, value); }
        inline bool af(bool value) { return set_flag(AF, value); }
        inline bool zf(bool value) { return set_flag(ZF, value); }
        inline bool sf(bool value) { return set_flag(SF, value); }
        inline bool of(bool value) { return set_flag(OF, value); }
        inline bool tf(bool value) {
            if (value)
                base_rflags() |= TF;
            else
                base_rflags() &= ~TF;
            return value;
        }
        inline bool df(bool value) {
            if (value)
                base_rflags() |= DF;
            else
                base_rflags() &= ~DF;
            return value;
        }

        inline bool force_lazy(int64_t mask, bool value = true) const {
            VLOG(1) << "lazy flag " << mask << " = " << (value ? "true" : "false");
            m_lazy_flags &= ~mask;
            if (value)
                base_rflags() |= mask;
            else
                base_rflags() &= ~mask;
            return value;
        }

        // set carry, parity, aux, zero, sign and overflow all at once, note, new_flag_handler must
        // not go out of scope before rflags() is requested!
        int64_t handle_rflags(const handler &h, int64_t result, int64_t op1 = 0, int64_t op2 = 0) {
            base_rflags() &= ~h.cleared_flags();
            m_lazy_flags &= ~(h.handled_flags() | h.cleared_flags()) & OSZAPC;
            // force any flags that don't overlap with the new flag handler to evaluate
            rflags();

            // install the new flag handler
            m_lazy_flags = h.handled_flags();
            m_handler = &h;
            m_context.op1 = op1;
            m_context.op2 = op2;
            m_context.result = result;
            return result;
        }
        inline int64_t &rflags() {
            if (m_lazy_flags != 0) {
                // force flags
                cf();
                pf();
                af();
                zf();
                sf();
                of();
            }
            return base_rflags();
        }

        inline void rflags(int64_t value) {
            m_lazy_flags = 0;
            base_rflags() = value;
        }
        inline bool test_cc(uint8_t cc) {
            bool result = jitpp::interpreting::flags::test_cc(*this, cc);
            VLOG(1) << "condition? " << (result ? "true" : "false");
            return result;
        }

        // Decoder state and prefix helpers.

        int64_t imm;        // any immediate value byte, word, dword
        int32_t disp;       // displacement
        uint16_t parts;     // part_* indicates which optional parts are present
        uint16_t code;      // xx of 1xx if opcode is a 1 byte or has 0f xx form
        uint8_t sse_prefix; // 0 = none, 1 = 66 only, 2 = f2, 3 = f3
        uint8_t prefix;     // rex nybble, flags for other prefixes
        uint8_t extra;      // 0f 3a and 0f 38 have an extra byte
        uint8_t mod;
        uint8_t reg;       // extended with rex_r
        uint8_t rm;        // extended with rex_b
        uint8_t log_scale; // 1,2,4 or 8
        uint8_t index;     // extended with rex_x
        uint8_t base;      // extended with rex_b
        union {
            uint8_t drex; // drex byte value if present
            uint8_t imm2; // stack slot count for enter
        };
        uint8_t seg_prefix;
        uint8_t log_v; // default operand size = 2^log_v bytes

        // disassemble the opcode starting at rip
        // returns address of subsequent instruction on success
        int64_t parse(int64_t rip);

        inline bool has_seg_prefix() const { return seg_prefix != 0; }
        inline bool has_os_prefix() const { return (prefix & prefix_66h_mask) != 0; }
        inline bool has_as_prefix() const { return (prefix & prefix_67h_mask) != 0; }
        inline bool has_lock_prefix() const { return (prefix & prefix_lock_mask) != 0; }
        inline bool has_repne_prefix() const { return sse_prefix == 2; }
        inline bool has_rep_prefix() const { return sse_prefix == 3; }
        inline bool has_repxx_prefix() const { return (prefix & prefix_repxx_mask) != 0; }
        inline bool has_rex() const { return (prefix & prefix_rex_mask) != 0; }
        inline bool has_rex_w() const { return (prefix & prefix_rex_w_mask) != 0; }
        inline bool has_rex_r() const { return (prefix & prefix_rex_r_mask) != 0; }
        inline bool has_rex_x() const { return (prefix & prefix_rex_x_mask) != 0; }
        inline bool has_rex_b() const { return (prefix & prefix_rex_b_mask) != 0; }
        inline bool has_extra() const { return (parts & part_extra) != 0; }
        inline bool has_modrm() const { return (parts & part_modrm) != 0; }
        inline bool has_sib() const { return (parts & part_sib) != 0; }
        inline bool has_drex() const { return (parts & part_drex) != 0; }
        inline bool has_disp() const { return (parts & part_disp) != 0; }
        inline bool has_imm() const { return (parts & part_imm) != 0; }
        inline bool has_imm2() const { return (parts & part_imm2) != 0; }
        inline bool is_rip_relative() const { return !has_sib() && (mod == 0) && ((rm & 7) == 5); }
        inline bool has_memory_operand() const { return mod != 3; } // assumes has_modrm
        inline bool address_size_is_64() const { return (prefix & prefix_67h_mask) == 0; }

        inline uint8_t v() const { return 1 << log_v; }
        inline uint8_t scale() const { return 1 << log_scale; }

        // augment with rex, input is a 3 bit wide value
        inline uint8_t rex_w(uint8_t b) const { return (prefix & prefix_rex_w_mask) | b; }
        inline uint8_t rex_r(uint8_t b) const { return ((prefix & prefix_rex_r_mask) << 1) | b; }
        inline uint8_t rex_x(uint8_t b) const { return ((prefix & prefix_rex_x_mask) << 2) | b; }
        inline uint8_t rex_b(uint8_t b) const { return ((prefix & prefix_rex_b_mask) << 3) | b; }

        // flags and lookups
        static const uint8_t prefix_legacy_mask = 0xf0;
        static const uint8_t prefix_repxx_mask = 0x80;
        static const uint8_t prefix_lock_mask = 0x40;
        static const uint8_t prefix_67h_mask = 0x20;
        static const uint8_t prefix_66h_mask = 0x10;

        static const uint8_t prefix_rex_mask = 0x0f;
        static const uint8_t prefix_rex_w_mask = 0x08;
        static const uint8_t prefix_rex_r_mask = 0x04;
        static const uint8_t prefix_rex_x_mask = 0x02;
        static const uint8_t prefix_rex_b_mask = 0x01;

        // extra, modrm, sib, drex, disp, imm, imm2
        static const uint8_t part_modrm = 0x80; // == encoding_modrm_byte
        static const uint8_t part_extra = 0x40; // == encoding_extra_byte
        static const uint8_t part_drex = 0x20;  // == encoding_drex_byte
        static const uint8_t part_sib = 0x10;   // scale index & base are valid
        static const uint8_t part_disp = 0x08;
        static const uint8_t part_imm = 0x04;
        static const uint8_t part_imm2 = 0x02;

        static const uint8_t encoding_has_modrm = 0x80;
        static const uint8_t encoding_extra_byte = 0x40;    // [0f 38] or [0f 3a]
        static const uint8_t encoding_drex_byte = 0x20;     // [0f 24] of [0f 25]
        static const uint8_t encoding_default_os_64 = 0x10; // 50-5f, ff/6 8f/0

        static const uint8_t encoding_immediate_mask = 0x07;
        static const uint8_t encoding_no_imm = 0x00;
        static const uint8_t encoding_Ib = 0x01;
        static const uint8_t encoding_Iz = 0x02;
        static const uint8_t encoding_Iw = 0x03;
        static const uint8_t encoding_IwIb = 0x04; // ENTER
        static const uint8_t encoding_Iv = 0x05;   // z,w,q as appropriate to size
        static const uint8_t encoding_hard = 0x06; // handle opcode by opcode hard code

        static const uint8_t encoding_lut[512];

        // Registers, memory operands, and diagnostics.

        // we defer the syscall for these until they are used.
        mutable int64_t m_fs_base;
        mutable int64_t m_gs_base;

        mutable bool m_fs_base_known;
        mutable bool m_gs_base_known;

        int64_t fs_base() const;
        int64_t gs_base() const;
        int64_t seg_base() const;

        void print_regs();
        void print_opcode(int64_t rip, int expected_size = 0);
        void print_address(int64_t addr);

        inline int32_t eip() const { return static_cast<int32_t>(rip()); }
        inline int64_t rip() const { return m_rip; }
        inline int64_t &rip() { return m_rip; }

        int64_t repetitions() const;
        int64_t mem(bool add_segment_base = true) const;

        inline int8_t &ah();
        inline int8_t ah() const;

        inline int64_t &rax() { return m_reg[0]; }
        inline int64_t &rcx() { return m_reg[1]; }
        inline int64_t &rdx() { return m_reg[2]; }
        inline int64_t &rbx() { return m_reg[3]; }
        inline int64_t &rsp() { return m_reg[4]; }
        inline int64_t &rbp() { return m_reg[5]; }
        inline int64_t &rsi() { return m_reg[6]; }
        inline int64_t &rdi() { return m_reg[7]; }
        inline int64_t &r8() { return m_reg[8]; }
        inline int64_t &r9() { return m_reg[9]; }
        inline int64_t &r10() { return m_reg[10]; }
        inline int64_t &r11() { return m_reg[11]; }
        inline int64_t &r12() { return m_reg[12]; }
        inline int64_t &r13() { return m_reg[13]; }
        inline int64_t &r14() { return m_reg[14]; }
        inline int64_t &r15() { return m_reg[15]; }

        inline int64_t rax() const { return m_reg[0]; }
        inline int64_t rcx() const { return m_reg[1]; }
        inline int64_t rdx() const { return m_reg[2]; }
        inline int64_t rbx() const { return m_reg[3]; }
        inline int64_t rsp() const { return m_reg[4]; }
        inline int64_t rbp() const { return m_reg[5]; }
        inline int64_t rsi() const { return m_reg[6]; }
        inline int64_t rdi() const { return m_reg[7]; }
        inline int64_t r8() const { return m_reg[8]; }
        inline int64_t r9() const { return m_reg[9]; }
        inline int64_t r10() const { return m_reg[10]; }
        inline int64_t r11() const { return m_reg[11]; }
        inline int64_t r12() const { return m_reg[12]; }
        inline int64_t r13() const { return m_reg[13]; }
        inline int64_t r14() const { return m_reg[14]; }
        inline int64_t r15() const { return m_reg[15]; }

        inline int32_t eax() const { return m_reg[0]; }
        inline int32_t ecx() const { return m_reg[1]; }
        inline int32_t edx() const { return m_reg[2]; }
        inline int32_t ebx() const { return m_reg[3]; }
        inline int32_t esp() const { return m_reg[4]; }
        inline int32_t ebp() const { return m_reg[5]; }
        inline int32_t esi() const { return m_reg[6]; }
        inline int32_t edi() const { return m_reg[7]; }

        inline int16_t ax() const { return m_reg[0]; }
        inline int16_t cx() const { return m_reg[1]; }
        inline int16_t dx() const { return m_reg[2]; }
        inline int16_t bx() const { return m_reg[3]; }
        inline int16_t sp() const { return m_reg[4]; }
        inline int16_t bp() const { return m_reg[5]; }
        inline int16_t si() const { return m_reg[6]; }
        inline int16_t di() const { return m_reg[7]; }

        inline int8_t al() const { return m_reg[0]; }
        inline int8_t cl() const { return m_reg[1]; }
        inline int8_t dl() const { return m_reg[2]; }
        inline int8_t bl() const { return m_reg[3]; }

        template <typename T> inline const char *reg_name(int r) const;
        template <typename T> inline T get_reg(int r) const;
        template <typename T> inline void set_reg(int r, T v);
        template <typename T> inline T M() const;
        template <typename T> inline void M(T v);
        template <typename T> inline T G() const;
        template <typename T> inline void G(T v);
        template <typename T> inline T R() const;
        template <typename T> inline void R(T v);
        template <typename T> inline T E() const;
        template <typename T> inline void E(T v);
        template <typename T> void push(T v);
        template <typename T> T pop();

        // Instruction group 1.

        inline int64_t add(int64_t x, int64_t y) { return handle_rflags(add_flags, x + y, x, y); }
        inline int64_t sub(int64_t x, int64_t y) { return handle_rflags(sub_flags, x - y, x, y); }
        inline int64_t and_(int64_t x, int64_t y) {
            VLOG(1) << std::hex << x << " & " << y << " = " << (x & y);
            return handle_rflags(logic_flags, x & y);
        }
        inline int64_t or_(int64_t x, int64_t y) { return handle_rflags(logic_flags, x | y); }
        inline int64_t xor_(int64_t x, int64_t y) { return handle_rflags(logic_flags, x ^ y); }
        inline int64_t adc(int64_t x, int64_t y) {
            if (cf())
                return handle_rflags(adc_flags, x + y + 1, x, y);
            else
                return handle_rflags(add_flags, x + y, x, y);
        }
        template <typename T> inline int64_t sbb(int64_t x, int64_t y) {
            if (cf())
                return handle_rflags(group_1_traits<T>::sbb_flags, x - y - 1, x, y);
            else
                return handle_rflags(sub_flags, x - y, x, y);
        }

        template <typename T> inline void interpret_group_1(int64_t imm) {
            switch (reg) {
            case 0:
                E<T>(add(E<T>(), imm));
                return; // ADD E?, I?
            case 1:
                E<T>(or_(E<T>(), imm));
                return; // OR  E?, I?
            case 2:
                E<T>(adc(E<T>(), imm));
                return; // ADC E?, I?
            case 3:
                E<T>(sbb<T>(E<T>(), imm));
                return; // SBB E?, I?
            case 4:
                E<T>(and_(E<T>(), imm));
                return; // AND E?, I?
            case 5:
                E<T>(sub(E<T>(), imm));
                return; // SUB E?, I?
            case 6:
                E<T>(xor_(E<T>(), imm));
                return; // XOR E?, I?
            case 7:
                sub(E<T>(), imm);
                return; // CMP E?, I?
            default:
                logic_error();
            }
        }

        static const flags::handler add_flags, adc_flags, sub_flags, logic_flags;

        // Instruction group 2.

        template <typename T> inline int64_t sal(int64_t x, int count) {
            return handle_rflags(group_2_traits<T>::sal_flags, x << count, x, count);
        }
        template <typename T> inline int64_t sar(int64_t x, int count) {
            return handle_rflags(group_2_traits<T>::sar_flags, x >> count, x, count);
        }
        template <typename T> inline uint64_t shr(uint64_t x, int count) {
            return handle_rflags(group_2_traits<T>::shr_flags, x >> count, x, count);
        }
        template <typename T> inline uint64_t rol(uint64_t x, int count) {
            return handle_rflags(group_2_traits<T>::rol_flags,
                                 (x << count) | (x >> (group_2_traits<T>::size - count)), x, count);
        }
        template <typename T> inline uint64_t ror(uint64_t x, int count) {
            return handle_rflags(group_2_traits<T>::ror_flags,
                                 (x << (group_2_traits<T>::size - count)) | (x >> count), x, count);
        }
        template <typename T> inline uint64_t rcl(uint64_t x, int count) {
            uint64_t carry_bit = cf() ? 1 : 0;
            uint64_t result = (x << count) | carry_bit << (count - 1);
            if (count != 1)
                result |= (x >> (group_2_traits<T>::size + 1 - count));
            bool carry_result = (x >> (group_2_traits<T>::size - count)) & 1;
            cf(carry_result);
            of(carry_result ^ (result < 0));
            return result;
        }
        template <typename T> inline uint64_t rcr(uint64_t x, int count) {
            uint64_t carry_bit = cf() ? 1 : 0;
            uint64_t result = (x >> count) | (carry_bit << (group_2_traits<T>::size - count));
            if (count != 1)
                result |= (x << (group_2_traits<T>::size + 1 - count));
            bool carry_result = (x >> (count - 1)) & 1;
            cf(carry_result);
            of(((result << 1) & result) < 0);
            return result;
        }

        template <typename T> inline uint64_t shrd(uint64_t x, uint64_t y, int count) {
            return handle_rflags(group_2_traits<T>::shrd_flags,
                                 (y << (group_2_traits<T>::size - count)) | (x >> count), x, count);
        }

        template <typename T> void interpret_group_2(int count) {
            count &= group_2_traits<T>::mask;
            if (!count)
                return;
            switch (reg) {
            case 0:
                E<T>(rol<T>(E<T>(), count));
                return; // ROL E?,??
            case 1:
                E<T>(ror<T>(E<T>(), count));
                return; // ROR E?,??
            case 2:
                E<T>(rcl<T>(E<T>(), count));
                return; // RCL E?,??
            case 3:
                E<T>(rcr<T>(E<T>(), count));
                return; // RCR E?,??
            case 4:
                E<T>(sal<T>(E<T>(), count));
                return; // SHL E?,??
            case 5:
                E<T>(shr<T>(E<T>(), count));
                return; // SHR E?,??
            case 6:
                E<T>(sal<T>(E<T>(), count));
                return; // SAL E?,?? = SHL E?,??
            case 7:
                E<T>(sar<T>(E<T>(), count));
                return; // SAR E?,??
            }
            logic_error();
        }

        // Instruction group 3.

        template <typename T> inline int64_t neg(int64_t x) {
            return handle_rflags(group_3_traits<T>::neg_flags, -x);
        }
        template <typename T> inline void mul();
        template <typename T> inline void imul();
        template <typename T> inline void div();
        template <typename T> inline void idiv();

        template <typename T> inline void interpret_group_3() {
            switch (reg) {
            case 0 ... 1:
                and_(E<T>(), imm);
                return; // TEST E?, I?*
            case 2:
                E<T>(~E<T>());
                return; // NOT E?
            case 3:
                E<T>(neg<T>(E<T>()));
                return; // NEG E?
            case 4:
                mul<T>();
                return; // MUL E?
            case 5:
                imul<T>();
                return; // IMUL E?
            case 6:
                div<T>();
                return; // DIV E?
            case 7:
                idiv<T>();
                return; // DIV E?
            default:
                unsupported();
            }
        }

        // Instruction group 4.

        template <typename T> inline int64_t inc(int64_t x) {
            return handle_rflags(group_4_traits<T>::inc_flags, x + 1);
        }
        template <typename T> inline int64_t dec(int64_t x) {
            return handle_rflags(group_4_traits<T>::dec_flags, x - 1);
        }
        template <typename T> inline void interpret_group_4() {
            switch (reg) {
            case 0:
                E<T>(inc<T>(E<T>()));
                return;
            case 1:
                E<T>(dec<T>(E<T>()));
                return;
            default:
                illegal();
            }
        }

        // Instruction group 5.

        template <typename T> inline void interpret_group_5() {
            switch (reg) {
            case 0:
                E<T>(inc<T>(E<T>()));
                return; // INC Ev
            case 1:
                E<T>(dec<T>(E<T>()));
                return; // DEC Ev
            case 2:
                push<T>(rip());
                rip() = E<T>();
                return; // CALL Ev
            case 3:
                unsupported();
            case 4:
                rip() = E<T>();
                return; // JMP Ev
            case 5:
                unsupported();
            case 6:
                push<T>(E<T>());
                return; // PUSH Ev
            case 7:
                illegal();
            default:
                unsupported();
            }
        } // interpret_group_5

        // Instruction group 6.

        inline uint16_t str() {
            uint16_t result;
            asm("str %0" : "=q"(result)::"memory");
            return result;
        }
        inline void verr(int16_t selector) {
            int8_t zero;
            asm("verr %1; setz %0" : "=q"(zero) : "q"(selector) : "memory");
            zf(zero != 0);
        }
        inline void verw(int16_t selector) {
            int8_t zero;
            asm("verw %1; setz %0" : "=q"(zero) : "q"(selector) : "memory");
            zf(zero != 0);
        }
        template <typename os> inline void interpret_group_6() {
            // memory operand
            switch (reg) {
            case 0:
                uninterpretable(); // SLDT Mw/Rv
            case 1:                // STR Mw/Rv
                if (mod == 3)
                    R<typename os::v>(str()); // zero extended
                else
                    M<int16_t>(str());
            case 2:
                uninterpretable(); // LLDT Mw/Rv
            case 3:
                uninterpretable(); // LTR Mw/Rv
            case 4:
                verr(E<int16_t>()); // VERR Mw/Rv
            case 5:
                verw(E<int16_t>()); // VERW Mw/Rv
            case 6:
                uninterpretable(); // JMPE Ev (Itanium only)
            case 7:
                illegal();
            default:
                logic_error();
            }
        } // interpret_group_6

        // Instruction group 7.

        // output TSC into EDX:EAX and TSC_AUX into ECX
        void rdtscp() {
            int64_t RAX, RCX, RDX;
            __asm__ __volatile__("rdtsc" : "=a"(RAX), "=d"(RDX), "=c"(RCX)::"memory");
            rax() = RAX;
            rcx() = RCX;
            rdx() = RDX;
        }
        // monitor a linear region of memory starting at RAX for changes
        inline void monitor() {
            int64_t RAX = rax();
            int64_t RCX = rcx();
            int64_t RDX = rdx();
            __asm__ __volatile__("monitor" ::"a"(RAX), "c"(RCX), "d"(RDX) : "memory");
        }
        // wait for the changes we are monitoring to occur
        inline void mwait() {
            int64_t RAX = rax();
            int64_t RCX = rcx();
            __asm__ __volatile__("mwait" ::"a"(RAX), "c"(RCX) : "memory");
        }
        uint64_t smsw() {
            uint64_t result;
            __asm__("smsw %0" : "=q"(result)::"memory");
            return result;
        }
        template <typename os> inline void interpret_group_7() {
            if (mod != 3) {
                // memory operand
                switch (reg) {
                case 0:
                    uninterpretable(); // SGDT Ms
                case 1:
                    uninterpretable(); // SIDT Ms
                case 2:
                    uninterpretable(); // LDGT Ms
                case 3:
                    uninterpretable(); // LIDT Ms
                case 4:
                    M<uint16_t>(smsw()); // SMSW Mw -- this we CAN execute!
                case 5:
                    illegal();
                case 6:
                    uninterpretable(); // LMSW Mw
                case 7:
                    uninterpretable(); // INVLPG Mb
                default:
                    logic_error();
                }
            } else {
                switch (reg) {
                case 0:
                    switch (rm) {
                    case 1 ... 4:
                        uninterpretable(); // VMCALL, VMLAUNCH, VMRESUME, VMXOFF
                    default:
                        illegal();
                    }
                case 1:
                    switch (rm) {
                    case 0:
                        monitor();
                        return; // MONITOR
                    case 1:
                        mwait();
                        return; // MWAIT
                    default:
                        illegal();
                    }
                case 2:
                    switch (rm) {
                    case 0 ... 1:
                        unsupported(); // XGETBV, XSETBV
                    default:
                        illegal();
                    }
                case 3:
                    uninterpretable(); // VMRUN, VMMCALL, VMLOAD, VMSAVE, STGI, CLGI, SKINIT,
                                       // INVLPGA
                case 4:
                    R<typename os::v>(smsw());
                    return; // SMSW Rv
                case 5:
                    illegal();
                case 6:
                    uninterpretable(); // LMSW Rw
                case 7:
                    switch (rm) {
                    case 0:
                        uninterpretable(); // SWAPGS
                    case 1:
                        rdtscp();
                        return;
                    default:
                        illegal();
                    }
                }
            }
        } // interpret_group_7

        // Miscellaneous instructions.

        template <typename T> inline void cmpxchg() {
            int8_t value = E<T>();
            if (zf(value == get_reg<T>(0)))
                E<T>(G<T>());
            else
                set_reg<T>(0, value);
        }
        template <typename T> inline void xchg() {
            T t = G<T>();
            G<T>(E<T>());
            E<T>(t);
        }
        template <typename T> inline void xchg(int r) {
            T t = get_reg<T>(r);
            set_reg<T>(r, get_reg<T>(0));
            set_reg<T>(0, t);
        }

        template <typename T> inline void movs() {
            int64_t base = seg_base();
            T *s = reinterpret_cast<T *>(base + rsi());
            T *d = reinterpret_cast<T *>(rdi());
            int count = repetitions();
            VLOG(1) << "movs:"
                    << " copying " << count << " " << sizeof(T) << " byte chunks from " << std::hex
                    << (base + rsi()) << " to " << rdi() << (df() ? " descending" : "");
            if (df())
                while (count-- != 0)
                    *s-- = *d--;
            else
                while (count-- != 0)
                    *s++ = *d++;
            // TODO: initialize any flags here ?
            rsi() = reinterpret_cast<int64_t>(s) - base;
            rdi() = reinterpret_cast<int64_t>(d);
        }

        void syscall_();
        void wbinvd();
        void invd();
        void wrmsr();
        void rdmsr();
        void rdtsc();
        void rdpmc();
        int64_t lsl(int16_t descriptor);
        int64_t lar(int16_t descriptor);

        // Opcode dispatch.
        template <typename os> void interpret_opcode();

        // Locked instructions.

        template <typename T> inline void lock_inc(T *ptr);
        template <typename T> inline void lock_dec(T *ptr);
        template <typename T> inline T lock_cmpxchg(T *p, T o, T n);

        template <typename os> void interpret_locked_opcode();
    };

    static_assert(std::is_standard_layout<interpreter>::value,
                  "assembly requires standard-layout state");
    static_assert(offsetof(interpreter, m_reg) == 0x10, "assembly offset: m_reg");
    static_assert(offsetof(interpreter, m_rip) == 0x90, "assembly offset: m_rip");
    static_assert(offsetof(interpreter, m_rflags) == 0x98, "assembly offset: m_rflags");
    static_assert(offsetof(interpreter, m_fx_fcw) == 0xa0, "assembly offset: m_fx_fcw");
    static_assert(offsetof(interpreter, m_stack) == 0x2a0, "assembly offset: m_stack");
    static_assert(offsetof(interpreter, m_stack_size) == 0x2a8, "assembly offset: m_stack_size");

} // namespace jitpp

#include <jit++/interpreting/base.h>
#include <jit++/interpreting/group_3.h>
#include <jit++/interpreting/locked.h>
#include <jit++/interpreting/opcode.h>

#endif // INCLUDED_JITPP_INTERPRETER_H
