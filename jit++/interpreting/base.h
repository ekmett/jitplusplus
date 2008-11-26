#ifndef INCLUDED_JITPP_INTERPRETING_BASE_H
#define INCLUDED_JITPP_INTERPRETING_BASE_H

#include <jit++/common.h>
#include <jit++/interpreting/decoder.h>
#include <jit++/interpreting/tracer.h>
#include <jit++/interpreting/flags.h>

namespace jitpp { 
  namespace interpreting { 

    struct os64 {
        typedef int64_t  v;
        typedef int32_t  z;
        typedef int64_t  qv;
	typedef int32_t  smaller_size;
        static const int bits = 64;
        static const char * reg_name(int r);
    };

    struct os32 {
        typedef int32_t  v;
        typedef int32_t  z;
        typedef int64_t  qv;
	typedef int16_t  smaller_size;
        static const int bits = 32;
        static const char * reg_name(int r);
    };

    struct os16 {
        typedef int16_t  v;
        typedef int16_t  z;
        typedef int16_t  qv;
	typedef int8_t   smaller_size;
        static const int bits = 16;
        static const char * reg_name(int r);
    };

    template <typename T> struct os;
    template <> struct os<int16_t> { typedef os16 value; };
    template <> struct os<int32_t> { typedef os32 value; };
    template <> struct os<int64_t> { typedef os64 value; };

    class interpreter_base : public flags::lazy_mixin<tracer>, public decoder { 
    public:
        virtual void run() = 0;

    protected:
        // we defer the syscall for these until they are used.
        mutable int64_t m_fs_base;
        mutable int64_t m_gs_base;

        mutable bool m_fs_base_known;
        mutable bool m_gs_base_known;

    public:
        int64_t fs_base() const;
        int64_t gs_base() const;
	int64_t seg_base() const; 

	void print_regs();
	void print_opcode(int64_t rip, int expected_size = 0);
	void print_address(int64_t addr);

        inline int32_t eip() const { return static_cast<int32_t>(rip()); }
        inline int64_t rip() const { return m_rip; }
        inline int64_t & rip() { return m_rip; }

	int64_t repetitions() const;
	int64_t mem(bool add_segment_base = true) const;

        inline int8_t & ah();
        inline int8_t ah() const;

	inline int64_t & rax() { return m_reg[0]; } 
	inline int64_t & rcx() { return m_reg[1]; } 
	inline int64_t & rdx() { return m_reg[2]; }
	inline int64_t & rbx() { return m_reg[3]; }
	inline int64_t & rsp() { return m_reg[4]; } 
	inline int64_t & rbp() { return m_reg[5]; } 
	inline int64_t & rsi() { return m_reg[6]; }
	inline int64_t & rdi() { return m_reg[7]; }
	inline int64_t & r8() { return m_reg[8]; }
	inline int64_t & r9() { return m_reg[9]; }
	inline int64_t & r10() { return m_reg[10]; }
	inline int64_t & r11() { return m_reg[11]; }
	inline int64_t & r12() { return m_reg[12]; }
	inline int64_t & r13() { return m_reg[13]; }
	inline int64_t & r14() { return m_reg[14]; }
	inline int64_t & r15() { return m_reg[15]; }

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

        template <typename T> inline const char * reg_name(int r) const;
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
    };

    extern const char * byte_reg_name(int r, bool has_rex);
    template <typename T> inline const char * interpreter_base::reg_name(int r) const {
	typedef typename os<T>::value os; 
	return os::reg_name(r); 
    }
    template <> inline const char * interpreter_base::reg_name<int8_t>(int r) const {
        return byte_reg_name(r,has_rex()); 
    }



    // TODO: make illegal and logic_error into cold calls?
    extern __attribute__((noreturn)) void illegal(); 	     // illegal opcode encountered, graceful exit, let user code handle it
    extern __attribute__((noreturn)) void unsupported();     // unsupported opcode encountered,  graceful exit, let user code handle it
    extern __attribute__((noreturn)) void uninterpretable(); // uninterpretable opcode encountered, graceful exit, ...
    extern __attribute__((noreturn)) void logic_error();     // uhoh. i misunderstood something

    static const char * reg_spaces = "                   ";
    static const char * mem_spaces = "                  ";

    template <> inline int64_t interpreter_base::get_reg<>(int r) const { return m_reg[r]; }
    template <> inline int32_t interpreter_base::get_reg<>(int r) const { return m_reg[r]; } 
    template <> inline int16_t interpreter_base::get_reg<>(int r) const { return m_reg[r]; }
    template <> inline int8_t interpreter_base::get_reg<>(int r) const { 
	int8_t result = has_rex() 
	    ?  *reinterpret_cast<const int8_t*>(m_reg + r)
            : *(reinterpret_cast<const int8_t*>(m_reg + (r & 3)) + (((r & 4) != 0) ? 1 : 0));
	VLOG(1) << reg_spaces << reg_name<int8_t>(r) << " -> " << std::hex << (int64_t)result;
	return result;
    }
 
    template <> inline void interpreter_base::set_reg<>(int r, int64_t v) { 
	VLOG(1) << reg_spaces << reg_name<int64_t>(r) << " := " << std::hex << (int64_t)v;
	m_reg[r] = v; 
    }
    template <> inline void interpreter_base::set_reg<>(int r, int32_t v) { 
	VLOG(1) << reg_spaces << reg_name<int32_t>(r) << " := " << std::hex << (int64_t)v;
	m_reg[r] = v; 
    } 
    template <> inline void interpreter_base::set_reg<>(int r, int16_t v) { 
	VLOG(1) << reg_spaces << reg_name<int16_t>(r) << " := " << std::hex << (int64_t)v;
	*reinterpret_cast<int16_t*>(m_reg + r) = v; 
    }
    template <> inline void interpreter_base::set_reg<>(int r, int8_t v) { 
	VLOG(1) << reg_spaces << reg_name<int8_t>(r) << " := " << std::hex << (int64_t)v;
        if (has_rex())
	    *reinterpret_cast<int8_t*>(m_reg + r) = v;
        else 
	    *(reinterpret_cast<int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0)) = v;
    }

    // M (r/m field of mod R/M byte selects a memory operand (mod == 3)
    template <typename T> inline T interpreter_base::M() const { 
	int64_t addr = mem();
        T result = *reinterpret_cast<T*>(addr);
	VLOG(1) << mem_spaces << "*" << std::hex << addr << " -> " << std::hex << (int64_t)result;
	return result;
    }

    template <typename T> inline void interpreter_base::M(T v) { 
	int64_t addr = mem();
	VLOG(1) << mem_spaces << "*" << std::hex << addr << " := " << std::hex << (int64_t)v;
        *reinterpret_cast<T*>(addr) = v; 
    }

    // G (reg field of mod R/M byte selects a general register)
    template <typename T> inline T interpreter_base::G() const{ 
        return get_reg<T>(reg); 
    } 
    template <typename T> inline void interpreter_base::G(T v) { 
        set_reg<T>(reg,v); 
    }

    // R (r/m field of mod R/M byte selects a general register)
    template <typename T> inline T interpreter_base::R() const { 
        return get_reg<T>(rm); 
    } 
    template <typename T> inline void interpreter_base::R(T v) { 
        set_reg<T>(rm,v); 
    }

    // E (r/m follows opcode and specifies operand, either memory or register)
    template <typename T> inline T interpreter_base::E() const { 
            return mod == 3 ? R<T>() : M<T>(); 
    }
    template <typename T> inline void interpreter_base::E(T v) { 
        if (mod == 3) R<T>(v); 
        else M<T>(v); 
    }

    template <typename T> void interpreter_base::push(T v) { 
        rsp() -= sizeof(T);
        *reinterpret_cast<T*>(rsp()) = v;
    }

    template <typename T> T interpreter_base::pop() { 
        T result = *reinterpret_cast<T*>(rsp());
        rsp() += sizeof(T);
        return result;
    }

    inline int8_t & interpreter_base::ah() { return reinterpret_cast<int8_t*>(m_reg)[1]; } 
    inline int8_t interpreter_base::ah() const { return reinterpret_cast<const int8_t*>(m_reg)[1]; } 
  } 
} // namespace jitpp::interpreting

#endif
