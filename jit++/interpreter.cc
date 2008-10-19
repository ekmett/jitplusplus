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
	    "push %4\n\t" \
	    "popf\n\t" \
	    "mov (%3), %%eax\n\t" \
	    "##binop##l## %%eax, (%1)\n\t" \
	    "pushf\n\t" \
	    "popq %0\n\t" \
	    : "=g"(env->flags), "=+g"(*x) \
	    : "r"(*y), "0"(env->flags), "1"(*x) \
	    : "cc" \
	) \
    }

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
    static const char * reg_names[16];
    static inline const char * reg_name(int r) throw() { 
	return reg_names[r];
    }
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
    static const char * reg_names[16];
    static inline const char * reg_name(int r) throw() { 
	return reg_names[r];
    }
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
    static const char * reg_names[16];
    static inline const char * reg_name(int r) throw() { 
	return reg_names[r];
    }
};

struct size8 { 
    typedef int8_t  signed_type;
    typedef uint8_t unsigned_type ;
    typedef int8_t  signed_max32;
    typedef uint8_t unsigned_max32;
    typedef int8_t  signed_max16;
    typedef uint8_t unsigned_max16;
    static const int bits = 8;
    static const char * reg_names_rex[16];
    static const char * reg_names_norex[8];
    static inline const char * reg_name(int r, bool has_rex) throw() { 
	return has_rex ? reg_names_rex[r] : reg_names_norex[r];
    }
};

const char * size64::reg_names[16] = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
const char * size32::reg_names[16] = {"eax","ecx","edx","ebx","esp","ebp","esi","edi","r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"};
const char * size16::reg_names[16] = {"ax","cx","dx","bx","sp","bp","si","di","r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w"};
const char * size8::reg_names_rex[16] = {"al","cl","dl","bl","spl","bpl","sil","dil","r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"};
const char * size8::reg_names_norex[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};

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
 	    unsupported_opcode_exception current_opcode((const void*)rip());
	    std::cout << current_opcode.what();
            uint8_t * i = reinterpret_cast<uint8_t *>(rip());
            m_rex = 0;
            m_segment_base = 0;
        
        refetch:
            // legacy, rex, escape, opcode, modrm, sib, displacement, immediate 
            m_opcode = fetch<uint8_t>(i);
	    VLOG(1) << "m_opcode = " << std::hex << (int)m_opcode;
        
            // any legacy prefix replaces rex prefix.
        
            switch (m_opcode) {
            case 0x0f:
                m_opcode = 0x100 | fetch<uint8_t>(i);
                break;
            case 0x2e: case 0x26: case 0x36: case 0x3e: goto refetch;
        
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:    
            case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f: // rex    
                m_rex |= m_opcode & 0x0f;
                goto refetch;
            case 0x64: // fs
                m_rex &= 0xf0; 
                m_segment_base = fs_base();
                goto refetch;
            case 0x65: // gs
                m_rex &= 0xf0; 
                m_segment_base = gs_base();
                goto refetch;
            case 0x66: // os 66h
                m_rex &= 0xf0; 
                m_rex |= prefix_66h_mask;
                goto refetch;
            case 0x67: // as 67h
                m_rex &= 0xf0; 
                m_rex |= prefix_67h_mask;
                goto refetch;
            case 0xf0: // lock
                m_rex &= 0xf0; 
                m_rex |= lock_mask;
                goto refetch;
            default: 
                break;
            }
	    switch ((m_rex & 0x38) >> 3) { 
	    case 0x0: rip() = reinterpret_cast<int64_t>(interpret_opcode<size32,size64>(i)); break; // 
	    case 0x2: rip() = reinterpret_cast<int64_t>(interpret_opcode<size16,size64>(i)); break; // 66
	    case 0x4: rip() = reinterpret_cast<int64_t>(interpret_opcode<size32,size32>(i)); break; // 67
	    case 0x6: rip() = reinterpret_cast<int64_t>(interpret_opcode<size16,size32>(i)); break; // 67 66
	    case 0x1: // fall through                                                               // rex
	    case 0x3: rip() = reinterpret_cast<int64_t>(interpret_opcode<size64,size64>(i)); break; // (66 ignored) rex
	    case 0x5: // fall through								    // 67 rex
	    case 0x7: rip() = reinterpret_cast<int64_t>(interpret_opcode<size64,size32>(i)); break; // 67 (66 ignored) rex
	    }
        } while (true);
    } catch (unsupported_opcode_exception & e) { 
        std::cout << e.what();
    }
}

// assumes: m_rex and m_opcode have been initialized appropriately based on the prefix
template <typename os, typename as> uint8_t * interpreter::interpret_opcode(uint8_t * i) { 
    typedef int8_t b;
    typedef int8_t w;
    typedef typename as::signed_type asv;
    typedef typename os::signed_type v;
    typedef typename os::signed_max32 z32;
    typedef typename os::signed_max16 z16;
    typedef typename os::signed_default64 d64;

    VLOG(1) << "os " << os::bits << " as " << as::bits;

    uint8_t modrm_flags = modrm_flag_lut[m_opcode];
    VLOG(1) << "modrm flags " << std::hex << (int)modrm_flags;
    if (modrm_flags & modrm_flag_has_modrm != 0) { 
        LOG(INFO) << "parsing mod r/m";
	if (modrm_flags & 2 == 2) { 
	    m_extra = fetch<uint8_t>(i);
            VLOG(1) << "extra " << std::hex << (int)m_extra;
	}
	m_modrm = fetch<uint8_t>(i);
	m_mod = m_modrm >> 6;
	m_nnn = ((m_modrm >> 3) & 7) | ((m_rex & 4) << 1);
	m_rm = (m_modrm & 7) | ((m_rex & 1) << 3);
	VLOG(1) << "modrm " << (int)m_modrm << " mod " << (int)m_mod << " nnn " << (int)m_nnn << " rm " << (int)m_rm;

	if (m_mod != 3) { 
            if (m_rm & 7 == 4) { // sib needed
		VLOG(1) << "SIB";
                uint8_t sib = fetch<uint8_t>(i);
                uint8_t scale = sib >> 6;
                uint8_t index = ((sib >> 3) & 7) | ((m_rex & 2) << 2);
                uint8_t base = (sib & 7) | ((m_rex & 1) << 3);
                int64_t rindex = index == 4 ? 0 : (reg<asv>(index) << scale);
	 	VLOG(1) << "index " << (index == 4 ? "(none)" : as::reg_name(index)) << " * scale " << scale << " = " << std::hex << rindex;
                int64_t rbase = reg<v>(base);
		VLOG(1) << "base reg " << os::reg_name(base);
                if (base == 5) {
                    switch (m_mod) {
                    case 0: 
			rbase = fetch<int32_t>(i); break;
			VLOG(1) << "base = " << std::hex << rbase;
                    case 1: 
			{
			    int8_t b = fetch<int8_t>(i);
			    VLOG(1) << "base = " << os::reg_name(base) << " + " << std::hex << (int)b << "b";
		            rbase += fetch<int8_t>(i); break;
			}
                    case 2: 
			{
			    int32_t d = fetch<int32_t>(i);
			    VLOG(1) << "base = " << os::reg_name(base) << " + " << std::hex << (int)d << "i";
			    rbase += d;
			    break;
			}
                    }
                }
                m_M = m_segment_base + rindex + rbase;
            } else { // no sib
		VLOG(1) << "no SIB";
                switch (m_mod) { 
                case 0: // rIP relative or [rXX]
                    if (m_rm & 7 == 5) {
			int8_t b = fetch<int8_t>(i);
			if (as::bits == 64) { 
			    m_M = m_segment_base + rip() + b;
			    VLOG(1) << "[RIP + " << b << "b]";
			} else { 
			    VLOG(1) << "[EIP + " << b << "b]";
			    m_M = m_segment_base + eip() + b;
			}
		    } else { 
			VLOG(1) << "[" << as::reg_name(m_rm) << "]";
                        m_M = m_segment_base + reg<asv>(m_rm);
		    }
                    break;
                case 1: // [rXX + int8]
		    {
			int8_t b = fetch<int8_t>(i);
		        VLOG(1) << "[" << as::reg_name(m_rm) << " + " << b << "b]";
                        m_M = m_segment_base + reg<asv>(m_rm) + b;
                        break;
		    }
                case 2: // [rXX + int32]
		    {
			int32_t b = fetch<int32_t>(i);
			VLOG(1) << "[" << as::reg_name(m_rm) << " + " << i << "i]";
                        m_M = m_segment_base + reg<asv>(m_rm) + fetch<int32_t>(i);
                        break;
		    }
                } 
            }
	    LOG(INFO) << "m_M = " << std::hex << m_M;
	    LOG(INFO) << "rsp = " << std::hex << rsp();
        } else { 
	    LOG(INFO) << "reg operands";
	}
        LOG(INFO) << "mod r/m + sib parsed";
    }


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
	LOG(INFO) << "MOV Eb,Gb";
        E<b>(G<b>());
        return i;
    case 0x89: // MOV Ev,Gv
	LOG(INFO) << "MOV Ev,Gv";
        E<v>(G<v>()); 
        return i;
    case 0x8a: // MOV Gb,Eb
	LOG(INFO) << "MOV Gb,Eb";
        G<b>(E<b>()); 
        return i;
    case 0x8b: // MOV Gv,Ev
	LOG(INFO) << "MOV Gv,Ev ";
        G<v>(E<v>()); 
        return i;
    case 0x8c: 
	die();
        // if (m_mod == 3) R<v>(0); // MOV Rv,Sw
        // else M<w>() = 0;         // MOV Mw,Sw
	// return i;
    case 0x8d: // LEA Gv,M
	G<v>(m_M - m_segment_base);
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
	die(); // asm("cli"); rflags() &= ~if_mask; return i;
    case 0xfb: // STI
	die(); // asm("sti"); rflags() |= if_mask; return i;
    case 0xfc: // CLD
	rflags() &= ~df_mask; return i;
    case 0xfd: // STD
	rflags() |= df_mask; return i;
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
