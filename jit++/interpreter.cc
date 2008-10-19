#include <ucontext.h> 
#include <iostream>
#include <jit++/internal.h>
#include <jit++/interpreter.h>
#include <jit++/exception.h>
#include <jit++/opcodes.h>

namespace jitpp { 

static const uint8_t lock_mask = 64;
static const uint8_t prefix_67h_mask = 32;
static const uint8_t prefix_66h_mask = 16;
static const uint8_t rex_w_mask = 8;
static const uint8_t rex_r_mask = 4;
static const uint8_t rex_x_mask = 2;
static const uint8_t rex_b_mask = 1;
static const int64_t cf_mask = 0x0001;
static const int64_t pf_mask = 0x0004;
static const int64_t af_mask = 0x0010;
static const int64_t zf_mask = 0x0040;
static const int64_t sf_mask = 0x0080;
static const int64_t tf_mask = 0x0100;
static const int64_t if_mask = 0x0200;
static const int64_t df_mask = 0x0400;

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

#define die() throw unsupported_opcode_exception((const void*)rip())
#define illegal() throw unsupported_opcode_exception((const void*)rip())

struct size64 { 
    typedef int64_t  signed_type;
    typedef uint64_t unsigned_type;
    typedef int32_t  signed_max32; 
    typedef uint32_t unsigned_max32;
    typedef int16_t  signed_max16;
    typedef uint16_t unsigned_max16;
    typedef int64_t  signed_default64;
    typedef uint64_t unsigned_default64;
    static const int bits = 64;
};

struct size32 { 
    typedef int32_t  signed_type;
    typedef uint32_t unsigned_type ;
    typedef int32_t  signed_max32;
    typedef uint32_t unsigned_max32;
    typedef int16_t  signed_max16;
    typedef uint16_t unsigned_max16;
    typedef int64_t  signed_default64;
    typedef uint64_t unsigned_default64;
    static const int bits = 32;
};

struct size16 { 
    typedef int16_t  signed_type;
    typedef uint16_t unsigned_type ;
    typedef int16_t  signed_max32;
    typedef uint16_t unsigned_max32;
    typedef int16_t  signed_max16;
    typedef uint16_t unsigned_max16;
    typedef int16_t  signed_default64;
    typedef uint16_t unsigned_default64;
    static const int bits = 16;
};

struct size8 { 
    typedef int8_t  signed_type;
    typedef uint8_t unsigned_type ;
    typedef int8_t  signed_max32;
    typedef uint8_t unsigned_max32;
    typedef int8_t  signed_max16;
    typedef uint8_t unsigned_max16;
    static const int bits = 8;
};

template <typename T> T fetch(uint8_t * & i) { 
    T result = *reinterpret_cast<T*>(i);
    i += sizeof(result);
    return result;
}

// assumes x86-64 long mode 
// TODO: check for correct handling of 66h on e0-e3,70-7f,eb,e9,ff/4,e8,ff/2,c3,c2
void interpreter::run() { 
    try { 
        do { 
            uint8_t * i = reinterpret_cast<uint8_t *>(rip());
            m_rex = 0;
            m_segment_base = 0;
        
        refetch:
            // legacy, rex, escape, opcode, modrm, sib, displacement, immediate 
            m_opcode = fetch<uint8_t>(i);
        
            // any legacy prefix replaces rex prefix.
            m_rex = 0; 
        
            switch (m_opcode) {
            case 0x0f:
                m_opcode = 0x100 | fetch<uint8_t>(i);
                break;
            case 0x2e: case 0x26: case 0x36: case 0x3e: goto refetch;
        
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:    
            case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f: // rex    
                m_rex = m_opcode & 0xf;
                goto refetch;
            case 0x64: // fs
                m_segment_base = fs_base();
                goto refetch;
            case 0x65: // gs
                m_segment_base = gs_base();
                goto refetch;
            case 0x66: // os 66h
                m_rex |= prefix_66h_mask;
                goto refetch;
            case 0x67: // as 67h
                m_rex |= prefix_67h_mask;
                goto refetch;
            case 0xf0: // lock
                m_rex |= lock_mask;
                goto refetch;
            default: 
                break;
            }
            if (m_rex & prefix_67h_mask == 0) { 
                if (m_rex & rex_w_mask != 0) 
		    rip() = reinterpret_cast<int64_t>(interpret_opcode<size64,size64>(i));
                else if (m_rex & prefix_66h_mask == 0) 
		    rip() = reinterpret_cast<int64_t>(interpret_opcode<size32,size64>(i));
                else 
		    rip() = reinterpret_cast<int64_t>(interpret_opcode<size16,size64>(i));
            } else { 
                if (m_rex & rex_w_mask != 0) 
		    rip() = reinterpret_cast<int64_t>(interpret_opcode<size64,size32>(i));
                else if (m_rex & prefix_66h_mask == 0) 
		    rip() = reinterpret_cast<int64_t>(interpret_opcode<size32,size32>(i));
                else 
		    rip() = reinterpret_cast<int64_t>(interpret_opcode<size16,size32>(i));
            }
        } while (true);
    } catch (unsupported_opcode_exception & e) { 
        std::cout << e.what();
    }
}

// assumes: m_rex and m_opcode have been initialized appropriately based on the prefix
template <typename os, typename as> uint8_t * interpreter::interpret_opcode(uint8_t * i) { 
    uint8_t modrm_flags = modrm_flag_lut[m_opcode];
    if (modrm_flags & modrm_flag_has_modrm != 0) { 
	m_extra = modrm_flags & modrm_flag_extra_byte != 0 ? fetch<uint8_t>(i) : 0;
	m_modrm = fetch<uint8_t>(i);
	m_mod = m_modrm >> 6;
	m_nnn = ((m_modrm >> 3) & 7) | ((m_rex & 4) << 1);
	m_rm = (m_modrm & 7) | ((m_rex & 1) << 3);

	if (m_mod != 3) { 
	    if (as::bits == 64) {
                if (m_rm & 7 == 4) { // sib needed
                    uint8_t sib = fetch<uint8_t>(i);
                    uint8_t scale = sib >> 6;
                    uint8_t index = ((sib >> 3) & 7) | ((m_rex & 2) << 2);
                    uint8_t base = (sib & 7) | ((m_rex & 1) << 3);
                    uint64_t rindex = index == 4 ? 0 : (m_reg[index] << scale);
                    uint64_t rbase = m_reg[base];
                    if (base == 5) {
                        switch (m_mod) {
                        case 0: rbase = fetch<int32_t>(i); break;
                        case 1: rbase += fetch<int8_t>(i); break;
                        case 2: rbase += fetch<int32_t>(i); break;
                        }
                    }
                    m_M = m_segment_base + rindex + rbase;
                } else { // no sib
                    switch (m_mod) { 
                    case 0: // rip relative or [rXX]
                        if (m_rm & 7 == 5) m_M = m_segment_base + rip() + fetch<int8_t>(i);
                        else m_M = m_segment_base + m_reg[m_rm];
                        break;
                    case 1: 
			// [rXX + int8]
                        m_M = m_segment_base + m_reg[m_rm] + fetch<int8_t>(i);
                        break;
                    case 2: 
			// [rXX + int32]
                        m_M = m_segment_base + m_reg[m_rm] + fetch<int32_t>(i);
                        break;
                    } 
                }
	    } else die(); // mod != 3, as::bits != 64 unsupported
        }
    }

    typedef int8_t b;
    typedef int8_t w;
    typedef typename os::signed_type v;
    typedef typename os::signed_max32 z32;
    typedef typename os::signed_max16 z16;
    typedef typename os::signed_default64 d64;

    if (m_rex & lock_mask != 0) die(); // lock unimplemented

    switch (m_opcode) { 
    case 0x06: illegal(); // PUSH ES
    case 0x07: illegal(); // POP ES
    case 0x0e: illegal(); // PUSH CS
    case 0x16: illegal(); // PUSH SS
    case 0x17: illegal(); // POP SS
    case 0x1e: illegal(); // PUSH DS
    case 0x1f: illegal(); // POP DS
    case 0x27: illegal(); // DAA
    case 0x2f: illegal(); // DAS
    case 0x37: illegal(); // AAA
    case 0x3f: illegal(); // AAS
    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	{
	    // PUSH rXX
            int r = (m_opcode & 7) + ((m_rex & 1) << 3);
	    push<d64>(reg<d64>(r));
	    break;
	}
    case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: 
	{ 
	    // POP rXX
            int r = (m_opcode & 7) + ((m_rex & 1) << 3);
	    reg<d64>(r, pop<d64>());
	    break;
	}
    case 0x60: illegal(); // PUSHA
    case 0x61: illegal(); // POPA
    case 0x62: illegal(); // BOUND Gv,Ma
    case 0x82: illegal(); // group #1 Eb, Ib
    case 0x88: // MOV Eb,Gb
        E<b>(G<b>());
        return i;
    case 0x89: // MOV Ev,Gv
        E<v>(G<v>()); 
        return i;
    case 0x8a: // MOV Gb,Eb
        G<b>(E<b>()); 
        return i;
    case 0x8b: // Mov Gv,Ev
        G<v>(E<v>()); 
        return i;
    case 0x8c: 
	die();
        // if (m_mod == 3) R<v>(0); // MOV Rv,Sw
        // else M<w>() = 0;         // MOV Mw,Sw
	// return i;
    case 0x8d: // LEA Gv,M
	G<v>(m_M);
	return i;
    case 0x8e: 
	die();
	// return i; // MOV Sw, Mw or MOV Sw,Rv
    case 0x8f: // POP Ev (8F /0)
	if (m_nnn != 0) illegal();
	E<d64>(pop<d64>());
	return i;
    case 0x9a: illegal(); // CALL Ap
    case 0x9e: // SAHF if CPUID.AHF = 1
        rflags() = (rflags() & ~0xff) | ah(); 
        return i;
    case 0x9f: // LAHF if CPUID.AHF = 1
        ah() = static_cast<uint8_t>(rflags() & 0xff);
	return i;
    case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf: 
	// MOV RXX, Iq
	{
            int r = (m_opcode & 7) + ((m_rex & 1) << 3);
            reg<v>(r,fetch<z32>(i));
	    return i;
	}
    case 0xc4: illegal(); // LES Gz,Mp
    case 0xc5: illegal(); // LDS Gz,Mp
    case 0xce: illegal(); // INTO
    case 0xd4: illegal(); // AAM Ib
    case 0xd5: illegal(); // AAD Ib
    case 0xd6: illegal(); // SALC
    case 0xe9: // JMP Jz
	return reinterpret_cast<uint8_t*>(rip() + fetch<z32>(i));
    case 0xea: illegal(); // JMP Ap
    case 0xf4: die(); // HLT
    case 0xf8: // CLC
	rflags() &= ~cf_mask; return i;
    case 0xf9: // STC
	rflags() |= cf_mask; return i;
    case 0xfa: // CLI
	asm("cli"); rflags() &= ~if_mask; return i;
    case 0xfb: // STI
	asm("sti"); rflags() |= if_mask; return i;
    case 0xfc: // CLD
	rflags() &= ~df_mask; return i;
    case 0xfd: // STD
	rflags() &= ~df_mask; return i;
    case 0xff: // opcode group 5
	switch (m_nnn) { 
	case 6: push<d64>(E<d64>()); return i; // PUSH Ev
	default: die();
	}
    case 0x120: die(); // MOV Rd,Cd
    case 0x121: die(); // MOV Rd,Dd
    case 0x122: die(); // MOV Cd,Rd
    case 0x123: die(); // MOV Dd,Rd
    case 0x134: die(); // SYSENTER (illegal on AMD64, legal on EMT64?, unrecognized by udis86)
    case 0x135: die(); // SYSEXIT (illegal on AMD64, legal on EMT64)
    case 0x1a0: die(); // push<d64>(fs()); return i; // PUSH FS 
    case 0x1a8: die(); // push<d64>(gs()); return i; // PUSH GS
    case 0x1a1: die(); // fs(pop<d64>()); return i;  // POP FS
    case 0x1a9: die(); // gs(pop<d64>()); return i;  // POP GS
    default: break;
    } 
/*
    if (m_opcode & 0x1c0 == 0) { 
        // arithmetic opcode matrix
        arithmetic_op op = arithops[opcode & 0x38 >> 4];
	switch (m_opcode & 7) { 
	case 0: op.rebind<int8_t>(E<int8_t>,G<int8_t>,this,i); // XXX Eb,Gb
	case 1: op.EvGv(this); // XXX Ev,Gv
	case 2: op.GbEb(this); // XXX Gb,Eb
	case 3: op.GvEv(this); // XXX Gv,Ev
	case 4: op.AlIb(this); // XXX AL,Ib
	case 5: op.rAXIz(this);// XXX rAX,Iz
	    die();
	default: 
            // PUSH ES, PUSH SS, POP ES, POP SS, DAA, AAA, PUSH CS, PUSH DS, POP DS, DAS, AAS illegal
            // 0F, ES, SS, CS, DS prefixes handled before entering interpret_opcode
	    LOG(DFATAL) << "Logic error in tracer"; 
	    die();
	}
    }
*/   
    die();
} // interpreter::interpret

} // namespace jitpp
