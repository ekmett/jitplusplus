#ifndef INCLUDED_JITPP_INTERPRETING_GROUP_1_H
#define INCLUDED_JITPP_INTERPRETING_GROUP_1_H

#include <jit++/interpreting/base.h>

namespace jitpp { 
  namespace interpreting { 

    extern bool sbb_af(const flags::context &);
    extern bool sbb_of(const flags::context &);

    template <typename T> struct group_1_traits {
        static inline bool sbb_cf(const flags::context & ctx) { 
            return (ctx.op1 < ctx.result || (static_cast<T>(ctx.op2) == static_cast<T>(0xffffffffffffffffULL)));
        }
        static const flags::handler sbb_flags;
    };
    template <typename T> const flags::handler group_1_traits<T>::sbb_flags = { sbb_cf, sbb_af, sbb_of, flags::OSZAPC, 0 };

    class group_1 : public virtual interpreter_base { 
    public:
    	inline int64_t add(int64_t x, int64_t y) {
            return handle_rflags(add_flags,x+y,x,y);
        }
    	inline int64_t sub(int64_t x, int64_t y) {
            return handle_rflags(sub_flags,x-y,x,y);
        }
    	inline int64_t and_(int64_t x, int64_t y) {
            return handle_rflags(logic_flags,x&y);
        }
    	inline int64_t or_(int64_t x, int64_t y) {
            return handle_rflags(logic_flags,x|y);
        }
    	inline int64_t xor_(int64_t x, int64_t y) {
            return handle_rflags(logic_flags,x^y);
        }
    	inline int64_t adc(int64_t x, int64_t y) {
            if (cf()) return handle_rflags(adc_flags,x+y+1,x,y);
            else return handle_rflags(add_flags,x+y,x,y);
        }
        template <typename T>
	inline int64_t sbb(int64_t x, int64_t y) {
            if (cf()) return handle_rflags(group_1_traits<T>::sbb_flags,x-y-1,x,y);
            else return handle_rflags(sub_flags,x-y,x,y);
        }

        template <typename T>
	inline void interpret_group_1(int64_t imm) {
            switch (reg) { 
            case 0: E<T>(add(E<T>(),imm)); return; // ADD E?, I?
            case 1: E<T>(or_(E<T>(),imm)); return; // OR  E?, I?
            case 2: E<T>(adc(E<T>(),imm)); return; // ADC E?, I?
            case 3: E<T>(sbb<T>(E<T>(),imm)); return; // SBB E?, I?
            case 4: E<T>(and_(E<T>(),imm)); return;// AND E?, I?
            case 5: E<T>(sub(E<T>(),imm)); return; // SUB E?, I?
            case 6: E<T>(xor_(E<T>(),imm));return; // XOR E?, I?
            case 7: sub(E<T>(),imm); return; // CMP E?, I?
            default: logic_error(); 
	    }
	}

    private:
        static const flags::handler add_flags, adc_flags, sub_flags, logic_flags;
    };
  }
} // namespace jitpp

#endif
