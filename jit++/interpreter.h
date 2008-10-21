#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

#include <jit++/tracer.h>

namespace jitpp { 
    class interpreter : public tracer { 
    public:
        interpreter() : tracer() {}
        inline void operator ()() { start(); } 
    private:
        void run();

    private:
        // we defer the syscall for these until they are used.
        mutable int64_t m_fs_base;
        mutable int64_t m_gs_base;

        mutable bool m_fs_base_known;
        mutable bool m_gs_base_known;

        // these are for the interpreter, not the code being traced!
    protected:
        int64_t fs_base() const;
        int64_t gs_base() const;

        int32_t eip() const { return static_cast<int32_t>(rip()); }

        inline int64_t rip () const { return m_rip; }
        inline int64_t & rip() { return m_rip; }

        inline int64_t rflags() const { return m_rflags; }
        inline int64_t & rflags() { return m_rflags; }

        inline int64_t rsp () const { return m_reg[4]; }
        inline int64_t & rsp() { return m_reg[4]; }

        inline int64_t * regs() { return m_reg; }
        inline const int64_t * regs() const { return m_reg; }

        inline mmx_t * mmx() { return m_fx_mmx; }
        inline const mmx_t * mmx() const { return m_fx_mmx; }

        inline xmm_t * xmm() { return m_fx_xmm; }
        inline const xmm_t * xmm() const { return m_fx_xmm; }

        // interpret the tail end of an opcode after the first byte has been read.
        template <typename os, typename as> uint8_t * interpret_opcode(uint8_t * i);

	// fetch a given register. if T = int8_t then rex byte influences result.
        template <typename T> inline T reg(int r) const; 
        template <typename T> inline void reg(int r, T v);

	// M (mod R/M specifies a memory operand)
        template <typename T> inline T M() const { return *reinterpret_cast<T*>(m_M); }
        template <typename T> inline void M(T v) { *reinterpret_cast<T*>(m_M) = v; }

        // G (reg field of mod R/M byte selects a general register)
        template <typename T> inline T G() const { return reg<T>(m_nnn); } 
        template <typename T> inline void G(T v) { reg<T>(m_nnn,v); }

        // R (r/m field of mod R/M byte selects a general register, mod = 3)
        template <typename T> inline T R() const { return reg<T>(m_rm); } 
        template <typename T> inline void R(T v) { reg<T>(m_rm,v); }

        // E (r/m follows opcode and specifies operand, either memory or register)
        template <typename T> inline T E() const { return m_mod == 3 ? R<T>() : M<T>(); }
        template <typename T> inline void E(T v) { if (m_mod == 3) R<T>(v); else M<T>(v); }

        uint8_t & ah() { return reinterpret_cast<uint8_t*>(m_reg)[1]; } 

        template <typename T> void push(T v) { 
            rsp() -= sizeof(T);
	    *reinterpret_cast<T*>(rsp()) = v;
	}
        template <typename T> T pop() { 
	    T result = *reinterpret_cast<T*>(rsp());
            rsp() += sizeof(T);
	    return result;
	}

        int64_t m_M;
        int64_t m_segment_base;
        uint16_t m_opcode;
        uint16_t m_prefix;
        uint8_t m_rex;
        uint8_t m_extra;
        uint8_t m_modrm;
        uint8_t m_mod;
        uint8_t m_nnn;
        uint8_t m_rm;
    };



    template <> inline int64_t interpreter::reg<>(int r) const { return m_reg[r]; }
    template <> inline int32_t interpreter::reg<>(int r) const { return *reinterpret_cast<const int32_t*>(m_reg + r); }
    template <> inline int16_t interpreter::reg<>(int r) const { return *reinterpret_cast<const int16_t*>(m_reg + r); }
    template <> inline int8_t interpreter::reg<>(int r) const { 
        if (m_rex & 0xf != 0)
            return *reinterpret_cast<const int8_t*>(m_reg + r);
	else
            return *(reinterpret_cast<const int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0));
    }

    template <> inline void interpreter::reg<>(int r, int64_t v) { m_reg[r] = v; }
    template <> inline void interpreter::reg<>(int r, int32_t v)  { m_reg[r] = v; } 
    template <> inline void interpreter::reg<>(int r, int16_t v) { *reinterpret_cast<int16_t*>(m_reg + r) = v; }
    template <> void interpreter::reg<>(int r, int8_t v) { 
        if (m_rex & 0xf != 0) 
	    *reinterpret_cast<int8_t*>(m_reg + r) = v;
        else 
	    *(reinterpret_cast<int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0)) = v;
    }


}

#endif
