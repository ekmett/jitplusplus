#ifndef INCLUDED_JITPP_RFLAGS_INTERNAL_H
#define INCLUDED_JITPP_RFLAGS_INTERNAL_H

#include <jit++/rflags.h>

namespace jitpp { 

//TODO: confirm the flags for neg, inc, dec!

template <typename T> class add_rflags : public rflags_handler {
public:
    inline bool cf(const rflags_context & ctx) const { 
	return ctx.m_result < ctx.m_op1;
    }
    inline bool af(const rflags_context & ctx) const { 
	return (ctx.m_op1 ^ ctx.m_op2 ^ ctx.m_result) & 0x10;
    }
    inline bool of(const rflags_context & ctx) const { 
	return (((~((ctx.m_op1) ^ (ctx.m_op2)) & ((ctx.m_op2) ^ (ctx.m_result))) & 0x8000000000000000ULL) != 0);
    }
    static const add_rflags instance;
};
template <typename T> const add_rflags<T> add_rflags<T>::instance = add_rflags<T>();


template <typename T> class adc_rflags : public rflags_handler { 
public:
    inline bool cf(const rflags_context & ctx) const { 
	return ctx.m_result <= ctx.m_op1;
    }
    inline bool af(const rflags_context & ctx) const { 
	return (ctx.m_op1 ^ ctx.m_op2 ^ ctx.m_result) & 0x10;
    }
    inline bool of(const rflags_context & ctx) const { 
	return (((~((ctx.m_op1) ^ (ctx.m_op2)) & ((ctx.m_op2) ^ (ctx.m_result))) & 0x8000000000000000ULL) != 0);
    }
    static const adc_rflags instance;
};
template <typename T> const adc_rflags<T> adc_rflags<T>::instance = adc_rflags<T>();


template <typename T> class sub_rflags : public rflags_handler {
public:
    inline bool cf(const rflags_context & ctx) const { 
	return ctx.m_op1 < ctx.m_op2;
    }
    inline bool af(const rflags_context & ctx) const { 
	return (ctx.m_op1 ^ ctx.m_op2 ^ ctx.m_result) & 0x10;
    }
    inline bool of(const rflags_context & ctx) const { 
	return (((((ctx.m_op1) ^ (ctx.m_op2)) & ((ctx.m_op2) ^ (ctx.m_result))) & 0x8000000000000000ULL) != 0);
    }
    static const sub_rflags instance;
};
template <typename T> const sub_rflags<T> sub_rflags<T>::instance = sub_rflags<T>();


template <typename T> class sbb_rflags : public rflags_handler {
public:
    inline bool cf(const rflags_context & ctx) const { 
	return (ctx.m_op1 < ctx.m_result || (ctx.op2<T>() == static_cast<T>(0xffffffffffffffffULL)));
    }
    inline bool af(const rflags_context & ctx) const { 
	return (ctx.m_op1 ^ ctx.m_op2 ^ ctx.m_result) & 0x10;
    }
    inline bool of(const rflags_context & ctx) const { 
	return (((((ctx.m_op1) ^ (ctx.m_op2)) & ((ctx.m_op2) ^ (ctx.m_result))) & 0x8000000000000000ULL) != 0);
    }
    static const sbb_rflags instance;
};
template <typename T> const sbb_rflags<T> sbb_rflags<T>::instance = sbb_rflags<T>();


template <typename T> class neg_rflags : public rflags_handler { 
    neg_rflags() : rflags_handler(rflags_cf_mask | rflags_af_mask | rflags_of_mask) {}
public:
    inline bool cf(const rflags_context & ctx) const { 
	return ctx.m_result != 0;
    }
    inline bool af(const rflags_context & ctx) const { 
	return (ctx.m_result & 0xf) != 0;
    }
    inline bool of(const rflags_context & ctx) const { 
	return ctx.m_result == 1ULL << ((sizeof(T) * 8) - 1);
    }
    static const neg_rflags instance;
};
template <typename T> const neg_rflags<T> neg_rflags<T>::instance;


template <typename T> class inc_rflags : public rflags_handler { 
    inc_rflags() : rflags_handler(rflags_af_mask | rflags_of_mask | rflags_sf_mask | rflags_pf_mask | rflags_zf_mask) {}
public:
    inline bool af(const rflags_context & ctx) const { 
	return (ctx.m_result & 0xf)  == 0;
    }
    inline bool of(const rflags_context & ctx) const { 
	return ctx.m_result == 1ULL << ((sizeof(T) * 8) - 1);
    }
    static const inc_rflags instance;
};
template <typename T> const inc_rflags<T> inc_rflags<T>::instance = inc_rflags<T>();


template <typename T> class dec_rflags : public rflags_handler { 
    dec_rflags() : rflags_handler(rflags_af_mask | rflags_of_mask | rflags_sf_mask | rflags_pf_mask | rflags_zf_mask) {}
public:
    inline bool af(const rflags_context & ctx) const { 
	return (ctx.m_result & 0xf) == 0xf;
    }
    inline bool of(const rflags_context & ctx) const { 
	return ctx.m_result == (1ULL << ((sizeof(T) * 8) - 1)) - 1;
    }
    static const dec_rflags instance;
};
template <typename T> const dec_rflags<T> dec_rflags<T>::instance = dec_rflags<T>();


template <typename T> class logic_rflags : public rflags_handler { 
public:
    inline bool cf(const rflags_context & ctx) const { return false; } 
    inline bool af(const rflags_context & ctx) const { return false; } 
    inline bool of(const rflags_context & ctx) const { return false; } 
    static const logic_rflags instance;
};
template <typename T> const logic_rflags<T> logic_rflags<T>::instance = logic_rflags<T>();

}

#endif
