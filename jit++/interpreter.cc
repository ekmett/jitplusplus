#include <ucontext.h> 
#include <jit++/internal.h>
#include <jit++/interpreter.h>
#include <jit++/exception.h>

namespace jitpp { 

static const uint8_t rex_w_mask = 8;
static const uint8_t rex_r_mask = 4;
static const uint8_t rex_x_mask = 2;
static const uint8_t rex_b_mask = 1;
static const uint32_t os32_mask = 512;
static const uint32_t os64_mask = 1024;

#define FLAGGED_BINOP(binop,size,arg) \
    inline void binop##size (env * env, arg * x, arg * y) { \
        asm ( \
	    "push %6\n\t" \
	    "popf\n\t" \
	    "mov (%3), %%eax\n\t" \
	    "##binop##l## %%eax, (%4)\n\t" \
	    "pushf\n\t" \
	    "popq %2\n\t" \
	    : "=g"(env->flags), "=+g"(*x) \
	    : "r"(*y), "0"(env->flags), "1"(*x) \
	    : "cc" \
	) \
    }

// FLAGGED_BINOP(add,l,int32_t)

#define bail() throw unsupported_opcode_exception((const void*)rip())

void interpreter::run() { 
    try { 
        interpret();   
    } catch (unsupported_opcode_exception & e) { 
        VLOG(1) << e.what();
    }
}

void interpreter::interpret() { 
    uint8_t * i = rip();

start_instruction:
    int32_t os = os32_mask;
    bool as32 = true;
    bool as64 = true;
    bool lock = false;
    int simd_prefix; 

    uint8_t rex = 0;
    void * segment = 0;

refetch:
    // legacy, rex, escape, opcode, modrm, sib, displacement, immediate 
    uint32_t prefix = *i++;

    // legacy replaces rex prefix. if we're a rex prefix we'll replace this
    rex = 0; 

    switch (prefix) {
    case 0x0f:
        prefix = *i++ | 0x100;
	break;
    case 0x2e: case 0x26: case 0x36: case 0x3e: 
	goto refetch;
    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:    
    case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:    
	rex = prefix;
	goto refetch;
    case 0x64: // fs
	segment = fs_base();
	goto refetch;
    case 0x65: // gs
	segment = gs_base();
	goto refetch;
    case 0x66:
	simd_prefix = 1;
        os &= ~os32_mask;
	goto refetch;
    case 0x67: // as64
	as64 = false;
	goto refetch;
    // fpu opcodes
    case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
	bail(); // esc(env,prefix);
    // lock prefix
    case 0xf0:
	lock = true;
	goto refetch;
    default: 
	break;
    }
    if (rex != 0) { 
	if (rex & 0x08) {
	    os = os64_mask | os32_mask;
	}
    }
    bail();
}

}
