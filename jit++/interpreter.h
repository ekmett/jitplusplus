#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

#include <jit++/decoder.h>
#include <jit++/tracer.h>
#include <jit++/flags.h>

namespace jitpp { 

    struct os64 {
        typedef int64_t  v;
        typedef int32_t  z;
        typedef int64_t  qv;
        static const int bits = 64;
        static inline const char * reg_name(int r);
    };

    struct os32 {
        typedef int32_t  v;
        typedef int32_t  z;
        typedef int64_t  qv;
        static const int bits = 32;
        static inline const char * reg_name(int r);
    };

    struct os16 {
        typedef int16_t  v;
        typedef int16_t  z;
        typedef int16_t  qv;
        static const int bits = 16;
        static inline const char * reg_name(int r);
    };
    template <typename T> struct os;
    template <> struct os<int16_t> { typedef os16 value; };
    template <> struct os<int32_t> { typedef os32 value; };
    template <> struct os<int64_t> { typedef os64 value; };

    const char * byte_reg_name(int r, bool has_rex);


    class interpreter : public flags::lazy_mixin<tracer> { 
    public:
        interpreter() {}
    private:
        void run();

        // we defer the syscall for these until they are used.
        mutable int64_t m_fs_base;
        mutable int64_t m_gs_base;

        mutable bool m_fs_base_known;
        mutable bool m_gs_base_known;

        int64_t fs_base() const;
        int64_t gs_base() const;

	decoder op;
	void print_regs();
	void print_opcode(int64_t rip, int expected_size = 0);

	inline bool test_cc(uint8_t cc) const;

        inline int32_t eip() const { return static_cast<int32_t>(rip()); }
        inline int64_t rip() const { return m_rip; }
        inline int64_t & rip() { return m_rip; }

	int64_t mem(bool add_segment_base = true) const;

        template <typename T> inline T reg_name<T>(int r) const;

	// fetch a register. if T = int8_t then rex byte existence influences result.
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

        template <typename T> void push(T v);
        template <typename T> T pop();

        inline int8_t & ah();
        inline int8_t ah() const;

	inline int64_t & rsp() { return m_reg[4]; } 
        
	static __attribute__((noreturn)) void illegal(); 	 // illegal opcode encountered, graceful exit, let user code handle it
	static __attribute__((noreturn)) void unsupported(); 	 // unsupported opcode encountered,  graceful exit, let user code handle it
	static __attribute__((noreturn)) void uninterpretable(); // uninterpretable opcode encountered, graceful exit, ...
	static __attribute__((noreturn)) void logic_error();     // uhoh. i misunderstood something

        void interpret_opcode_16();
        void interpret_opcode_32();
        void interpret_opcode_64();
        template <typename T> void interpret_opcode();

        void interpret_locked_opcode_16();
        void interpret_locked_opcode_32();
        void interpret_locked_opcode_64();
        template <typename T> void interpret_locked_opcode();

    };

    template <typename T> inline T interpreter::reg_name<T>(int r) const { 
	typedef typename os<T>::value os; 
	return os::reg_name(r); 
    }

    template <> inline T interpreter::reg_name<int8_t>(int r) const { 
	return byte_reg_name(r,op.has_rex()); 
    }

    inline int8_t & interpreter::ah() { return reinterpret_cast<int8_t*>(m_reg)[1]; } 
    inline int8_t interpreter::ah() const { return reinterpret_cast<const int8_t*>(m_reg)[1]; } 

    // M (mod R/M specifies a memory operand)
    template <typename T> inline T interpreter::M() const { 
        int64_t addr = mem();
        T result = *reinterpret_cast<T*>(addr);
        return result;
    }

    template <typename T> inline void interpreter::M(T v) { 
	int64_t addr = mem()
	VLOG(1) << "*" << std::hex << addr << " = " << v;
        *reinterpret_cast<T*>(mem()) = v; 
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
    template <> inline int32_t interpreter::reg<>(int r) const { return m_reg[r]; } 
    template <> inline int16_t interpreter::reg<>(int r) const { return m_reg[r]; }
    template <> inline int8_t interpreter::reg<>(int r) const { 
        if (op.has_rex()) 
            return *reinterpret_cast<const int8_t*>(m_reg + r);
	else
            return *(reinterpret_cast<const int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0));
    }

    template <> inline void interpreter::reg<>(int r, int64_t v) { 
	VLOG(1) << reg_name<int64_t>(r) << " := " << v;
	m_reg[r] = v; 
    }
    template <> inline void interpreter::reg<>(int r, int32_t v) { 
	VLOG(1) << reg_name<int32_t>(r) << " := " << v;
	m_reg[r] = v; 
    } 
    template <> inline void interpreter::reg<>(int r, int16_t v) { 
	VLOG(1) << reg_name<int16_t>(r) << " := " << v;
	*reinterpret_cast<int16_t*>(m_reg + r) = v; 
    }
    template <> inline void interpreter::reg<>(int r, int8_t v) { 
	VLOG(1) << reg_name<int8_t>(r) << " := " << v;
        if (op.has_rex())
	    *reinterpret_cast<int8_t*>(m_reg + r) = v;
        else 
	    *(reinterpret_cast<int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0)) = v;
    }

    template <> inline void interpreter::reg_name<int64_t>(int r) { 
	return os64::reg_name(r);
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
