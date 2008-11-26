#include <jit++/common.h>
#include <jit++/interpreting/base.h>
#include <jit++/interpreting/group_1.h>
#include <jit++/interpreting/group_2.h>
#include <jit++/interpreting/group_3.h>
#include <jit++/interpreting/group_4.h>
#include <jit++/interpreting/group_5.h>
#include <jit++/interpreting/group_6.h>
#include <jit++/interpreting/group_7.h>
#include <jit++/interpreting/misc.h>

namespace jitpp { 
  namespace interpreting { 
 
class opcode_interpreter
 : public virtual group_1, 
   public virtual group_2,
   public virtual group_3,
   public virtual group_4, 
   public virtual group_5,
   public virtual group_6,
   public virtual group_7,
   public virtual misc {

public:
    template <typename os> 
    void interpret_opcode() { 
        typedef int8_t b;
        typedef int16_t w;
        typedef int32_t d;
        typedef typename os::v v;
        typedef typename os::z z;
    
        switch (code) { 
        case 0x00: E<b>(add(E<b>(),G<b>())); return;    // ADD Eb, Gb
        case 0x01: E<v>(add(E<v>(),G<v>())); return;    // ADD Ev, Gv
        case 0x02: G<b>(add(G<b>(),E<b>())); return;    // ADD Gb, Eb
        case 0x03: G<v>(add(G<v>(),E<v>())); return;    // ADD Gv, Ev
        case 0x04: set_reg<b>(0,add(get_reg<b>(0),imm)); return; // ADD AL, Ib
        case 0x05: set_reg<v>(0,add(get_reg<v>(0),imm)); return; // ADD rAX, Iz
        case 0x06: illegal(); // PUSH ES
        case 0x07: illegal(); // POP ES
        case 0x08: E<b>(or_(E<b>(),G<b>())); return;    // OR Eb, Gb
        case 0x09: E<v>(or_(E<v>(),G<v>())); return;    // OR Ev, Gv
        case 0x0a: G<b>(or_(G<b>(),E<b>())); return;    // OR Gb, Eb
        case 0x0b: G<v>(or_(G<v>(),E<v>())); return;    // OR Gv, Ev
        case 0x0c: set_reg<b>(0,or_(get_reg<b>(0),imm)); return; // OR AL, Ib
        case 0x0d: set_reg<v>(0,or_(get_reg<v>(0),imm)); return; // OR rAX, Iz
        case 0x0e: illegal(); // PUSH CS
        case 0x0f: unsupported(); // 3DNow! 0F 0F prefixes not supported
        case 0x10: E<b>(adc(E<b>(),G<b>())); return;    // ADC Eb, Gb
        case 0x11: E<v>(adc(E<v>(),G<v>())); return;    // ADC Ev, Gv
        case 0x12: G<b>(adc(G<b>(),E<b>())); return;    // ADC Gb, Eb
        case 0x13: G<v>(adc(G<v>(),E<v>())); return;    // ADC Gv, Ev
        case 0x14: set_reg<b>(0,adc(get_reg<b>(0),imm)); return; // ADC AL, Ib
        case 0x15: set_reg<v>(0,adc(get_reg<v>(0),imm)); return; // ADC rAX, Iz
        case 0x16: illegal(); // PUSH SS
        case 0x17: illegal(); // POP SS
        case 0x18: E<b>(sbb<b>(E<b>(),G<b>())); return;    // SBB Eb, Gb
        case 0x19: E<v>(sbb<v>(E<v>(),G<v>())); return;    // SBB Ev, Gv
        case 0x1a: G<b>(sbb<b>(G<b>(),E<b>())); return;    // SBB Gb, Eb
        case 0x1b: G<v>(sbb<v>(G<v>(),E<v>())); return;    // SBB Gv, Ev
        case 0x1c: set_reg<b>(0,sbb<b>(get_reg<b>(0),imm)); return; // SBB AL, Ib
        case 0x1d: set_reg<v>(0,sbb<v>(get_reg<v>(0),imm)); return; // SBB rAX, Iz
        case 0x1e: illegal(); // PUSH DS
        case 0x1f: illegal(); // POP DS
        case 0x20: E<b>(and_(E<b>(),G<b>())); return;    // AND Eb, Gb
        case 0x21: E<v>(and_(E<v>(),G<v>())); return;    // AND Ev, Gv
        case 0x22: G<b>(and_(G<b>(),E<b>())); return;    // AND Gb, Eb
        case 0x23: G<v>(and_(G<v>(),E<v>())); return;    // AND Gv, Ev
        case 0x24: set_reg<b>(0,and_(get_reg<b>(0),imm)); return; // AND AL, Ib
        case 0x25: set_reg<v>(0,and_(get_reg<v>(0),imm)); return; // AND rAX, Iz
        case 0x27: illegal(); // DAA
        case 0x26: logic_error(); // ES: prefix
        case 0x28: E<b>(sub(E<b>(),G<b>())); return;    // SUB Eb, Gb
        case 0x29: E<v>(sub(E<v>(),G<v>())); return;    // SUB Ev, Gv
        case 0x2a: G<b>(sub(G<b>(),E<b>())); return;    // SUB Gb, Eb
        case 0x2b: G<v>(sub(G<v>(),E<v>())); return;    // SUB Gv, Ev
        case 0x2c: set_reg<b>(0,sub(get_reg<b>(0),imm)); return; // SUB AL, Ib
        case 0x2d: set_reg<v>(0,sub(get_reg<v>(0),imm)); return; // SUB rAX, Iz
        case 0x2e: logic_error(); // CS: prefix
        case 0x2f: illegal(); // DAS
        case 0x30: E<b>(xor_(E<b>(),G<b>())); return;    // XOR Eb, Gb
        case 0x31: E<v>(xor_(E<v>(),G<v>())); return;    // XOR Ev, Gv
        case 0x32: G<b>(xor_(G<b>(),E<b>())); return;    // XOR Gb, Eb
        case 0x33: G<v>(xor_(G<v>(),E<v>())); return;    // XOR Gv, Ev
        case 0x34: set_reg<b>(0,xor_(get_reg<b>(0),imm)); return; // XOR AL, Ib
        case 0x35: set_reg<v>(0,xor_(get_reg<v>(0),imm)); return; // XOR rAX, Iz
        case 0x36: logic_error(); // SS: prefix
        case 0x37: illegal(); // AAA
        case 0x38: sub(E<b>(),G<b>()); return;    // CMP Eb, Gb
        case 0x39: sub(E<v>(),G<v>()); return;    // CMP Ev, Gv
        case 0x3a: sub(G<b>(),E<b>()); return;    // CMP Gb, Eb
        case 0x3b: sub(G<v>(),E<v>()); return;    // CMP Gv, Ev
        case 0x3c: sub(get_reg<b>(0),imm); return; // CMP AL, Ib
        case 0x3d: sub(get_reg<v>(0),imm); return; // CMP rAX, Iz
        case 0x3e: logic_error(); // DS: prefix
        case 0x3f: illegal(); // AAS
        case 0x40 ... 0x4f: logic_error(); // REX prefix
        case 0x50 ... 0x57: push<v>(get_reg<v>(rex_b(code & 7))); return; // PUSH rXX
        case 0x58 ... 0x5f: set_reg<v>(rex_b(code & 7),pop<v>()); return; // POP rXX
        case 0x60: illegal(); // PUSHA
        case 0x61: illegal(); // POPA
        case 0x62: illegal(); // BOUND Gv,Ma
        case 0x63: G<v>(E<d>()); return; // MOVSXD Gv, Ed // formerly ARPL Ew, Gw
        case 0x64: logic_error(); // FS: prefix
        case 0x65: logic_error(); // GS: prefix
        case 0x66: logic_error(); // OS prefix
        case 0x67: logic_error(); // AS prefix
        case 0x68: push<v>(imm); return; // PUSH Iz
        case 0x69: unsupported(); // TODO: IMUL Gv,Ev,Iz
        case 0x6a: push<v>(imm); return; // PUSH Ib (check stack slot size?)
        case 0x6b: unsupported(); // TODO: IMUL Gv,Ev,Ib
        case 0x6c: unsupported(); // TODO: INS Yb, DX
        case 0x6d: unsupported(); // TODO: INS Yz, DX
        case 0x6e: unsupported(); // TODO: OUTS DX, Xb
        case 0x6f: unsupported(); // TODO: OUTS DX, Xz
        case 0x70 ... 0x7f: // JCC Ib
            if (test_cc(code & 0xf)) rip() += imm;
            return;

        case 0x80: interpret_group_1<b>(imm); return; // group #1 Eb, Ib
        case 0x81: interpret_group_1<v>(imm); return; // group #1 Ev, Iz
        case 0x82: illegal(); // group #1* Eb, Ib
        case 0x83: interpret_group_1<v>(imm); return; // group #1 Ev, Ib
        case 0x84: and_(E<b>(),G<b>()); return; // TEST Eb, Gb
        case 0x85: and_(E<v>(),G<v>()); return; // TEST Ev, Gv
	case 0x86: xchg<b>(); return; // XCHG Eb,Gb
	case 0x87: xchg<v>(); return; // XCHG Ev,Gv
        case 0x88: E<b>(G<b>()); return; // MOV Eb,Gb
        case 0x89: E<v>(G<v>()); return; // MOV Ev,Gv
        case 0x8a: G<b>(E<b>()); return; // MOV Gb,Eb
        case 0x8b: G<v>(E<v>()); return; // MOV Gv,Ev
        case 0x8c: unsupported(); // MOV Rv, Sw or MOV Mw,Sw
        case 0x8d: G<v>(mem(false)); return; // LEA Gv, M
        case 0x8e: unsupported(); // MOV Sw, Mw or MOV Sw,Rv
        case 0x8f: // POP Ev (8F /0) (group 10)
            if (reg != 0) illegal();
            E<v>(pop<v>());
            return;
        case 0x90: asm("pause"); return; // PAUSE (NOP 90)
	case 0x91 ... 0x97: // XCHG rXX, rAX
	    xchg<v>(rex_b(code & 7)); return;
	case 0x98: // CBW, CWDE,CDQE (sign extend one size, staying in reg 1)
	    set_reg<v>(0,get_reg<typename os::smaller_size>(0));
	    return;
	case 0x99: // CWD, CDQ, CQO (copy sign bit in rAX to all bits of rDX)
	    set_reg<v>(2,get_reg<v>(0) < 0 ? -1 : 0);
	    return;
        case 0x9a: illegal(); // CALL Ap
	case 0x9b: unsupported(); // WAIT, FWAIT
	case 0x9c: push<v>(rflags()); return; // PUSHF
	case 0x9d: rflags() = pop<v>(); return; // POPF
        case 0x9e: // SAHF if CPUID.AHF = 1
            rflags() = (rflags() & ~0xff) | ah(); 
            return;
        case 0x9f: // LAHF if CPUID.AHF = 1
            ah() = static_cast<uint8_t>(rflags() & 0xff);
            return;
	case 0xa4: movs<b>(); return; // REP? MOVSB
	case 0xa5: movs<v>(); return; // REP? MOVS[WDQ]
	case 0xa8: and_(al(),imm); return; // TEST AL, Ib
	case 0xa9: and_(get_reg<v>(0),imm); return; // TEST rAX, Iz
	case 0xb0 ... 0xb7: // MOV RXXB, Ib
	    set_reg<b>(rex_b(code & 7),imm);
            return; 
        case 0xb8 ... 0xbf: // MOV RXX, Iq
            set_reg<v>(rex_b(code & 7),imm);
            return;
        case 0xc0: interpret_group_2<b>(imm); return; // group 2 Eb, Ib
        case 0xc1: interpret_group_2<v>(imm); return; // group 2 Ev, Ib
	case 0xc2: rip() = pop<v>(); rsp() += sizeof(v)*imm; return; // RET (Near) Iw
        case 0xc3: rip() = pop<v>(); return; // RET (Near)
        case 0xc4: illegal(); // LES Gz,Mp
        case 0xc5: illegal(); // LDS Gz,Mp
        case 0xc6: // MOV Eb, Ib (group #12)
            if (reg != 0) illegal();
            E<b>(imm);
            return;
        case 0xc7: // MOV Ev, Iz (group #12)
            if (reg != 0) illegal();
            E<v>(imm);
            return;
	case 0xc9: // LEAVE
	    rsp() = rbp();
	    rbp() = pop<v>();
            return;
	case 0xcc: unsupported(); // INT3
        case 0xce: illegal(); // INTO
	case 0xd0: interpret_group_2<b>(1); return; // group 2 Eb, 1
	case 0xd1: interpret_group_2<v>(1); return; // group 2 Ev, 1
	case 0xd2: interpret_group_2<b>(cl()); return; // group 2 Eb, CL
	case 0xd3: interpret_group_2<v>(cl()); return; // group 2 Ev, CL
        case 0xd4: illegal(); // AAM Ib
        case 0xd5: illegal(); // AAD Ib
        case 0xd6: illegal(); // SALC
        case 0xe8: // CALL Jz
            push<v>(rip());
            rip() += imm;
            return;
        case 0xe9: rip() += imm; return; // JMP Jz
        case 0xea: illegal(); // JMP Ap
        case 0xeb: rip() += imm; return; // JMP Jb
        case 0xf4: uninterpretable(); // HLT
        case 0xf6: interpret_group_3<b>(); return; // group 3 Eb
        case 0xf7: interpret_group_3<v>(); return; // group 3 Ev
        case 0xf8: cf(false); return; // CLC
        case 0xf9: cf(true); return; // STC
        case 0xfa: uninterpretable(); // CLI
        case 0xfb: uninterpretable(); // STI
        case 0xfc: df(false); return; // CLD
        case 0xfd: df(true); return; // STD
        case 0xfe: interpret_group_4<b>(); return; // group 4 Eb
        case 0xff: interpret_group_5<v>(); return; // group 5 Ev
	case 0x101: interpret_group_7<os>(); return; // group 7
	case 0x102: G<v>(lar(E<w>())); return; // LAR Gv, Ew
	case 0x103: G<v>(lsl(E<w>())); return; // LSL Gv, Ew
	case 0x110 ... 0x113: uninterpretable(); // UMOV 
	case 0x105: syscall_(); return; // SYSCALL (wrapped above)
	case 0x106: uninterpretable(); // CLTS 
	case 0x107: uninterpretable(); // SYSRET 
	case 0x108: invd(); return;
	case 0x109: wbinvd(); return;
        case 0x10b: illegal(); // UD2
        case 0x118 ... 0x11f: return; // PREFETCH M, NOP Ev and HINT NOP Ev
        case 0x120: unsupported(); // MOV Rd,Cd
        case 0x121: unsupported(); // MOV Rd,Dd
        case 0x122: unsupported(); // MOV Cd,Rd
        case 0x123: unsupported(); // MOV Dd,Rd
	case 0x130: wrmsr(); return;
	case 0x131: rdtsc(); return;
	case 0x132: rdmsr(); return;
	case 0x133: rdpmc(); return;
        case 0x134: unsupported(); // SYSENTER (illegal on AMD64, legal on EMT64?, unrecognized by udis86)
        case 0x135: unsupported(); // SYSEXIT (illegal on AMD64, legal on EMT64)
        case 0x140 ... 0x14f: // CMOVcc Gv,Ev
            if (test_cc(code & 0xf)) G<v>(E<v>());
            return;
        case 0x180 ... 0x18f: // Jcc Jz
            if (test_cc(code & 0xf)) rip() += imm;
            return;
        case 0x190 ... 0x19f: // SETcc Eb
            E<b>(test_cc(code & 0xf));
            return;
        case 0x1a0: unsupported(); // push<v>(i,fs()); return i; // PUSH FS 
        case 0x1a8: unsupported(); // push<v>(i,gs()); return i; // PUSH GS
        case 0x1a1: unsupported(); // fs(pop<v>(i)); return i;  // POP FS
        case 0x1a9: unsupported(); // gs(pop<v>(i)); return i;  // POP GS
	case 0x1ac: E<v>(shrd<v>(E<v>(), G<v>(), imm)); return;  // SHRD Ev,Gv,Ib
	case 0x1ad: E<v>(shrd<v>(E<v>(), G<v>(), cl())); return; // SHRD Ev,Gv,CL
        case 0x1b0: cmpxchg<b>(); return; // CMPXCHG Eb,Gb
        case 0x1b1: cmpxchg<v>(); return; // CMPXCHG Ev,Gv
        case 0x1b6: G<v>(static_cast<uint64_t>(static_cast<uint8_t>(E<b>()))); return; // MOVZX Gv, Eb
        case 0x1b7: G<v>(static_cast<uint64_t>(static_cast<uint8_t>(E<w>()))); return; // MOVZX Gv, Ew
        case 0x1b9: illegal(); // UD1
        case 0x1ff: illegal(); // UD0
        default: break;
        } 
        unsupported();
    } // interpreter_impl::interpret_opcode<T>
}; // class opcode_interpreter
    
} // namespace interpreting
} // namespace jitpp
