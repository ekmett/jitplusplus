#include <ucontext.h>

namespace jitney { 

class interpreter : public tracer { 
	interpreter() : tracer() {}
	void run();
};

static const uint8_t rex_w_mask = 8;
static const uint8_t rex_r_mask = 4;
static const uint8_t rex_x_mask = 2;
static const uint8_t rex_b_mask = 1;
static const uint32_t os32_mask = 512;
static const uint32_t os64_mask = 1024;

#define FLAGGED_BINOP(binop,size,arg) \
    inline void binop(env * env, arg * x, arg * y) { \
        asm (
	    "push %6\n\t" \
	    "popf\n\t" \
	    "mov (%3), %%eax\n\t" \
	    "##binop##l## %%eax, (%4)\n\t" \
	    "pushf\n\t" \
	    "popq %2\n\t" \
	    : "=g"(env->flags), "=+g"(*x) \
	    : "r"(*y), "0"(env->flags), "1"(*x) \
	    : "cc"
	) \
    }

FLAGGED_BINOP(add,l,int32_t);
void add(evaluator * env, int32_t * x, int32_t * y) {
    FLAGGED_BINOP("movl 0(%1), %%eax; add %%eax, 0(%0)");
}

void interpreter::run() { 
    uint8_t * i = e.rip;

start_instruction:
    int32 os = os32_mask;
    bool as32 = true;
    bool as64 = true;
    bool lock = false;
    int simd_prefix; 

    unsigned uint8_t rex = 0;
    void * segment = null;

refetch:
    // legacy, rex, escape, opcode, modrm, sib, displacement, immediate 
    uint32_t prefix = *i++;

    // legacy replaces rex prefix. if we're a rex prefix we'll replace this
    rex = 0; 

    switch (prefix) {
    case 0x0f:
        b = *i++ | 0x100;
	goto refetch;
    case 0x2e: case 0x26: case 0x36: case 0x3e: 
	goto refetch;
    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:    
    case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:    
	rex = prefix;
	goto refetch;
    case 0x64: // fs
	segment = e.fs;
	goto refetch;
    case 0x65: // gs
	segment = e.gs;
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
	esc(env,prefix);
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
}

}
