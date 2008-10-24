#ifndef INCLUDED_JITPP_RFLAGS_INTERNAL_H
#define INCLUDED_JITPP_RFLAGS_INTERNAL_H

#include <jit++/rflags.h>

namespace jitpp { 

template <typename T> class add_rflags : public rflags_handler {
public:
    inline bool cf(rflags_context & ctx) const { 
	return ctx.result<T>() < ctx.op1<T>();
    }
    inline bool af(rflags_context & ctx) const { 
	return (ctx.m_op1 ^ ctx.m_op2 ^ ctx.m_result) & 0x10;
    }
    inline bool of(rflags_context & ctx) const { 
	return (((~((ctx.m_op1) ^ (ctx.m_op2)) & ((ctx.m_op2) ^ (ctx.m_result))) & 0x8000000000000000ULL) != 0)
    }
    static const add_rflags instance;
};
template <typename T> static const add_rflags<T> add_rflags<T>::instance = add_rflags<T>();


template <typename T> class adc_rflags : public rflags_handler { 
public:
    inline bool cf(rflags_context & ctx) const { 
	return ctx.result<T>() <= ctx.op1<T>();
    }
    inline bool af(rflags_context & ctx) const { 
	return (ctx.m_op1 ^ ctx.m_op2 ^ ctx.m_result) & 0x10;
    }
    inline bool of(rflags_context & ctx) const { 
	return (((~((ctx.m_op1) ^ (ctx.m_op2)) & ((ctx.m_op2) ^ (ctx.m_result))) & 0x8000000000000000ULL) != 0)
    }
    static const adc_rflags instance;
};
template <typename T> static const adc_rflags<T> adc_rflags<T>::instance = adc_rflags<T>();


template <typename T> class sub_rflags : public rflags_handler {
public:
    inline bool cf(rflags_context & ctx) const { 
	return ctx.op1<T>() < ctx.op2<T>();
    }
    inline bool af(rflags_context & ctx) const { 
	return (ctx.m_op1 ^ ctx.m_op2 ^ ctx.m_result) & 0x10;
    }
    inline bool of(rflags_context & ctx) const { 
	return (((((ctx.m_op1) ^ (ctx.m_op2)) & ((ctx.m_op2) ^ (ctx.m_result))) & 0x8000000000000000ULL) != 0)
    }
    static const sub_rflags instance;
};
template <typename T> static const sub_rflags<T> sub_rflags<T>::instance = sub_rflags<T>();


template <typename T> class sbb_rflags : public rflags_handler {
public:
    inline bool cf(rflags_context & ctx) const { 
	return (ctx.op1<T>() < ctx.result<T>()) || (ctx.op2<T>() == static_cast<T>(0xffffffffffffffffULL));
    }
    inline bool af(rflags_context & ctx) const { 
	return (ctx.m_op1 ^ ctx.m_op2 ^ ctx.m_result) & 0x10;
    }
    inline bool of(rflags_context & ctx) const { 
	return (((((ctx.m_op1) ^ (ctx.m_op2)) & ((ctx.m_op2) ^ (ctx.m_result))) & 0x8000000000000000ULL) != 0)
    }
    static const sbb_rflags instance;
};
template <typename T> static const sbb_rflags<T> sbb_rflags<T>::instance = sbb_rflags<T>();


template <typename T> class neg_rflags : public rflags_handler { 
    dec_rflags() : public rflags_handler(rflags_cf_mask | rflags_af_mask | rflags_of_mask);
    inline bool cf(rflags_context & ctx) const { 
	return ctx.result<T>() != 0;
    }
    inline bool af(rflags_context & ctx) const { 
	return (ctx.m_result & 0xf) != 0;
    }
    inline bool of(rflags_context & ctx) const { 
	return ctx.result == 1ULL << ((sizeof(T) * 8) - 1);
    }
    static const neg_rflags instance;
};
template <typename T> static const neg_rflags<T> neg_rflags<T>::instance;


template <typename T> class inc_rflags : public rflags_handler { 
    dec_rflags() : public rflags_handler(rflags_af_mask | rflags_of_mask);
    inline bool af(rflags_context & ctx) const { 
	return (ctx.m_result & 0xf)  == 0;
    }
    inline bool of(rflags_context & ctx) const { 
	return ctx.result == 1ULL << ((sizeof(T) * 8) - 1);
    }
    static const inc_rflags instance;
};
template <typename T> static const inc_rflags<T> inc_rflags<T>::instance = inc_rflags<T>();


template <typename T> class dec_rflags : public rflags_handler { 
    dec_rflags() : public rflags_handler(rflags_af_mask | rflags_of_mask);
    inline bool af(rflags_context & ctx) const { 
	return (ctx.m_result & 0xf) == 0xf;
    }
    inline bool of(rflags_context & ctx) const { 
	return ctx.result == (1ULL << ((sizeof(T) * 8) - 1)) - 1;
    }
    static const dec_rflags instance;
};
template <typename T> static const dec_rflags<T> dec_rflags<T>::instance = dec_rflags<T>();


template <typename T> class logic_rflags : public rflags_handler { 
    inline bool cf(rflags_context & ctx) const { return false; } 
    inline bool af(rflags_context & ctx) const { return false; } 
    inline bool of(rflags_context & ctx) const { return false; } 
    static const logic_rflags instance;
};
template <typename T> static const logic_rflags<T> logic_rflags<T>::instance = logic_rflags<T>();

#endif
