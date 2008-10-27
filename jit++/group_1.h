#ifndef INCLUDED_JITPP_GROUP_1_H
#define INCLUDED_JITPP_GROUP_1_H

#include <jit++/interpreter_internal.h>

namespace jitpp { 

    // common flag management for stuff that doesn't care about types
    struct group_1_base { 
    	static inline int64_t add(interpreter & i, int64_t x, int64_t y) {
            return i.handle_rflags(add_flags,x+y,x,y);
        }
    	static inline int64_t sub(interpreter & i, int64_t x, int64_t y) {
            return i.handle_rflags(sub_flags,x-y,x,y);
        }
    	static inline int64_t and_(interpreter & i, int64_t x, int64_t y) {
            return i.handle_rflags(logic_flags,x&y);
        }
    	static inline int64_t or_(interpreter & i, int64_t x, int64_t y) {
            return i.handle_rflags(logic_flags,x|y);
        }
    	static inline int64_t xor_(interpreter & i, int64_t x, int64_t y) {
            return i.handle_rflags(logic_flags,x^y);
        }
    	static inline int64_t adc(interpreter & i, int64_t x, int64_t y) {
            if (i.cf()) return i.handle_rflags(adc_flags,x+y+1,x,y);
            else return i.handle_rflags(add_flags,x+y,x,y);
        }

        static bool sbb_af(const flags::context &);
        static bool sbb_of(const flags::context &);

        static const flags::handler add_flags, adc_flags, sub_flags, logic_flags;
    };

    template <typename T> struct group_1 : group_1_base { 
	static inline int64_t sbb(interpreter & i, int64_t x, int64_t y) {
            if (i.cf()) return i.handle_rflags(sbb_flags,x-y-1,x,y);
            else return i.handle_rflags(sub_flags,x-y,x,y);
        }
	// flags
        static inline bool sbb_cf(const flags::context & ctx) { 
            return (ctx.op1 < ctx.result || (static_cast<T>(ctx.op2) == static_cast<T>(0xffffffffffffffffULL)));
        }

        static const flags::handler sbb_flags;

	// interpret group
	static inline void interpret(interpreter & i, int64_t imm) {
            switch (i.reg) { 
            case 0: E<T>(i,add(i,E<T>(i),imm)); return; // ADD E?, I?
            case 1: E<T>(i,or_(i,E<T>(i),imm)); return; // OR  E?, I?
            case 2: E<T>(i,adc(i,E<T>(i),imm)); return; // ADC E?, I?
            case 3: E<T>(i,sbb(i,E<T>(i),imm)); return; // SBB E?, I?
            case 4: E<T>(i,and_(i,E<T>(i),imm)); return;// AND E?, I?
            case 5: E<T>(i,sub(i,E<T>(i),imm)); return; // SUB E?, I?
            case 6: E<T>(i,xor_(i,E<T>(i),imm));return; // XOR E?, I?
            case 7: sub(i,E<T>(i),imm); return;         // CMP E?, I?
            default: logic_error(); 
	    }
	}
    };

    template <typename T> const flags::handler group_1<T>::sbb_flags = { sbb_cf, sbb_af, sbb_of, flags::OSZAPC, 0 };

#define ADD(T,x,y) group_1_base::add(i,(x),(y))
#define SUB(T,x,y) group_1_base::sub(i,(x),(y))
#define SBB(T,x,y) group_1<T>::sbb(i,(x),(y))
#define CMP(T,x,y) group_1_base::add(i,(x),(y))
#define XOR(T,x,y) group_1_base::xor_(i,(x),(y))
#define AND(T,x,y) group_1_base::and_(i,(x),(y))
#define OR(T,x,y)  group_1_base::or_(i,(x),(y))
#define ADC(T,x,y) group_1_base::adc(i,(x),(y))

} // namespace jitpp

#endif
