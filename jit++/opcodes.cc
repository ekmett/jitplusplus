#include <jit++/common.h>
#include <jit++/interpreter_internal.h>
#include <jit++/group_1.h>
#include <jit++/group_2.h>
#include <jit++/group_3.h>
#include <jit++/group_4.h>


namespace jitpp { 
    using namespace jitpp::flags;

    template <typename T> static inline void cmpxchg(interpreter & i) {
        int8_t value = E<T>(i);
        if (i.zf(value == get_reg<T>(i,0))) E<T>(i,G<T>(i));
        else set_reg<T>(i,0,value);
	return;
    }

    template <typename T> static inline void xchg(interpreter & i) { 
        T t = G<T>(i); 
	G<T>(i,E<T>(i)); 
	E<T>(i, t);
    }

    template <typename T> static inline void xchg(interpreter & i, int r) { 
	T t = get_reg<T>(i,r);
	set_reg<T>(i,r,get_reg<T>(i,0));
	set_reg<T>(i,0,t);
	return;
    }

    int repetitions(const interpreter & i) { 
	if (!i.has_repxx_prefix()) return 1;
	else if (i.address_size_is_64()) return i.ecx();
	else return i.rcx();
    }

    template <typename T> static inline void movs(interpreter & i) { 
	int64_t base = i.seg_base();
	T * s = reinterpret_cast<T*>(base + i.rsi());
	T * d = reinterpret_cast<T*>(i.rdi());
	int count = repetitions(i);
	VLOG(1) 
   	    << "movs:"
	    << " copying " << count << " " << sizeof(T) << " byte chunks from " 
	    << std::hex << (base + i.rsi()) << " to " << i.rdi() 
	    << (i.df() ? " descending" : "");
	if (i.df()) 
	    while (count-- != 0)
		*s-- = *d--;
	else 
	    while (count-- != 0)
	        *s++ = *d++;
	// initialize flags here
	i.rsi() = reinterpret_cast<int64_t>(s) - base;
	i.rdi() = reinterpret_cast<int64_t>(d);
	return;
    }

    void syscall_(interpreter & i) { 
	int64_t rax = i.rax();
	VLOG(1) << "executing syscall " << std::hex << rax;
	if (rax == 0xe7) uninterpretable(); // check out and let the other process clean us up on exit
	int64_t rbx = i.rbx();
	int64_t rdx = i.rdx();
	int64_t rsi = i.rsi();
	int64_t rdi = i.rdi();
	register int64_t r8 asm("r8") = i.r8();
	register int64_t r9 asm("r9") = i.r9(); // how to specify these constraints directly?
	register int64_t r11 asm("r11");

	// should be valid for linux x86-64 abi
	__asm__ __volatile__ (
	    "syscall"
	    : "=a"(rax), "=b"(rbx), "=r"(r11)
	    : "a"(rax), "b"(rbx), "d"(rdx), "r"(r9), "R"(rsi), "D"(rdi), "r"(r8)
	    : "memory", "cc", "cx"
	);

	i.rax() = rax;
	i.rbx() = rbx;
	i.r11() = r11;
    }

    template <typename os> static void interpret_opcode(interpreter & i) { 
        typedef int8_t b;
        typedef int16_t w;
        typedef int32_t d;
        typedef typename os::v v;
        typedef typename os::z z;
    
        switch (i.code) { 
        case 0x00: E<b>(i,ADD(b,E<b>(i),G<b>(i))); return;    // ADD Eb, Gb
        case 0x01: E<v>(i,ADD(b,E<v>(i),G<v>(i))); return;    // ADD Ev, Gv
        case 0x02: G<b>(i,ADD(b,G<b>(i),E<b>(i))); return;    // ADD Gb, Eb
        case 0x03: G<v>(i,ADD(v,G<v>(i),E<v>(i))); return;    // ADD Gv, Ev
        case 0x04: set_reg<b>(i,0,ADD(b,get_reg<b>(i,0),i.imm)); return; // ADD AL, Ib
        case 0x05: set_reg<v>(i,0,ADD(v,get_reg<v>(i,0),i.imm)); return; // ADD rAX, Iz
        case 0x06: illegal(); // PUSH ES
        case 0x07: illegal(); // POP ES

        case 0x08: E<b>(i,OR(b,E<b>(i),G<b>(i))); return;    // OR Eb, Gb
        case 0x09: E<v>(i,OR(v,E<v>(i),G<v>(i))); return;    // OR Ev, Gv
        case 0x0a: G<b>(i,OR(b,G<b>(i),E<b>(i))); return;    // OR Gb, Eb
        case 0x0b: G<v>(i,OR(v,G<v>(i),E<v>(i))); return;    // OR Gv, Ev
        case 0x0c: set_reg<b>(i,0,OR(b,get_reg<b>(i,0),i.imm)); return; // OR AL, Ib
        case 0x0d: set_reg<v>(i,0,OR(v,get_reg<v>(i,0),i.imm)); return; // OR rAX, Iz
        case 0x0e: illegal(); // PUSH CS
        case 0x0f: unsupported(); // 3DNow! 0F 0F prefixes not supported

        case 0x10: E<b>(i,ADC(b,E<b>(i),G<b>(i))); return;    // ADC Eb, Gb
        case 0x11: E<v>(i,ADC(v,E<v>(i),G<v>(i))); return;    // ADC Ev, Gv
        case 0x12: G<b>(i,ADC(b,G<b>(i),E<b>(i))); return;    // ADC Gb, Eb
        case 0x13: G<v>(i,ADC(v,G<v>(i),E<v>(i))); return;    // ADC Gv, Ev
        case 0x14: set_reg<b>(i,0,ADC(b,get_reg<b>(i,0),i.imm)); return; // ADC AL, Ib
        case 0x15: set_reg<v>(i,0,ADC(v,get_reg<v>(i,0),i.imm)); return; // ADC rAX, Iz
        case 0x16: illegal(); // PUSH SS
        case 0x17: illegal(); // POP SS

        case 0x18: E<b>(i,SBB(b,E<b>(i),G<b>(i))); return;    // SBB Eb, Gb
        case 0x19: E<v>(i,SBB(v,E<v>(i),G<v>(i))); return;    // SBB Ev, Gv
        case 0x1a: G<b>(i,SBB(b,G<b>(i),E<b>(i))); return;    // SBB Gb, Eb
        case 0x1b: G<v>(i,SBB(v,G<v>(i),E<v>(i))); return;    // SBB Gv, Ev
        case 0x1c: set_reg<b>(i,0,SBB(b,get_reg<b>(i,0),i.imm)); return; // SBB AL, Ib
        case 0x1d: set_reg<v>(i,0,SBB(v,get_reg<v>(i,0),i.imm)); return; // SBB rAX, Iz
        case 0x1e: illegal(); // PUSH DS
        case 0x1f: illegal(); // POP DS

        case 0x20: E<b>(i,AND(b,E<b>(i),G<b>(i))); return;    // AND Eb, Gb
        case 0x21: E<v>(i,AND(v,E<v>(i),G<v>(i))); return;    // AND Ev, Gv
        case 0x22: G<b>(i,AND(b,G<b>(i),E<b>(i))); return;    // AND Gb, Eb
        case 0x23: G<v>(i,AND(v,G<v>(i),E<v>(i))); return;    // AND Gv, Ev
        case 0x24: set_reg<b>(i,0,AND(b,get_reg<b>(i,0),i.imm)); return; // AND AL, Ib
        case 0x25: set_reg<v>(i,0,AND(v,get_reg<v>(i,0),i.imm)); return; // AND rAX, Iz
        case 0x27: illegal(); // DAA
        case 0x26: logic_error(); // ES: prefix

        case 0x28: E<b>(i,SUB(b,E<b>(i),G<b>(i))); return;    // SUB Eb, Gb
        case 0x29: E<v>(i,SUB(v,E<v>(i),G<v>(i))); return;    // SUB Ev, Gv
        case 0x2a: G<b>(i,SUB(b,G<b>(i),E<b>(i))); return;    // SUB Gb, Eb
        case 0x2b: G<v>(i,SUB(v,G<v>(i),E<v>(i))); return;    // SUB Gv, Ev
        case 0x2c: set_reg<b>(i,0,SUB(b,get_reg<b>(i,0),i.imm)); return; // SUB AL, Ib
        case 0x2d: set_reg<v>(i,0,SUB(v,get_reg<v>(i,0),i.imm)); return; // SUB rAX, Iz
        case 0x2e: logic_error(); // CS: prefix
        case 0x2f: illegal(); // DAS

        case 0x30: E<b>(i,XOR(b,E<b>(i),G<b>(i))); return;    // XOR Eb, Gb
        case 0x31: E<v>(i,XOR(v,E<v>(i),G<v>(i))); return;    // XOR Ev, Gv
        case 0x32: G<b>(i,XOR(b,G<b>(i),E<b>(i))); return;    // XOR Gb, Eb
        case 0x33: G<v>(i,XOR(v,G<v>(i),E<v>(i))); return;    // XOR Gv, Ev
        case 0x34: set_reg<b>(i,0,XOR(b,get_reg<b>(i,0),i.imm)); return; // XOR AL, Ib
        case 0x35: set_reg<v>(i,0,XOR(v,get_reg<v>(i,0),i.imm)); return; // XOR rAX, Iz
        case 0x36: logic_error(); // SS
        case 0x37: illegal(); // AAA

        case 0x38: SUB(b,E<b>(i),G<b>(i)); return;    // CMP Eb, Gb
        case 0x39: SUB(v,E<v>(i),G<v>(i)); return;    // CMP Ev, Gv
        case 0x3a: SUB(b,G<b>(i),E<b>(i)); return;    // CMP Gb, Eb
        case 0x3b: SUB(v,G<v>(i),E<v>(i)); return;    // CMP Gv, Ev
        case 0x3c: SUB(b,get_reg<b>(i,0),i.imm); return; // CMP AL, Ib
        case 0x3d: SUB(v,get_reg<v>(i,0),i.imm); return; // CMP rAX, Iz
        case 0x3e: logic_error(); // DS: prefix
        case 0x3f: illegal(); // AAS

        case 0x40: case 0x41: case 0x42: case 0x43: 
        case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4a: case 0x4b: 
        case 0x4c: case 0x4d: case 0x4e: case 0x4f: // REX
            logic_error();

        case 0x50: case 0x51: case 0x52: case 0x53: 
        case 0x54: case 0x55: case 0x56: case 0x57: // PUSH rXX
            push<v>(i,get_reg<v>(i,i.rex_b(i.code & 7))); return;

        case 0x58: case 0x59: case 0x5a: case 0x5b: 
        case 0x5c: case 0x5d: case 0x5e: case 0x5f: // POP rXX
            set_reg<v>(i,i.rex_b(i.code & 7),pop<v>(i)); return;

        case 0x60: illegal(); // PUSHA
        case 0x61: illegal(); // POPA
        case 0x62: illegal(); // BOUND Gv,Ma
        case 0x63: G<v>(i,E<d>(i)); return; // MOVSXD Gv, Ed // formerly ARPL Ew, Gw
        case 0x64: logic_error(); // FS: prefix
        case 0x65: logic_error(); // GS: prefix
        case 0x66: logic_error(); // OS prefix
        case 0x67: logic_error(); // AS prefix
        case 0x68: push<v>(i,i.imm); return; // PUSH Iz
        case 0x69: unsupported(); // TODO: IMUL Gv,Ev,Iz
        case 0x6a: push<v>(i,i.imm); return; // PUSH Ib (check stack slot size?)
        case 0x6b: unsupported(); // TODO: IMUL Gv,Ev,Ib
        case 0x6c: unsupported(); // TODO: INS Yb, DX
        case 0x6d: unsupported(); // TODO: INS Yz, DX
        case 0x6e: unsupported(); // TODO: OUTS DX, Xb
        case 0x6f: unsupported(); // TODO: OUTS DX, Xz

        case 0x70: case 0x71: case 0x72: case 0x73: 
        case 0x74: case 0x75: case 0x76: case 0x77: 
        case 0x78: case 0x79: case 0x7a: case 0x7b: 
        case 0x7c: case 0x7d: case 0x7e: case 0x7f: // JCC Ib
            if (test_cc(i,i.code & 0xf)) 
                i.rip() += i.imm;
            return;

        case 0x80: group_1<b>::interpret(i,i.imm); return; // group #1 Eb, Ib
        case 0x81: group_1<v>::interpret(i,i.imm); return; // group #1 Ev, Iz
        case 0x82: illegal(); // group #1* Eb, Ib
        case 0x83: group_1<v>::interpret(i,i.imm); return; // group #1 Ev, Ib
        case 0x84: AND(b,E<b>(i),G<b>(i)); return; // TEST Eb, Gb
        case 0x85: AND(v,E<v>(i),G<v>(i)); return; // TEST Ev, Gv
	case 0x86: xchg<b>(i); return; // XCHG Eb,Gb
	case 0x87: xchg<v>(i); return; // XCHG Ev,Gv
        case 0x88: E<b>(i,G<b>(i)); return; // MOV Eb,Gb
        case 0x89: E<v>(i,G<v>(i)); return; // MOV Ev,Gv
        case 0x8a: G<b>(i,E<b>(i)); return; // MOV Gb,Eb
        case 0x8b: G<v>(i,E<v>(i)); return; // MOV Gv,Ev
        case 0x8c: unsupported(); // MOV Rv, Sw or MOV Mw,Sw
        case 0x8d: G<v>(i,i.mem(false)); return; // LEA Gv, M
        case 0x8e: unsupported(); // MOV Sw, Mw or MOV Sw,Rv
        case 0x8f: // POP Ev (8F /0) (group 10)
            if (i.reg != 0) illegal();
            E<v>(i,pop<v>(i));
            return;

        case 0x90: asm("pause"); return; // PAUSE (NOP 90)
	/* 0x90 */ case 0x91: case 0x92: case 0x93: 
	case 0x94: case 0x95: case 0x96: case 0x97: // XCHG rXX, rAX
	    xchg<v>(i,i.rex_b(i.code & 7)); return;
	case 0x98: // CBW, CWDE,CDQE (sign extend one size, staying in reg 1)
	    set_reg<v>(i,0,get_reg<typename os::smaller_size>(i,0));
	    return;
	case 0x99: // CWD, CDQ, CQO (copy sign bit in rAX to all bits of rDX)
	    set_reg<v>(i,2,get_reg<v>(i,0) < 0 ? -1 : 0);
	    return;
        case 0x9a: illegal(); // CALL Ap
	case 0x9b: unsupported(); // WAIT, FWAIT
	case 0x9c: push<v>(i,i.rflags()); return; // PUSHF
	case 0x9d: i.rflags() = pop<v>(i); return; // POPF
        case 0x9e: // SAHF if CPUID.AHF = 1
            i.rflags() = (i.rflags() & ~0xff) | i.ah(); 
            return;
        case 0x9f: // LAHF if CPUID.AHF = 1
            i.ah() = static_cast<uint8_t>(i.rflags() & 0xff);
            return;

	case 0xa4: movs<b>(i); return; // REP? MOVSB
	case 0xa5: movs<v>(i); return; // REP? MOVS[WDQ]
	case 0xa8: AND(b,i.al(),i.imm); return; // TEST AL, Ib
	case 0xa9: AND(v,get_reg<v>(i,0),i.imm); return; // TEST rAX, Iz
	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
	case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	    set_reg<b>(i,i.rex_b(i.code & 7),i.imm);
        case 0xb8: case 0xb9: case 0xba: case 0xbb: 
	case 0xbc: case 0xbd: case 0xbe: case 0xbf: 
            // MOV RXX, Iq
            set_reg<v>(i,i.rex_b(i.code & 7),i.imm);
            return;
        case 0xc0: group_2<b>::interpret(i,i.imm); return; // group 2 Eb, Ib
        case 0xc1: group_2<v>::interpret(i,i.imm); return; // group 2 Ev, Ib
	case 0xc2: i.rip() = pop<v>(i); i.rsp() += sizeof(v)*i.imm; return; // RET (Near) Iw
        case 0xc3: i.rip() = pop<v>(i); return; // RET (Near)
        case 0xc4: illegal(); // LES Gz,Mp
        case 0xc5: illegal(); // LDS Gz,Mp
        case 0xc6: // MOV Eb, Ib (group #12)
            if (i.reg != 0) illegal();
            E<b>(i,i.imm);
            return;
        case 0xc7: // MOV Ev, Iz (group #12)
            if (i.reg != 0) illegal();
            E<v>(i,i.imm);
            return;
	case 0xc9: // LEAVE
	    i.rsp() = i.rbp();
	    i.rbp() = pop<v>(i);
            return;
        case 0xce: illegal(); // INTO
	case 0xd0: group_2<b>::interpret(i,1); return; // group 2 Eb, 1
	case 0xd1: group_2<v>::interpret(i,1); return; // group 2 Ev, 1
	case 0xd2: group_2<b>::interpret(i,i.cl()); return; // group 2 Eb, CL
	case 0xd3: group_2<v>::interpret(i,i.cl()); return; // group 2 Ev, CL
        case 0xd4: illegal(); // AAM Ib
        case 0xd5: illegal(); // AAD Ib
        case 0xd6: illegal(); // SALC
        case 0xe8: // CALL Jz
            push<v>(i,i.rip());
            i.rip() += i.imm;
            return;
        case 0xe9: i.rip() += i.imm; return; // JMP Jz
        case 0xea: illegal(); // JMP Ap
        case 0xeb: i.rip() += i.imm; return; // JMP Jb
        case 0xf4: uninterpretable(); // HLT
        case 0xf6: group_3<b>::interpret(i); return; // group 3 Eb
        case 0xf7: group_3<v>::interpret(i); return; // group 3 Eb
        case 0xf8: i.cf(false); return; // CLC
        case 0xf9: i.cf(true); return; // STC
        case 0xfa: uninterpretable(); // CLI
        case 0xfb: uninterpretable(); // STI
        case 0xfc: i.df(false); return; // CLD
        case 0xfd: i.df(true); return; // STD
        case 0xfe: group_4<b>::interpret(i); return; // group 4 Eb
        case 0xff: // opcode group 5
            switch (i.reg) { 
            case 0: E<v>(i,group_4<v>::inc(i,E<v>(i))); return; // INC Ev
            case 1: E<v>(i,group_4<v>::dec(i,E<v>(i))); return; // DEC Ev
            case 2: push<v>(i,i.rip()); i.rip() = E<v>(i); return; // CALL Ev
            case 3: unsupported();
            case 4: i.rip() = E<v>(i); return; // JMP Ev
            case 5: unsupported();
            case 6: push<v>(i,E<v>(i)); return; // PUSH Ev
            case 7: illegal();
            default: unsupported();
            }
	case 0x105: syscall_(i); return; // SYSCALL (wrapped above)
	case 0x107: uninterpretable(); return; // SYSRET 
        case 0x10b: illegal(); // UD2
        case 0x118: case 0x119: case 0x11a: case 0x11b: 
        case 0x11c: case 0x11d: case 0x11e: case 0x11f: 
            return; // PREFETCH M, NOP Ev and HINT NOP Ev
        case 0x120: unsupported(); // MOV Rd,Cd
        case 0x121: unsupported(); // MOV Rd,Dd
        case 0x122: unsupported(); // MOV Cd,Rd
        case 0x123: unsupported(); // MOV Dd,Rd
        case 0x134: unsupported(); // SYSENTER (illegal on AMD64, legal on EMT64?, unrecognized by udis86)
        case 0x135: unsupported(); // SYSEXIT (illegal on AMD64, legal on EMT64)
        case 0x140: case 0x141: case 0x142: case 0x143: 
        case 0x144: case 0x145: case 0x146: case 0x147: 
        case 0x148: case 0x149: case 0x14a: case 0x14b:
        case 0x14c: case 0x14d: case 0x14e: case 0x14f: // CMOVcc Gv,Ev
            if (test_cc(i,i.code & 0xf)) 
                G<v>(i,E<v>(i));
            return;
    
        case 0x180: case 0x181: case 0x182: case 0x183: 
        case 0x184: case 0x185: case 0x186: case 0x187: 
        case 0x188: case 0x189: case 0x18a: case 0x18b:
        case 0x18c: case 0x18d: case 0x18e: case 0x18f: // Jcc Jz
            if (test_cc(i,i.code & 0xf)) 
                i.rip() += i.imm;
            return;
        case 0x190: case 0x191: case 0x192: case 0x193: 
        case 0x194: case 0x195: case 0x196: case 0x197:
        case 0x198: case 0x199: case 0x19a: case 0x19b: 
        case 0x19c: case 0x19d: case 0x19e: case 0x19f: // SETcc Eb
            E<b>(i,test_cc(i,i.code & 0xf));
            return;
        case 0x1a0: unsupported(); // push<v>(i,fs()); return i; // PUSH FS 
        case 0x1a8: unsupported(); // push<v>(i,gs()); return i; // PUSH GS
        case 0x1a1: unsupported(); // fs(pop<v>(i)); return i;  // POP FS
        case 0x1a9: unsupported(); // gs(pop<v>(i)); return i;  // POP GS
	case 0x1ac: E<v>(i, group_2<v>::shrd(i, E<v>(i), G<v>(i), i.imm)); return;  // SHRD Ev,Gv,Ib
	case 0x1ad: E<v>(i, group_2<v>::shrd(i, E<v>(i), G<v>(i), i.cl())); return; // SHRD Ev,Gv,CL
        case 0x1b0: cmpxchg<b>(i); return; // CMPXCHG Eb,Gb
        case 0x1b1: cmpxchg<v>(i); return; // CMPXCHG Ev,Gv
        case 0x1b6: G<v>(i,static_cast<uint64_t>(static_cast<uint8_t>(E<b>(i)))); return; // MOVZX Gv, Eb
        case 0x1b7: G<v>(i,static_cast<uint64_t>(static_cast<uint8_t>(E<w>(i)))); return; // MOVZX Gv, Ew
        case 0x1b9: illegal(); // UD1
        case 0x1ff: illegal(); // UD0
        default: break;
        } 
        unsupported();
    } // interpreter::interpret
    
    void interpret_opcode_16(interpreter & i) {
        return interpret_opcode<os16>(i);
    }
    void interpret_opcode_32(interpreter & i) {
        return interpret_opcode<os32>(i);
    }
    void interpret_opcode_64(interpreter & i) {
        return interpret_opcode<os64>(i);
    }
} // namespace jitpp
