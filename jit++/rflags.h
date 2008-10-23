#ifndef INCLUDED_JITPP_RFLAGS
#define INCLUDED_JITPP_RFLAGS

#include <jit++/tracer.h>

namespace jitpp { 
    
    static const int64_t rflags_cf_mask = 0x0001;
    static const int64_t rflags_pf_mask = 0x0004;
    static const int64_t rflags_af_mask = 0x0010;
    static const int64_t rflags_zf_mask = 0x0040;
    static const int64_t rflags_sf_mask = 0x0080;
    static const int64_t rflags_tf_mask = 0x0100;
    static const int64_t rflags_if_mask = 0x0200;
    static const int64_t rflags_df_mask = 0x0400;
    static const int64_t rflags_of_mask = 0x0800;

    static const int64_t arith_rflags = rflags_of_mask | rflags_sf_mask | rflags_zf_mask | rflags_af_mask | rflags_pf_mask | rflags_cf_mask; 

    // plain old data
    struct rflags_context { 
        int64_t op1, op2, result;
    };

    class rflags_handler {
    public:
        rflags_handler(int64_t lazy_rflags = arith_rflags) : m_lazy_rflags(lazy_rflags) {}
        __attribute__((pure)) virtual bool of(const rflags_context &) const = 0;
        __attribute__((pure)) virtual bool sf(const rflags_context &) const = 0;
        __attribute__((pure)) virtual bool zf(const rflags_context &) const = 0;
        __attribute__((pure)) virtual bool af(const rflags_context &) const = 0;
        __attribute__((pure)) virtual bool pf(const rflags_context &) const = 0;
        __attribute__((pure)) virtual bool cf(const rflags_context &) const = 0;
        inline int64_t lazy_rflags() const { return m_lazy_rflags; } 
    private:
        int64_t m_lazy_rflags;
    };

    template <typename Base> class lazy_rflags_strategy_mixin : public Base { 
    private:
	rflags_handler * m_rflags_handler;
	rflags_context m_rflags_context;

	int64_t & super_rflags() const { return static_cast<const Base *>(this)->m_rflags; } 

	mutable int64_t m_lazy_rflags;

        inline bool has_lazy_cf() const { return (m_lazy_rflags & rflags_cf_mask) != 0; }
        inline bool has_lazy_pf() const { return (m_lazy_rflags & rflags_pf_mask) != 0; }
        inline bool has_lazy_af() const { return (m_lazy_rflags & rflags_af_mask) != 0; }
        inline bool has_lazy_zf() const { return (m_lazy_rflags & rflags_zf_mask) != 0; }
        inline bool has_lazy_sf() const { return (m_lazy_rflags & rflags_sf_mask) != 0; }
        inline bool has_lazy_of() const { return (m_lazy_rflags & rflags_of_mask) != 0; }

    public:
	inline bool cf() const { return has_lazy_cf() ? cf_lazy(m_rflags_handler->cf(m_rflags_context)) : (super_rflags() & rflags_cf_mask) != 0; }
        inline bool pf() const { return has_lazy_pf() ? pf_lazy(m_rflags_handler->pf(m_rflags_context)) : (super_rflags() & rflags_pf_mask) != 0; }
        inline bool af() const { return has_lazy_af() ? af_lazy(m_rflags_handler->af(m_rflags_context)) : (super_rflags() & rflags_af_mask) != 0; }
        inline bool zf() const { return has_lazy_zf() ? zf_lazy(m_rflags_handler->zf(m_rflags_context)) : (super_rflags() & rflags_zf_mask) != 0; }
        inline bool sf() const { return has_lazy_sf() ? sf_lazy(m_rflags_handler->sf(m_rflags_context)) : (super_rflags() & rflags_sf_mask) != 0; }
        inline bool of() const { return has_lazy_of() ? of_lazy(m_rflags_handler->of(m_rflags_context)) : (super_rflags() & rflags_of_mask) != 0; }
        inline bool tf() const { return (super_rflags() & rflags_tf_mask) != 0; }
        inline bool df() const { return (super_rflags() & rflags_df_mask) != 0; }

        inline bool set_rflag(int64_t mask, bool value = true) { 
	    m_lazy_rflags &= ~mask;
	    if (value) super_rflags() |= mask; 
	    else super_rflags() &= ~mask;
	    return value;
	}

        inline bool cf(bool value) { return set_rflag(rflags_cf_mask,value); }
        inline bool pf(bool value) { return set_rflag(rflags_pf_mask,value); }
        inline bool af(bool value) { return set_rflag(rflags_af_mask,value); }
        inline bool zf(bool value) { return set_rflag(rflags_zf_mask,value); }
        inline bool sf(bool value) { return set_rflag(rflags_sf_mask,value); }
        inline bool of(bool value) { return set_rflag(rflags_of_mask,value); }
        inline bool tf(bool value) { if (value) super_rflags() |= rflags_tf_mask; else super_rflags() &= ~rflags_tf_mask; return value; }
        inline bool df(bool value) { if (value) super_rflags() |= rflags_df_mask; else super_rflags() &= ~rflags_df_mask; return value; }

    private:
        inline bool set_rflag_lazy(int64_t mask, bool value = true) const { 
	    m_lazy_rflags &= ~mask;
	    if (value) super_rflags() |= mask; 
	    else super_rflags() &= ~mask;
	    return value;
	}

        inline bool cf_lazy(bool value) const { return set_rflag_lazy(rflags_cf_mask,value); }
        inline bool pf_lazy(bool value) const { return set_rflag_lazy(rflags_pf_mask,value); }
        inline bool af_lazy(bool value) const { return set_rflag_lazy(rflags_af_mask,value); }
        inline bool zf_lazy(bool value) const { return set_rflag_lazy(rflags_zf_mask,value); }
        inline bool sf_lazy(bool value) const { return set_rflag_lazy(rflags_sf_mask,value); }
        inline bool of_lazy(bool value) const { return set_rflag_lazy(rflags_of_mask,value); }
	
    public:
	// set carry, parity, aux, zero, sign and overflow all at once, note, new_flag_handler must not go out of scope before rflags() is requested!
	void set_rflags_context(rflags_handler & new_rflags_handler, int64_t op1, int64_t op2, int64_t result) { 
	     // force any flags that don't overlap with the new flag handler to evaluate
	     m_lazy_rflags &= ~new_rflags_handler.lazy_rflags() & arith_rflags;
	     rflags();
	
	     // install the new flag handler
	     m_lazy_rflags = new_rflags_handler.lazy_rflags();
	     m_rflags_handler = &new_rflags_handler;
	     m_rflags_context.op1 = op1;
	     m_rflags_context.op2 = op2;
	     m_rflags_context.result = result;
	}
	inline int64_t & rflags() { 
	     if (m_lazy_rflags != 0) { 
	         if ((m_lazy_rflags & rflags_cf_mask) != 0) cf(m_rflags_handler->cf(m_rflags_context));
	         if ((m_lazy_rflags & rflags_pf_mask) != 0) pf(m_rflags_handler->pf(m_rflags_context));
	         if ((m_lazy_rflags & rflags_af_mask) != 0) af(m_rflags_handler->af(m_rflags_context));
	         if ((m_lazy_rflags & rflags_zf_mask) != 0) zf(m_rflags_handler->zf(m_rflags_context));
	         if ((m_lazy_rflags & rflags_sf_mask) != 0) sf(m_rflags_handler->sf(m_rflags_context));
	         if ((m_lazy_rflags & rflags_of_mask) != 0) of(m_rflags_handler->of(m_rflags_context));
  	     }
	     return super_rflags();
	}
	     
        inline void rflags(int64_t value) { 
	     m_lazy_rflags = 0;
	     super_rflags() = value;
	}
    };

    // evaluate all flags eagerly
    template <typename Base> class strict_rflags_strategy_mixin : public Base { 
    public:
	inline bool cf() { return (super_rflags() & rflags_cf_mask) != 0; }
        inline bool pf() { return (super_rflags() & rflags_pf_mask) != 0; }
        inline bool af() { return (super_rflags() & rflags_af_mask) != 0; }
        inline bool zf() { return (super_rflags() & rflags_zf_mask) != 0; }
        inline bool sf() { return (super_rflags() & rflags_sf_mask) != 0; }
        inline bool of() { return (super_rflags() & rflags_of_mask) != 0; }
        inline bool tf() { return (super_rflags() & rflags_tf_mask) != 0; }
        inline bool df() { return (super_rflags() & rflags_df_mask) != 0; }

        inline bool cf(bool value) { return set_rflag(rflags_cf_mask,value); }
        inline bool pf(bool value) { return set_rflag(rflags_pf_mask,value); }
        inline bool af(bool value) { return set_rflag(rflags_af_mask,value); }
        inline bool zf(bool value) { return set_rflag(rflags_zf_mask,value); }
        inline bool sf(bool value) { return set_rflag(rflags_sf_mask,value); }
        inline bool of(bool value) { return set_rflag(rflags_of_mask,value); }
        inline bool tf(bool value) { return set_rflag(rflags_tf_mask,value); }
        inline bool df(bool value) { return set_rflag(rflags_df_mask,value); }

        bool set_rflag(int64_t mask, bool value = true) { 
            if (value) super_rflags() |= mask;
	    else super_rflags() &= ~mask;
	    return value;
	}

	void set_rflags_context(rflags_handler & new_flag_handler, int64_t op1, int64_t op2, int64_t result) { 
	    rflags_context ctx;
	    ctx.op1 = op1;
	    ctx.op2 = op2;
	    ctx.result = result;
	    cf(new_flag_handler.cf(ctx));
	    pf(new_flag_handler.pf(ctx));
	    af(new_flag_handler.af(ctx));
	    zf(new_flag_handler.zf(ctx));
	    sf(new_flag_handler.sf(ctx));
	    of(new_flag_handler.of(ctx));
	}

	inline int64_t & rflags() const { return super_rflags(); }
	inline void rflags(int64_t value) { super_rflags() = value; }

    private:
	int64_t & super_rflags() const { return static_cast<const Base *>(this)->m_rflags; } 
    };
}

#endif
