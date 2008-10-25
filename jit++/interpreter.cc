#include <ucontext.h> 
#include <iostream>
#include <iomanip>
#include <jit++/internal.h>
#include <jit++/interpreter.h>
#include <jit++/exception.h>
#include <jit++/opcode.h>
#include <jit++/rflags_internal.h>
#include <asm/prctl.h>        // SYS_arch_prctl
#include <sys/syscall.h>      // syscall
#include <errno.h>
#include <udis86.h>

namespace { 
    const char * os64_reg_names[16] = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
    const char * os32_reg_names[16] = {"eax","ecx","edx","ebx","esp","ebp","esi","edi","r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"};
    const char * os16_reg_names[16] = {"ax","cx","dx","bx","sp","bp","si","di","r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w"};
    const char * byte_reg_names_rex[16] = {"al","cl","dl","bl","spl","bpl","sil","dil","r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"};
    const char * byte_reg_names_norex[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};

    struct os64 { 
        typedef int64_t  v;
        typedef int32_t  z;
        typedef int64_t  qv;
        static const int bits = 64;
        static inline const char * reg_name(int r) { return os64_reg_names[r]; }
    };
    
    struct os32 { 
        typedef int32_t  v;
        typedef int32_t  z;
        typedef int64_t  qv;
        static const int bits = 32;
        static inline const char * reg_name(int r) { return os32_reg_names[r]; }
    };
    
    struct os16 { 
        typedef int16_t  v;
        typedef int16_t  z;
        typedef int16_t  qv;
        static const int bits = 16;
        static inline const char * reg_name(int r) { return os16_reg_names[r]; }
    };
 
    inline const char * byte_reg_name(int r, bool has_rex) { 
        return has_rex ? byte_reg_names_rex[r] : byte_reg_names_norex[r];
    }

}

namespace jitpp { 

    namespace interpreter_result { 
	const char * description[max] = {
	    "ok",
	    "illegal",
	    "won't trace",
	    "unsupported",
	    "logic error"
	};
    }
    int64_t interpreter::memory_operand_address(bool add_segment_base) const {
        int64_t addr = op.disp;
	VLOG(1) << "disp " << std::hex << addr;

	if (add_segment_base) { 
            if (op.seg_prefix == 0x64)
                addr += fs_base();
            else if (op.seg_prefix == 0x65)
                addr += gs_base();
	    VLOG(1) << "addr w/ segment base " << std::hex << addr;
	}

	if (op.address_size_is_64()) { 
	    VLOG(1) << "64 bit address";
            if (op.has_sib()) {
	        VLOG(1) << "with sib";
                if (op.base != 5) { 
                    addr += reg<int64_t>(op.base);
	            VLOG(1) << "addr w/ base " << os64::reg_name(op.base) << " = " << std::hex << addr;
		}
                if (op.index != 4) {
                    addr += reg<int64_t>(op.index) << op.log_scale;
	            VLOG(1) << "addr w/ " << os64::reg_name(op.index) << " * " << op.scale() << " = " << std::hex << addr;
		}
            } else if (op.is_rip_relative()) {
		addr += rip();
	        VLOG(1) << "addr w/ rip (" << std::hex << rip() << ") = " << std::hex << addr;
	    } else addr += reg<int64_t>(op.rm);
	} else { 
	    VLOG(1) << "32 bit address";
	    if (op.has_sib()) { 
                if (op.base != 5) { 
                    addr += reg<int32_t>(op.base);
	            VLOG(1) << "addr w/ base " << os32::reg_name(op.base) << " = " << std::hex << addr;
		}
                if (op.index != 4) {
                    addr += reg<int32_t>(op.index) << op.log_scale;
	            VLOG(1) << "addr w/ " << os32::reg_name(op.index) << " * " << op.scale() << " = " << std::hex << addr;
		}
	    } else if (op.is_rip_relative()) {
		addr += eip();
	        VLOG(1) << "addr w/ eip = " << std::hex << addr;
	    } else addr += reg<int32_t>(op.rm);
	}
	VLOG(1) << "addr = " << std::hex << addr;
        return addr;
    }


// operand sizes to template interpret_operand below

int64_t interpreter::fs_base() const {
    if (unlikely(!m_fs_base_known)) {
        syscall(SYS_arch_prctl,ARCH_GET_FS,&m_fs_base);
        m_fs_base_known = true;
    }
    return m_fs_base;
}

int64_t interpreter::gs_base() const {
    if (unlikely(!m_gs_base_known)) {
        syscall(SYS_arch_prctl,ARCH_GET_FS,&m_gs_base);
        m_gs_base_known = true;
    }
    return m_gs_base;
}



// assumes x86-64 long mode 
// TODO: check for correct handling of 66h on e0-e3,70-7f,eb,e9,ff/4,e8,ff/2,c3,c2
void interpreter::run() { 
    m_fs_base_known = m_gs_base_known = false;
    old_errno = errno; // restore before any interpreted memory read for safety due to TLS conflict, set after any memory write
    ud_t ud;
    ud_init(&ud);
    ud_set_mode(&ud,64);
    ud_set_syntax(&ud,UD_SYN_ATT);
    while (true) { 
	VLOG(2) << "rax " << std::hex << reg<int64_t>(0);
	VLOG(2) << "rcx " << std::hex << reg<int64_t>(1);
	VLOG(2) << "rdx " << std::hex << reg<int64_t>(2);
	VLOG(2) << "rbx " << std::hex << reg<int64_t>(3);
	VLOG(2) << "rsp " << std::hex << reg<int64_t>(4);
	VLOG(2) << "rbp " << std::hex << reg<int64_t>(5);
	VLOG(2) << "rsi " << std::hex << reg<int64_t>(6);
	VLOG(2) << "rdi " << std::hex << reg<int64_t>(7);
	VLOG(2) << "rip " << std::hex << rip();
	VLOG(2) << "rflags " << std::hex << lazy_rflags() << "/" << rflags()  
		<< (cf() ? " CF" : "") 
		<< (sf() ? " SF" : "")
		<< (of() ? " OF" : "")
		<< (pf() ? " PF" : "")
		<< (af() ? " AF" : "")
		<< (zf() ? " ZF" : "");
	
        ud_set_input_buffer(&ud,reinterpret_cast<uint8_t*>(m_rip),15);
        ud_set_pc(&ud,rip());
        size_t bytes = ud_disassemble(&ud);
	LOG(INFO) 
	    << std::noshowbase << std::setfill('0') << std::hex << std::setw(16) << ud_insn_off(&ud) 
            << ": " << std::setfill(' ') << std::setw(32) << ud_insn_hex(&ud)
	    << " " << ud_insn_asm(&ud);


	int64_t old_rip = rip();
	rip() = op.parse(old_rip);

	LOG_IF(WARNING, (bytes != rip() - old_rip)) 
	    << "udis86 and jit++ disagree on opcode size! (udis86: " << bytes << " vs. jit++: " << (rip() - old_rip) << ")";

	interpreter_result::code result;
	if (op.has_lock_prefix()) { 
	    switch (op.log_v) { 
	    case 1: result = interpret_locked_opcode<os16>(); break;
	    case 2: result = interpret_locked_opcode<os32>(); break;
	    case 3: result = interpret_locked_opcode<os64>(); break;
	    default: result = interpreter_result::logic_error; break;
	    }
	} else {
	    switch (op.log_v) {
	    case 1: result = interpret_opcode<os16>(); break;
	    case 2: result = interpret_opcode<os32>(); break;
	    case 3: result = interpret_opcode<os64>(); break;
	    default: result = interpreter_result::logic_error; break;
	    }
 	}

	VLOG(2) << "result: " << interpreter_result::description[result] << " (" << (int)result << ")";
	
	if (result != interpreter_result::ok) {
	    rip() = old_rip; // restore rip to before the parsing attempt
	    rflags();        // force evaluation of any lazy rflags
	    return;	     // stop interpreting
        }
    }
}

    template <typename T> inline void lock_inc(interpreter & i, T * ptr);
    template <> inline void lock_inc<>(interpreter & i, int8_t * ptr) { 
	int8_t o,s,z,a,p;
	__asm__ __volatile__(
	    "lock; incb %0\n\tseto %b1\n\tsets %b2\n\tsetz %b3\n\tseta %b4\n\tsetp %b5\n\t"
	    : "=m"(*ptr), "=qm"(o),"=qm"(s),"=qm"(z),"=qm"(a),"=qm"(p) : "m"(*ptr)
	);
	i.of(o != 0); i.sf(s != 0); i.zf(z != 0); i.af(z != 0); i.pf(p != 0);
    }
    template <> inline void lock_inc<>(interpreter & i, int16_t * ptr) { 
	int8_t o,s,z,a,p;
	__asm__ __volatile__(
	    "lock; incw %0\n\tseto %b1\n\tsets %b2\n\tsetz %b3\n\tseta %b4\n\tsetp %b5\n\t"
	    : "=m"(*ptr), "=qm"(o),"=qm"(s),"=qm"(z),"=qm"(a),"=qm"(p) : "m"(*ptr)
	);
	i.of(o != 0); i.sf(s != 0); i.zf(z != 0); i.af(z != 0); i.pf(p != 0);
    }
    template <> inline void lock_inc<>(interpreter & i, int32_t * ptr) { 
	int8_t o,s,z,a,p;
	__asm__ __volatile__(
	    "lock; incl %0\n\tseto %b1\n\tsets %b2\n\tsetz %b3\n\tseta %b4\n\tsetp %b5\n\t"
	    : "=m"(*ptr), "=qm"(o),"=qm"(s),"=qm"(z),"=qm"(a),"=qm"(p) : "m"(*ptr)
	);
	i.of(o != 0); i.sf(s != 0); i.zf(z != 0); i.af(z != 0); i.pf(p != 0);
    }
    template <> inline void lock_inc<>(interpreter & i, int64_t * ptr) { 
	int8_t o,s,z,a,p;
	__asm__ __volatile__(
	    "lock; incq %0\n\tseto %b1\n\tsets %b2\n\tsetz %b3\n\tseta %b4\n\tsetp %b5\n\t"
	    : "=m"(*ptr), "=qm"(o),"=qm"(s),"=qm"(z),"=qm"(a),"=qm"(p) : "m"(*ptr)
	);
	i.of(o != 0); i.sf(s != 0); i.zf(z != 0); i.af(z != 0); i.pf(p != 0);
    }
 

    template <typename T> inline void lock_dec(interpreter & i, T * ptr);
    template <> inline void lock_dec<>(interpreter & i, int8_t * ptr) { 
	int8_t o,s,z,a,p;
	__asm__ __volatile__(
	    "lock; decb %0\n\tseto %b1\n\tsets %b2\n\tsetz %b3\n\tseta %b4\n\tsetp %b5\n\t"
	    : "=m"(*ptr), "=qm"(o),"=qm"(s),"=qm"(z),"=qm"(a),"=qm"(p) : "m"(*ptr)
	);
	i.of(o != 0); i.sf(s != 0); i.zf(z != 0); i.af(z != 0); i.pf(p != 0);
    }
    template <> inline void lock_dec<>(interpreter & i, int16_t * ptr) { 
	int8_t o,s,z,a,p;
	__asm__ __volatile__(
	    "lock; decw %0\n\tseto %b1\n\tsets %b2\n\tsetz %b3\n\tseta %b4\n\tsetp %b5\n\t"
	    : "=m"(*ptr), "=qm"(o),"=qm"(s),"=qm"(z),"=qm"(a),"=qm"(p) : "m"(*ptr)
	);
	i.of(o != 0); i.sf(s != 0); i.zf(z != 0); i.af(z != 0); i.pf(p != 0);
    }
    template <> inline void lock_dec<>(interpreter & i, int32_t * ptr) { 
	int8_t o,s,z,a,p;
	__asm__ __volatile__(
	    "lock; decl %0\n\tseto %b1\n\tsets %b2\n\tsetz %b3\n\tseta %b4\n\tsetp %b5\n\t"
	    : "=m"(*ptr), "=qm"(o),"=qm"(s),"=qm"(z),"=qm"(a),"=qm"(p) : "m"(*ptr)
	);
	i.of(o != 0); i.sf(s != 0); i.zf(z != 0); i.af(z != 0); i.pf(p != 0);
    }
    template <> inline void lock_dec<>(interpreter & i, int64_t * ptr) { 
	int8_t o,s,z,a,p;
	__asm__ __volatile__(
	    "lock; decq %0\n\tseto %b1\n\tsets %b2\n\tsetz %b3\n\tseta %b4\n\tsetp %b5\n\t"
	    : "=m"(*ptr), "=qm"(o),"=qm"(s),"=qm"(z),"=qm"(a),"=qm"(p) : "m"(*ptr)
	);
	i.of(o != 0); i.sf(s != 0); i.zf(z != 0); i.af(z != 0); i.pf(p != 0);
    }



    template <typename T> inline T lock_cmpxchg(interpreter & i, T * p, T o, T n);

    template <> inline int8_t lock_cmpxchg<>(interpreter & i, int8_t * p, int8_t o, int8_t n) {
        int8_t r;
        __asm__ __volatile__("lock; cmpxchgb %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        i.zf(r == o);
        return r;
    }
    template <> inline int16_t lock_cmpxchg<>(interpreter & i, int16_t * p, int16_t o, int16_t n) {
        int16_t r;
        __asm__ __volatile__("lock; cmpxchgw %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        i.zf(r == o);
        return r;
    }
    template <> inline int32_t lock_cmpxchg<>(interpreter & i, int32_t * p, int32_t o, int32_t n) {
        int32_t r;
        __asm__ __volatile__("lock; cmpxchgl %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        i.zf(r == o);
        return r;
    }
    template <> inline int64_t lock_cmpxchg<>(interpreter & i, int64_t * p, int64_t o, int64_t n) {
        int64_t r;
        __asm__ __volatile__("lock; cmpxchgq %2,%1" : "=a"(r), "=m"(*p) : "q"(n), "m"(*p), "0"(o));
        i.zf(r == o);
        return r;
    }

    template <typename T> inline int64_t op_sbb(interpreter & i, int64_t x, int64_t y) {
        if (i.cf()) return i.set_rflags_context(adc_rflags<T>::instance,x-y-1,x,y);
        else return i.set_rflags_context(add_rflags<T>::instance,x-y,x,y);
    }

    template <typename T> inline int64_t op_adc(interpreter & i, int64_t x, int64_t y) {
        if (i.cf()) return i.set_rflags_context(adc_rflags<T>::instance,x+y+1,x,y);
        else return i.set_rflags_context(add_rflags<T>::instance,x+y,x,y);
    }

    template <typename T> inline int64_t op_add(interpreter & i, int64_t x, int64_t y) {
        return i.set_rflags_context(add_rflags<T>::instance,x+y,x,y);
    }

    template <typename T> inline int64_t op_sub(interpreter & i, int64_t x, int64_t y) {
        return i.set_rflags_context(sub_rflags<T>::instance,x-y,x,y);
    }

    template <typename T> inline int64_t op_and(interpreter & i, int64_t x, int64_t y) {
        return i.set_rflags_context(logic_rflags<T>::instance,x&y);
    }

    template <typename T> inline int64_t op_or(interpreter & i, int64_t x, int64_t y) {
        return i.set_rflags_context(logic_rflags<T>::instance,x|y);
    }

    template <typename T> inline int64_t op_xor(interpreter & i, int64_t x, int64_t y) {
        return i.set_rflags_context(logic_rflags<T>::instance,x^y);
    }

    template <typename T> inline int64_t op_inc(interpreter & i, int64_t x) {
        return i.set_rflags_context(inc_rflags<T>::instance,x+1);
    }

    template <typename T> inline int64_t op_dec(interpreter & i, int64_t x) {
        return i.set_rflags_context(dec_rflags<T>::instance,x-1);
    }

    template <typename T> inline int64_t op_neg(interpreter & i, int64_t x) {
        return i.set_rflags_context(neg_rflags<T>::instance,-x);
    }

#define ADD(T,x,y) op_add<T>(*this,(x),(y))
#define SUB(T,x,y) op_sub<T>(*this,(x),(y))
#define SBB(T,x,y) op_sbb<T>(*this,(x),(y))
#define CMP(T,x,y) op_add<T>(*this,(x),(y))
#define XOR(T,x,y) op_xor<T>(*this,(x),(y))
#define AND(T,x,y) op_and<T>(*this,(x),(y))
#define OR(T,x,y)  op_or<T>(*this,(x),(y))
#define ADC(T,x,y) op_adc<T>(*this,(x),(y))
#define INC(T,x) op_inc<T>(*this,(x))
#define DEC(T,x) op_dec<T>(*this,(x))
#define NEG(T,x) op_neg<T>(*this,(x))

#define ok() return interpreter_result::ok	     // implemented
#define die() return interpreter_result::unsupported // operation not supported yet (should implement)
#define illegal() return interpreter_result::illegal // illegal byte sequence (can't implement)
#define wont() return interpreter_result::wont_trace // not worth tracing (won't implement)
#define impossible() return interpreter_result::logic_error

template <typename os> interpreter_result::code interpreter::interpret_locked_opcode() { 
    typedef int8_t b;
    typedef int16_t w;
    typedef typename os::v v;
    typedef typename os::z z;

    if (op.mod == 3) 
	illegal(); 

    int64_t addr = memory_operand_address();
    v * vp = reinterpret_cast<v*>(addr); // aliases
    b * bp = reinterpret_cast<b*>(addr);
    w * wp = reinterpret_cast<w*>(addr);
    z * zp = reinterpret_cast<z*>(addr);

    switch (op.code) { 
    case 0x1b0: // LOCK CMPXCHG Mb,Gb
	reg<b>(0,lock_cmpxchg<b>(*this,reinterpret_cast<b*>(memory_operand_address()), reg<b>(0), G<b>())); 
	ok();
    case 0x1b1: // LOCK CMPXCHG Mv,Gv
	reg<v>(0,lock_cmpxchg<v>(*this,reinterpret_cast<v*>(memory_operand_address()), reg<v>(0), G<v>())); 
	ok();
    case 0xfe:
	switch (op.reg) { 
        case 0: lock_inc<b>(*this,bp); ok(); // LOCK INC Mb
        case 1: lock_dec<b>(*this,bp); ok(); // LOCK DEC Mb
	default: illegal();
	}
    case 0xff:
	switch (op.reg) { 
        case 0: lock_inc<v>(*this,vp); ok(); // LOCK INC Mv
        case 1: lock_dec<v>(*this,vp); ok(); // LOCK DEC Mv
	default: illegal();
	}
    default: die();
    }
}

template <typename os> interpreter_result::code interpreter::interpret_opcode() { 
    typedef int8_t b;
    typedef int16_t w;
    typedef typename os::v v;
    typedef typename os::z z;

    switch (op.code) { 
    case 0x00: E<b>(ADD(b,E<b>(),G<b>())); ok();    // ADD Eb, Gb
    case 0x01: E<v>(ADD(b,E<v>(),G<v>())); ok();    // ADD Ev, Gv
    case 0x02: G<b>(ADD(b,G<b>(),E<b>())); ok();    // ADD Gb, Eb
    case 0x03: G<v>(ADD(v,G<v>(),E<v>())); ok();    // ADD Gv, Ev
    case 0x04: reg<b>(0, ADD(b,reg<b>(0),op.imm)); ok(); // ADD AL, Ib
    case 0x05: reg<v>(0, ADD(v,reg<v>(0),op.imm)); ok(); // ADD rAX, Iz
    case 0x06: illegal(); // PUSH ES
    case 0x07: illegal(); // POP ES
    case 0x08: E<b>(OR(b,E<b>(),G<b>())); ok();    // OR Eb, Gb
    case 0x09: E<v>(OR(v,E<v>(),G<v>())); ok();    // OR Ev, Gv
    case 0x0a: G<b>(OR(b,G<b>(),E<b>())); ok();    // OR Gb, Eb
    case 0x0b: G<v>(OR(v,G<v>(),E<v>())); ok();    // OR Gv, Ev
    case 0x0c: reg<b>(0, OR(b,reg<b>(0),op.imm)); ok(); // OR AL, Ib
    case 0x0d: reg<v>(0, OR(v,reg<v>(0),op.imm)); ok(); // OR rAX, Iz
    case 0x0e: illegal(); // PUSH CS
    case 0x0f: die(); // 3DNow! 0F 0F prefixes not supported
    case 0x10: E<b>(ADC(b,E<b>(),G<b>())); ok();    // ADC Eb, Gb
    case 0x11: E<v>(ADC(v,E<v>(),G<v>())); ok();    // ADC Ev, Gv
    case 0x12: G<b>(ADC(b,G<b>(),E<b>())); ok();    // ADC Gb, Eb
    case 0x13: G<v>(ADC(v,G<v>(),E<v>())); ok();    // ADC Gv, Ev
    case 0x14: reg<b>(0, ADC(b,reg<b>(0),op.imm)); ok(); // ADC AL, Ib
    case 0x15: reg<v>(0, ADC(v,reg<v>(0),op.imm)); ok(); // ADC rAX, Iz
    case 0x16: illegal(); // PUSH SS
    case 0x17: illegal(); // POP SS
    case 0x18: E<b>(SBB(b,E<b>(),G<b>())); ok();    // SBB Eb, Gb
    case 0x19: E<v>(SBB(v,E<v>(),G<v>())); ok();    // SBB Ev, Gv
    case 0x1a: G<b>(SBB(b,G<b>(),E<b>())); ok();    // SBB Gb, Eb
    case 0x1b: G<v>(SBB(v,G<v>(),E<v>())); ok();    // SBB Gv, Ev
    case 0x1c: reg<b>(0, SBB(b,reg<b>(0),op.imm)); ok(); // SBB AL, Ib
    case 0x1d: reg<v>(0, SBB(v,reg<v>(0),op.imm)); ok(); // SBB rAX, Iz
    case 0x1e: illegal(); // PUSH DS
    case 0x1f: illegal(); // POP DS
    case 0x20: E<b>(AND(b,E<b>(),G<b>())); ok();    // AND Eb, Gb
    case 0x21: E<v>(AND(v,E<v>(),G<v>())); ok();    // AND Ev, Gv
    case 0x22: G<b>(AND(b,G<b>(),E<b>())); ok();    // AND Gb, Eb
    case 0x23: G<v>(AND(v,G<v>(),E<v>())); ok();    // AND Gv, Ev
    case 0x24: reg<b>(0, AND(b,reg<b>(0),op.imm)); ok(); // AND AL, Ib
    case 0x25: reg<v>(0, AND(v,reg<v>(0),op.imm)); ok(); // AND rAX, Iz
    case 0x27: illegal(); // DAA
    case 0x26: impossible(); // ES: prefix
    case 0x28: E<b>(SUB(b,E<b>(),G<b>())); ok();    // SUB Eb, Gb
    case 0x29: E<v>(SUB(v,E<v>(),G<v>())); ok();    // SUB Ev, Gv
    case 0x2a: G<b>(SUB(b,G<b>(),E<b>())); ok();    // SUB Gb, Eb
    case 0x2b: G<v>(SUB(v,G<v>(),E<v>())); ok();    // SUB Gv, Ev
    case 0x2c: reg<b>(0,SUB(b,reg<b>(0),op.imm)); ok(); // SUB AL, Ib
    case 0x2d: reg<v>(0,SUB(v,reg<v>(0),op.imm)); ok(); // SUB rAX, Iz
    case 0x2e: impossible(); // CS: prefix
    case 0x2f: illegal(); // DAS
    case 0x30: E<b>(XOR(b,E<b>(),G<b>())); ok();    // XOR Eb, Gb
    case 0x31: E<v>(XOR(v,E<v>(),G<v>())); ok();    // XOR Ev, Gv
    case 0x32: G<b>(XOR(b,G<b>(),E<b>())); ok();    // XOR Gb, Eb
    case 0x33: G<v>(XOR(v,G<v>(),E<v>())); ok();    // XOR Gv, Ev
    case 0x34: reg<b>(0,XOR(b,reg<b>(0),op.imm)); ok(); // XOR AL, Ib
    case 0x35: reg<v>(0,XOR(v,reg<v>(0),op.imm)); ok(); // XOR rAX, Iz
    case 0x36: impossible(); // SS
    case 0x37: illegal(); // AAA
    case 0x38: SUB(b,E<b>(),G<b>()); ok();    // CMP Eb, Gb
    case 0x39: SUB(v,E<v>(),G<v>()); ok();    // CMP Ev, Gv
    case 0x3a: SUB(b,G<b>(),E<b>()); ok();    // CMP Gb, Eb
    case 0x3b: SUB(v,G<v>(),E<v>()); ok();    // CMP Gv, Ev
    case 0x3c: SUB(b,reg<b>(0),op.imm); ok(); // CMP AL, Ib
    case 0x3d: SUB(v,reg<v>(0),op.imm); ok(); // CMP rAX, Iz
    case 0x3e: impossible(); // DS: prefix
    case 0x3f: illegal(); // AAS
    case 0x40: case 0x41: case 0x42: case 0x43: 
    case 0x44: case 0x45: case 0x46: case 0x47:
    case 0x48: case 0x49: case 0x4a: case 0x4b: 
    case 0x4c: case 0x4d: case 0x4e: case 0x4f: // REX
	impossible();
    case 0x50: case 0x51: case 0x52: case 0x53: 
    case 0x54: case 0x55: case 0x56: case 0x57: // PUSH rXX
	push<v>(reg<v>(op.rex_b(op.code & 7))); ok();
    case 0x58: case 0x59: case 0x5a: case 0x5b: 
    case 0x5c: case 0x5d: case 0x5e: case 0x5f: // POP rXX
	reg<v>(op.rex_b(op.code & 7),pop<v>()); ok();
    case 0x60: illegal(); // PUSHA
    case 0x61: illegal(); // POPA
    case 0x62: illegal(); // BOUND Gv,Ma
    case 0x63: die();     // TODO: ARPL Ew, Gw
    case 0x64: impossible(); // FS: prefix
    case 0x65: impossible(); // GS: prefix
    case 0x66: impossible(); // OS prefix
    case 0x67: impossible(); // AS prefix
    case 0x68: push<v>(op.imm); ok(); // PUSH Iz
    case 0x69: die(); // TODO: IMUL Gv,Ev,Iz
    case 0x6a: push<v>(op.imm); ok(); // PUSH Ib (check stack slot size?)
    case 0x6b: die(); // TODO: IMUL Gv,Ev,Ib
    case 0x6c: die(); // TODO: INS Yb, DX
    case 0x6d: die(); // TODO: INS Yz, DX
    case 0x6e: die(); // TODO: OUTS DX, Xb
    case 0x6f: die(); // TODO: OUTS DX, Xz
    case 0x70: case 0x71: case 0x72: case 0x73: 
    case 0x74: case 0x75: case 0x76: case 0x77: 
    case 0x78: case 0x79: case 0x7a: case 0x7b: 
    case 0x7c: case 0x7d: case 0x7e: case 0x7f: // JCC Ib
	if (test_cc(op.code & 0xf)) 
	    rip() += op.imm;
	ok();
    case 0x80: // group #1 Eb, Ib
	switch (op.reg) { 
	case 0: E<b>(ADD(b,E<b>(),op.imm)); break;
	case 1: E<b>(OR(b,E<b>(),op.imm)); break;
	case 2: E<b>(ADC(b,E<b>(),op.imm)); break;
	case 3: E<b>(SBB(b,E<b>(),op.imm)); break;
	case 4: E<b>(AND(b,E<b>(),op.imm)); break;
	case 5: E<b>(SUB(b,E<b>(),op.imm)); break;
	case 6: E<b>(XOR(b,E<b>(),op.imm)); break;
	case 7: SUB(b,E<b>(),op.imm); break;
	}
	ok();
    // case 0x81 below
    case 0x82: illegal(); // group #1* Eb, Ib
    case 0x81: // group #1 Ev, Iz
    case 0x83: // group #1 Ev, Ib
	switch (op.reg) { 
	case 0: E<v>(ADD(v,E<v>(),op.imm)); break;
	case 1: E<v>(OR(v,E<v>(),op.imm)); break;
	case 2: E<v>(ADC(v,E<v>(),op.imm)); break;
	case 3: E<v>(SBB(v,E<v>(),op.imm)); break;
	case 4: E<v>(AND(v,E<v>(),op.imm)); break;
	case 5: E<v>(SUB(v,E<v>(),op.imm)); break;
	case 6: E<v>(XOR(v,E<v>(),op.imm)); break;
	case 7: SUB(v,E<v>(),op.imm); break;
	}
	ok();
    case 0x84: AND(b,E<b>(),G<b>()); ok(); // TEST Eb, Gb
    case 0x85: AND(v,E<v>(),G<v>()); ok(); // TEST Ev, Gv
    // NB: exhaustive case match ends here
    case 0x88: E<b>(G<b>()); ok(); // MOV Eb,Gb
    case 0x89: E<v>(G<v>()); ok(); // MOV Ev,Gv
    case 0x90: asm("pause"); ok(); // PAUSE (NOP 0x90)
    case 0x8a: G<b>(E<b>()); ok(); // MOV Gb,Eb
    case 0x8b: G<v>(E<v>()); ok(); // MOV Gv,Ev
    case 0x8c: die(); // MOV Rv, Sw or MOV Mw,Sw
    case 0x8d: G<v>(memory_operand_address(false)); ok(); // LEA Gv, M
    case 0x8e: die(); // MOV Sw, Mw or MOV Sw,Rv
    case 0x8f: // POP Ev (8F /0)
	if (op.reg != 0) illegal();
	E<v>(pop<v>());
	ok();
    case 0x9a: illegal(); // CALL Ap
    case 0x9e: // SAHF if CPUID.AHF = 1
	// TODO: be smarter about lazy flags here
        rflags() = (rflags() & ~0xff) | ah(); 
	ok();
    case 0x9f: // LAHF if CPUID.AHF = 1
	// TODO: be smarter about lazy flags here
        ah() = static_cast<uint8_t>(rflags() & 0xff);
	ok();
    case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf: 
	// MOV RXX, Iq
        reg<v>(op.rex_b(op.code & 7),op.imm);
	ok();
    case 0xc3: rip() = pop<v>(); ok(); // RET
    case 0xc4: illegal(); // LES Gz,Mp
    case 0xc5: illegal(); // LDS Gz,Mp
    case 0xc6: // MOV Eb, Ib (group #12)
	if (op.reg != 0) illegal();
	E<b>(op.imm);
	ok();
    case 0xc7: // MOV Ev, Iz (group #12)
	if (op.reg != 0) illegal();
	E<v>(op.imm);
	ok();
    case 0xce: illegal(); // INTO
    case 0xd4: illegal(); // AAM Ib
    case 0xd5: illegal(); // AAD Ib
    case 0xd6: illegal(); // SALC
    case 0xe8: // CALL Jz
	push<v>(rip());
	rip() += op.imm;
	ok();
    case 0xe9: rip() += op.imm; ok(); // JMP Jz
    case 0xea: illegal(); // JMP Ap
    case 0xeb: rip() += op.imm; ok(); // JMP Jb
    case 0xf4: wont(); // HLT
    case 0xf6: // group 3 Eb
	switch (op.reg) { 
	case 0: // TEST Eb,Ib
	case 1: // TEST Eb,Ib*
	    AND(b,E<b>(),op.imm); 
  	    ok(); 
	}
    case 0xf7: // group 3 Ev
	switch (op.reg) { 
	case 0: //  TEST Ev,Iz 
	case 1:	//  TEST Ev,Iz*
	    rip() += sizeof(z);
	    AND(v,E<v>(),op.imm); 
	    ok();
	}
    case 0xf8: cf(false); ok(); // CLC
    case 0xf9: cf(true); ok(); // STC
    case 0xfa: wont(); // CLI
    case 0xfb: wont(); // STI
    case 0xfc: df(false); ok(); // CLD
    case 0xfd: df(true); ok(); // STD
    case 0xfe: // opcode group 4
	switch (op.reg) {
        case 0: E<b>(INC(b,E<b>())); ok(); // INC Eb
        case 1: E<b>(DEC(b,E<b>())); ok(); // DEC Eb
	default: illegal();
	}
    case 0xff: // opcode group 5
	switch (op.reg) { 
        case 0: E<v>(INC(v,E<v>())); ok(); // INC Ev
        case 1: E<v>(DEC(v,E<v>())); ok(); // DEC Ev
	case 2: // CALL Ev
	    push<v>(rip()); 
	    rip() = E<v>(); 
	    ok();
	case 3: die();
	case 4: rip() = E<v>(); ok(); // JMP Ev
	case 5: die();
	case 6: push<v>(E<v>()); ok(); // PUSH Ev
	case 7: illegal();
	default: die();
	}
    case 0x10b: illegal(); // UD2
    case 0x120: die(); // MOV Rd,Cd
    case 0x121: die(); // MOV Rd,Dd
    case 0x122: die(); // MOV Cd,Rd
    case 0x123: die(); // MOV Dd,Rd
    case 0x134: die(); // SYSENTER (illegal on AMD64, legal on EMT64?, unrecognized by udis86)
    case 0x135: die(); // SYSEXIT (illegal on AMD64, legal on EMT64)
    case 0x140: case 0x141: case 0x142: case 0x143: 
    case 0x144: case 0x145: case 0x146: case 0x147: 
    case 0x148: case 0x149: case 0x14a: case 0x14b:
    case 0x14c: case 0x14d: case 0x14e: case 0x14f: // CMOVcc Gv,Ev
	if (test_cc(op.code & 0xf)) 
	    G<v>(E<v>());
	ok();

    case 0x180: case 0x181: case 0x182: case 0x183: 
    case 0x184: case 0x185: case 0x186: case 0x187: 
    case 0x188: case 0x189: case 0x18a: case 0x18b:
    case 0x18c: case 0x18d: case 0x18e: case 0x18f: // Jcc Jz
	if (test_cc(op.code & 0xf)) 
	    rip() += op.imm;
	ok();
    case 0x190: case 0x191: case 0x192: case 0x193: 
    case 0x194: case 0x195: case 0x196: case 0x197:
    case 0x198: case 0x199: case 0x19a: case 0x19b: 
    case 0x19c: case 0x19d: case 0x19e: case 0x19f: // SETcc Eb
        E<b>(test_cc(op.code & 0xf));
	ok();
    case 0x1a0: die(); // push<v>(fs()); return i; // PUSH FS 
    case 0x1a8: die(); // push<v>(gs()); return i; // PUSH GS
    case 0x1a1: die(); // fs(pop<v>()); return i;  // POP FS
    case 0x1a9: die(); // gs(pop<v>()); return i;  // POP GS
    case 0x1b0: // CMPXCHG Eb,Gb
	{
	    int8_t value = E<b>();
	    if (zf(value == reg<b>(0))) E<b>(G<b>());
 	    else reg<b>(0,value);
	    ok();
	}
    case 0x1b1: // CMPXCHG Ev,Gv
	{
	    int8_t value = E<v>();
	    if (zf(value == reg<v>(0))) E<v>(G<v>());
 	    else reg<v>(0,value);
	    ok();
	}
    case 0x1b6: G<v>(static_cast<uint64_t>(static_cast<uint8_t>(E<b>()))); ok(); // MOVZX Gv, Ev
    case 0x1b9: illegal(); // UD1
    case 0x1ff: illegal(); // UD0
    default: break;
    } 
    die();
} // interpreter::interpret

} // namespace jitpp
