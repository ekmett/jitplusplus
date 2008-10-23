#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

#include <jit++/opcode.h>
#include <jit++/tracer.h>
#include <jit++/rflags.h>

namespace jitpp { 

    // used internally
    namespace interpreter_result {
        enum code {       
            ok = 0, 		// all set
	    illegal, 		// not a valid opcode
	    wont_trace, 	// this opcode should never be traced for technical reasons
	    unsupported,	// not a supported opcode
	    logic_error,	// logic error in the interpreter
	    max
        };
	extern const char * description[max];
    }

    class interpreter : public lazy_rflags_strategy_mixin<tracer> { 
    public:
        interpreter() {}
        inline void operator ()() { start(); } 
    private:
        void run();

    private:
        // we defer the syscall for these until they are used.
        mutable int64_t m_fs_base;
        mutable int64_t m_gs_base;

        mutable bool m_fs_base_known;
        mutable bool m_gs_base_known;

	opcode op;
	int old_errno;
    protected:

	inline bool test_cc(uint8_t cc) const;

        int64_t fs_base() const;
        int64_t gs_base() const;

        inline int32_t eip() const { return static_cast<int32_t>(rip()); }
        inline int64_t rip() const { return m_rip; }
        inline int64_t & rip() { return m_rip; }

        inline int64_t rsp () const { return m_reg[4]; }
        inline int64_t & rsp() { return m_reg[4]; }

        inline int64_t * regs() { return m_reg; }
        inline const int64_t * regs() const { return m_reg; }

        inline mmx_t * mmx() { return m_fx_mmx; }
        inline const mmx_t * mmx() const { return m_fx_mmx; }

        inline xmm_t * xmm() { return m_fx_xmm; }
        inline const xmm_t * xmm() const { return m_fx_xmm; }

	int64_t memory_operand_address(bool add_segment_base = true) const;

        template <typename T> interpreter_result::code interpret_opcode();

	// fetch a given register. if T = int8_t then rex byte influences result.
        template <typename T> inline T reg(int r) const; 
        template <typename T> inline void reg(int r, T v);

	// M (mod R/M specifies a memory operand)
        template <typename T> inline T M() const;
        template <typename T> inline void M(T v);

        // G (reg field of mod R/M byte selects a general register)
        template <typename T> inline T G() const;
        template <typename T> inline void G(T v);

        // R (r/m field of mod R/M byte selects a general register)
        template <typename T> inline T R() const;
        template <typename T> inline void R(T v);

        // E (r/m follows opcode and specifies operand, either memory or register)
        template <typename T> inline T E() const;
        template <typename T> inline void E(T v);

        inline uint8_t & ah();
        inline uint8_t ah() const;

        template <typename T> void push(T v);
        template <typename T> T pop();
    };

    inline uint8_t & interpreter::ah() { return reinterpret_cast<uint8_t*>(m_reg)[1]; } 
    inline uint8_t interpreter::ah() const { return reinterpret_cast<const uint8_t*>(m_reg)[1]; } 

    // M (mod R/M specifies a memory operand)
    template <typename T> inline T interpreter::M() const { 
        int64_t addr = memory_operand_address();
        T result = *reinterpret_cast<T*>(addr);
        return result;
    }

    template <typename T> inline void interpreter::M(T v) { 
        *reinterpret_cast<T*>(memory_operand_address()) = v; 
    }

    // G (reg field of mod R/M byte selects a general register)
    template <typename T> inline T interpreter::G() const { 
        return reg<T>(op.reg); 
    } 
    template <typename T> inline void interpreter::G(T v) { 
        reg<T>(op.reg,v); 
    }

    // R (r/m field of mod R/M byte selects a general register)
    template <typename T> inline T interpreter::R() const { 
        return reg<T>(op.rm); 
    } 
    template <typename T> inline void interpreter::R(T v) { 
        reg<T>(op.rm,v); 
    }

    // E (r/m follows opcode and specifies operand, either memory or register)
    template <typename T> inline T interpreter::E() const { 
            return op.mod == 3 ? R<T>() : M<T>(); 
    }
    template <typename T> inline void interpreter::E(T v) { 
        if (op.mod == 3) R<T>(v); 
        else M<T>(v); 
    }

    template <typename T> void interpreter::push(T v) { 
        rsp() -= sizeof(T);
        *reinterpret_cast<T*>(rsp()) = v;
    }

    template <typename T> T interpreter::pop() { 
        T result = *reinterpret_cast<T*>(rsp());
        rsp() += sizeof(T);
        return result;
    }

    template <> inline int64_t interpreter::reg<>(int r) const { return m_reg[r]; }
    template <> inline int32_t interpreter::reg<>(int r) const { return *reinterpret_cast<const int32_t*>(m_reg + r); }
    template <> inline int16_t interpreter::reg<>(int r) const { return *reinterpret_cast<const int16_t*>(m_reg + r); }
    template <> inline int8_t interpreter::reg<>(int r) const { 
        if (op.has_rex()) 
            return *reinterpret_cast<const int8_t*>(m_reg + r);
	else
            return *(reinterpret_cast<const int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0));
    }

    template <> inline void interpreter::reg<>(int r, int64_t v) { m_reg[r] = v; }
    template <> inline void interpreter::reg<>(int r, int32_t v)  { m_reg[r] = v; } 
    template <> inline void interpreter::reg<>(int r, int16_t v) { *reinterpret_cast<int16_t*>(m_reg + r) = v; }
    template <> void interpreter::reg<>(int r, int8_t v) { 
        if (op.has_rex())
	    *reinterpret_cast<int8_t*>(m_reg + r) = v;
        else 
	    *(reinterpret_cast<int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0)) = v;
    }

    inline bool interpreter::test_cc(uint8_t cc) const { 
	switch (cc) { 
	case 0x0: return of();
	case 0x1: return !of();
	case 0x2: return cf();
	case 0x3: return !cf();
	case 0x4: return zf();
	case 0x5: return !zf();
	case 0x6: return cf() || zf();
	case 0x7: return !(cf() || zf());
	case 0x8: return sf();
	case 0x9: return !sf();
	case 0xa: return pf();
	case 0xb: return !pf();
	case 0xc: return sf() != of();
	case 0xd: return sf() == of();
	case 0xe: return zf() || (sf() != of());
	case 0xf: return zf() && (sf() == of());
	}
    }
}

#endif
