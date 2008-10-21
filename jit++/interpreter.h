#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

#include <jit++/tracer.h>

namespace jitpp { 
    class interpreter : public tracer { 
    public:
        interpreter() : tracer() {}
        inline void operator ()() throw() { start(); } 
    private:
        void run() throw();

        // interpret the tail end of an opcode after the first byte has been read.
        template <typename os, typename as> uint8_t * interpret_opcode(uint8_t * i);

	// fetch a given register. if T = int8_t then rex byte influences result.
        template <typename T> inline T reg(int r) const throw(); 
        template <typename T> inline void reg(int r, T v) throw();

	// M (mod R/M specifies a memory operand)
        template <typename T> inline T M() const throw() { return *reinterpret_cast<T*>(m_M); }
        template <typename T> inline void M(T v) throw() { *reinterpret_cast<T*>(m_M) = v; }

        // G (reg field of mod R/M byte selects a general register)
        template <typename T> inline T G() const throw() { return reg<T>(m_nnn); } 
        template <typename T> inline void G(T v) throw() { reg<T>(m_nnn,v); }

        // R (r/m field of mod R/M byte selects a general register, mod = 3)
        template <typename T> inline T R() const throw() { return reg<T>(m_rm); } 
        template <typename T> inline void R(T v) throw() { reg<T>(m_rm,v); }

        // E (r/m follows opcode and specifies operand, either memory or register)
        template <typename T> inline T E() const throw() { return m_mod == 3 ? R<T>() : M<T>(); }
        template <typename T> inline void E(T v) throw() { if (m_mod == 3) R<T>(v); else M<T>(v); }

        uint8_t & ah() throw() { return reinterpret_cast<uint8_t*>(m_reg)[1]; } 

        template <typename T> void push(T v) throw() { 
            rsp() -= sizeof(T);
	    *reinterpret_cast<T*>(rsp()) = v;
	}
        template <typename T> T pop() throw() { 
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



    template <> inline int64_t interpreter::reg<>(int r) const throw() { return m_reg[r]; }
    template <> inline int32_t interpreter::reg<>(int r) const throw() { return *reinterpret_cast<const int32_t*>(m_reg + r); }
    template <> inline int16_t interpreter::reg<>(int r) const throw() { return *reinterpret_cast<const int16_t*>(m_reg + r); }
    template <> inline int8_t interpreter::reg<>(int r) const throw() { 
        if (m_rex & 0xf != 0)
            return *reinterpret_cast<const int8_t*>(m_reg + r);
	else
            return *(reinterpret_cast<const int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0));
    }

    template <> inline void interpreter::reg<>(int r, int64_t v) throw() { m_reg[r] = v; }
    template <> inline void interpreter::reg<>(int r, int32_t v) throw()  { m_reg[r] = v; } 
    template <> inline void interpreter::reg<>(int r, int16_t v) throw() { *reinterpret_cast<int16_t*>(m_reg + r) = v; }
    template <> void interpreter::reg<>(int r, int8_t v) throw() { 
        if (m_rex & 0xf != 0) 
	    *reinterpret_cast<int8_t*>(m_reg + r) = v;
        else 
	    *(reinterpret_cast<int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0)) = v;
    }


}

#endif
