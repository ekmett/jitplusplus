#ifndef INCLUDED_JITPP_INTERPRETER_INTERNAL_H
#define INCLUDED_JITPP_INTERPRETER_INTERNAL_H

#include <jit++/common.h>
#include <jit++/interpreter.h>

// allow variadic macros in c++
#pragma GCC system_header

namespace jitpp { 

    struct os64 {
        typedef int64_t  v;
        typedef int32_t  z;
        typedef int64_t  qv;
        static const int bits = 64;
        static const char * reg_name(int r);
    };

    struct os32 {
        typedef int32_t  v;
        typedef int32_t  z;
        typedef int64_t  qv;
        static const int bits = 32;
        static const char * reg_name(int r);
    };

    struct os16 {
        typedef int16_t  v;
        typedef int16_t  z;
        typedef int16_t  qv;
        static const int bits = 16;
        static const char * reg_name(int r);
    };

    template <typename T> struct os;
    template <> struct os<int16_t> { typedef os16 value; };
    template <> struct os<int32_t> { typedef os32 value; };
    template <> struct os<int64_t> { typedef os64 value; };

    extern const char * byte_reg_name(int r, bool has_rex);

    extern __attribute__((noreturn)) void illegal(); 	     // illegal opcode encountered, graceful exit, let user code handle it
    extern __attribute__((noreturn)) void unsupported();     // unsupported opcode encountered,  graceful exit, let user code handle it
    extern __attribute__((noreturn)) void uninterpretable(); // uninterpretable opcode encountered, graceful exit, ...
    extern __attribute__((noreturn)) void logic_error();     // uhoh. i misunderstood something

    extern void interpret_opcode_16(interpreter & i);
    extern void interpret_opcode_32(interpreter & i);
    extern void interpret_opcode_64(interpreter & i);

    extern void interpret_locked_opcode_16(interpreter & i);
    extern void interpret_locked_opcode_32(interpreter & i);
    extern void interpret_locked_opcode_64(interpreter & i);

    template <typename T> static inline const char * reg_name(const interpreter & i, int r) {
	typedef typename os<T>::value os; 
	return os::reg_name(r); 
    }

    template <> static inline const char * reg_name<int8_t>(const interpreter & i, int r) {
	return byte_reg_name(r,i.has_rex()); 
    }
    template <typename T> static inline T get_reg(const interpreter & i, int r);
    template <typename T> static inline void set_reg(interpreter & i, int r, T v);
    template <typename T> static inline T M(const interpreter & i);
    template <typename T> static inline void M(interpreter & i, T v);
    template <typename T> static inline T G(const interpreter & i);
    template <typename T> static inline void G(interpreter & i, T v);
    template <typename T> static inline T R(const interpreter & i);
    template <typename T> static inline void R(interpreter & i,T v);
    template <typename T> static inline T E(const interpreter & i);
    template <typename T> static inline void E(interpreter & i, T v);
    template <typename T> static void push(interpreter & i, T v);
    template <typename T> static T pop(interpreter & i);

    template <> inline int64_t get_reg<>(const interpreter & i, int r) { return i.m_reg[r]; }
    template <> inline int32_t get_reg<>(const interpreter & i, int r) { return i.m_reg[r]; } 
    template <> inline int16_t get_reg<>(const interpreter & i, int r) { return i.m_reg[r]; }
    template <> inline int8_t get_reg<>(const interpreter & i, int r) { 
        if (i.has_rex()) 
            return *reinterpret_cast<const int8_t*>(i.m_reg + r);
	else
            return *(reinterpret_cast<const int8_t*>(i.m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0));
    }

    template <> inline void set_reg<>(interpreter & i, int r, int64_t v) { 
	VLOG(1) << reg_name<int64_t>(i,r) << " := " << v;
	i.m_reg[r] = v; 
    }
    template <> inline void set_reg<>(interpreter & i, int r, int32_t v) { 
	VLOG(1) << reg_name<int32_t>(i,r) << " := " << v;
	i.m_reg[r] = v; 
    } 
    template <> inline void set_reg<>(interpreter & i, int r, int16_t v) { 
	VLOG(1) << reg_name<int16_t>(i,r) << " := " << v;
	*reinterpret_cast<int16_t*>(i.m_reg + r) = v; 
    }
    template <> inline void set_reg<>(interpreter & i, int r, int8_t v) { 
	VLOG(1) << reg_name<int8_t>(i,r) << " := " << v;
        if (i.has_rex())
	    *reinterpret_cast<int8_t*>(i.m_reg + r) = v;
        else 
	    *(reinterpret_cast<int8_t*>(i.m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0)) = v;
    }


    // M (r/m field of mod R/M byte selects a memory operand (mod == 3)
    template <typename T> inline T M(const interpreter & i) { 
        int64_t addr = i.mem();
        T result = *reinterpret_cast<T*>(addr);
        return result;
    }

    template <typename T> inline void M(interpreter & i, T v) { 
	int64_t addr = i.mem();
	VLOG(1) << "*" << std::hex << addr << " := " << v;
        *reinterpret_cast<T*>(addr) = v; 
    }

    // G (reg field of mod R/M byte selects a general register)
    template <typename T> inline T G(const interpreter & i) { 
        return get_reg<T>(i,i.reg); 
    } 
    template <typename T> inline void G(interpreter & i, T v) { 
        set_reg<T>(i,i.reg,v); 
    }

    // R (r/m field of mod R/M byte selects a general register)
    template <typename T> inline T R(const interpreter & i) { 
        return get_reg<T>(i,i.rm); 
    } 
    template <typename T> inline void R(interpreter & i, T v) { 
        set_reg<T>(i,i.rm,v); 
    }

    // E (r/m follows opcode and specifies operand, either memory or register)
    template <typename T> inline T E(const interpreter & i) { 
            return i.mod == 3 ? R<T>(i) : M<T>(i); 
    }
    template <typename T> inline void E(interpreter & i, T v) { 
        if (i.mod == 3) R<T>(i,v); 
        else M<T>(i,v); 
    }

    template <typename T> void push(interpreter & i, T v) { 
        i.rsp() -= sizeof(T);
        *reinterpret_cast<T*>(i.rsp()) = v;
    }

    template <typename T> T pop(interpreter & i) { 
        T result = *reinterpret_cast<T*>(i.rsp());
        i.rsp() += sizeof(T);
        return result;
    }
}

#endif
