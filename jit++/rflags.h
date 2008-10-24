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

    static const int64_t arith_lazy_rflags = rflags_of_mask | rflags_sf_mask | rflags_zf_mask | rflags_af_mask | rflags_pf_mask | rflags_cf_mask; 

    // plain old data
    struct rflags_context { 
        int64_t m_op1, m_op2, m_result;
        template <typename T> T op1() const { return static_cast<T>(m_op1); }
        template <typename T> T op2() const { return static_cast<T>(m_op2); }
        template <typename T> T result() const { return static_cast<T>(m_result); }
        template <typename T> void op1(T value) { m_op1 = static_cast<int64_t>(m_op1); }
        template <typename T> void op2(T value) { m_op2 = static_cast<int64_t>(m_op2); }
        template <typename T> void result(T value) { m_result = static_cast<int64_t>(m_result); }
    };

    class rflags_handler {
    public:
        rflags_handler(int64_t lazy_rflags = arith_lazy_rflags) : m_lazy_rflags(lazy_rflags) {}
        virtual bool of(const rflags_context & ctx) const;
        virtual bool af(const rflags_context & ctx) const;
        virtual bool cf(const rflags_context & ctx) const;
        inline int64_t lazy_rflags() const { return m_lazy_rflags; } 
    private:
        const int64_t m_lazy_rflags;
    };


    extern const uint8_t rflags_parity_lut[256];
    template <typename T> inline bool rflags_calculate_parity(T b) { return rflags_parity_lut[static_cast<uint8_t>(b)] != 0; }

    template <typename Base> class lazy_rflags_strategy_mixin : public Base { 
    public:
	lazy_rflags_strategy_mixin() : m_rflags_handler(0), m_lazy_rflags(0) {}
    private:
	rflags_handler * m_rflags_handler;
	rflags_context m_rflags_context;

	// this intentionally discards constness
	int64_t & base_rflags() const { return const_cast<Base *>(static_cast<const Base *>(this))->m_rflags; } 
        inline bool is_lazy_rflag(int64_t mask) const { return (m_lazy_rflags & mask) != 0; } 
        inline bool has_rflag(int64_t mask) const { return (base_rflags() & mask) != 0; } 

	mutable int64_t m_lazy_rflags;

    public:
	inline bool cf() const { 
	    return is_lazy_rflag(rflags_cf_mask) 
		? force_lazy_rflag(rflags_cf_mask,m_rflags_handler->cf(m_rflags_context)) 
		: has_rflag(rflags_cf_mask);
	}
        inline bool pf() const { 
	    return is_lazy_rflag(rflags_pf_mask)
		? force_lazy_rflag(rflags_pf_mask, rflags_calculate_parity(m_rflags_context.result<uint8_t>()))
		: has_rflag(rflags_pf_mask);
	}
        inline bool af() const { 
	    return is_lazy_rflag(rflags_pf_mask)
	        ? force_lazy_rflag(rflags_af_mask, m_rflags_handler->af(m_rflags_context)) 
	        : has_rflag(rflags_af_mask);
	}
        inline bool zf() const {
	    return is_lazy_rflag(rflags_zf_mask)
		? force_lazy_rflag(rflags_zf_mask, m_rflags_context.m_result == 0) 
		: has_rflag(rflags_zf_mask);
	}
        inline bool sf() const { 
	    return is_lazy_rflag(rflags_sf_mask)
		? force_lazy_rflag(m_rflags_context.m_result < 0) 
		: has_rflag(rflags_zf_mask);
	}
        inline bool of() const { 
	    return is_lazy_rflag(rflags_of_mask) 
		? force_lazy_rflag(m_rflags_handler->of(m_rflags_context)) 
		: has_rflag(rflags_of_mask);
	}
        inline bool tf() const { return has_rflag(rflags_tf_mask); }
        inline bool df() const { return has_rflag(rflags_df_mask); }

        inline bool set_rflag(int64_t mask, bool value = true) { 
	    m_lazy_rflags &= ~mask;
	    if (value) base_rflags() |= mask; 
	    else base_rflags() &= ~mask;
	    return value;
	}

        inline bool cf(bool value) { return set_rflag(rflags_cf_mask,value); }
        inline bool pf(bool value) { return set_rflag(rflags_pf_mask,value); }
        inline bool af(bool value) { return set_rflag(rflags_af_mask,value); }
        inline bool zf(bool value) { return set_rflag(rflags_zf_mask,value); }
        inline bool sf(bool value) { return set_rflag(rflags_sf_mask,value); }
        inline bool of(bool value) { return set_rflag(rflags_of_mask,value); }
        inline bool tf(bool value) { if (value) base_rflags() |= rflags_tf_mask; else base_rflags() &= ~rflags_tf_mask; return value; }
        inline bool df(bool value) { if (value) base_rflags() |= rflags_df_mask; else base_rflags() &= ~rflags_df_mask; return value; }

    private:
        inline bool force_lazy_rflag(int64_t mask, bool value = true) const { 
	    m_lazy_rflags &= ~mask;
	    if (value) base_rflags() |= mask; 
	    else base_rflags() &= ~mask;
	    return value;
	}

    public:
	// set carry, parity, aux, zero, sign and overflow all at once, note, new_flag_handler must not go out of scope before rflags() is requested!
	template <typename T> T set_rflags_context(rflags_handler & new_rflags_handler, T result, int64_t op1 = 0, int64_t op2 = 0) {
	     // force any flags that don't overlap with the new flag handler to evaluate
	     m_lazy_rflags &= ~new_rflags_handler.lazy_rflags() & arith_lazy_rflags;
	     rflags();
	
	     // install the new flag handler
	     m_lazy_rflags = new_rflags_handler.lazy_rflags();
	     m_rflags_handler = &new_rflags_handler;
	     m_rflags_context.op1 = op1;
	     m_rflags_context.op2 = op2;
	     m_rflags_context.result = result;
	     return result;
	}
	inline int64_t & rflags() { 
	     if (m_lazy_rflags != 0) { 
		 // force flags
	         cf(); pf(); af(); zf(); sf(); of();
  	     }
	     return base_rflags();
	}
	     
        inline void rflags(int64_t value) { 
	     m_lazy_rflags = 0;
	     base_rflags() = value;
	}
    };

    // evaluate all flags eagerly
    template <typename Base> class strict_rflags_strategy_mixin : public Base { 
    public:
	inline bool cf() { return (base_rflags() & rflags_cf_mask) != 0; }
        inline bool pf() { return (base_rflags() & rflags_pf_mask) != 0; }
        inline bool af() { return (base_rflags() & rflags_af_mask) != 0; }
        inline bool zf() { return (base_rflags() & rflags_zf_mask) != 0; }
        inline bool sf() { return (base_rflags() & rflags_sf_mask) != 0; }
        inline bool of() { return (base_rflags() & rflags_of_mask) != 0; }
        inline bool tf() { return (base_rflags() & rflags_tf_mask) != 0; }
        inline bool df() { return (base_rflags() & rflags_df_mask) != 0; }

        inline bool cf(bool value) { return set_rflag(rflags_cf_mask,value); }
        inline bool pf(bool value) { return set_rflag(rflags_pf_mask,value); }
        inline bool af(bool value) { return set_rflag(rflags_af_mask,value); }
        inline bool zf(bool value) { return set_rflag(rflags_zf_mask,value); }
        inline bool sf(bool value) { return set_rflag(rflags_sf_mask,value); }
        inline bool of(bool value) { return set_rflag(rflags_of_mask,value); }
        inline bool tf(bool value) { return set_rflag(rflags_tf_mask,value); }
        inline bool df(bool value) { return set_rflag(rflags_df_mask,value); }

        bool set_rflag(int64_t mask, bool value = true) { 
            if (value) base_rflags() |= mask;
	    else base_rflags() &= ~mask;
	    return value;
	}

	template <typename T> T set_rflags_context(rflags_handler & new_rflags_handler, T result, int64_t op1 = 0, int64_t op2 = 0) {
	    rflags_context ctx;
	    ctx.op1(op1);
	    ctx.op2(op2);
	    ctx.result(result);
	    int64_t handled_flags = new_flag_handler.lazy_rflags();
	    if ((handled_flags & rflags_cf_mask) != 0) cf(new_flag_handler.cf(ctx));
	    if ((handled_flags & rflags_af_mask) != 0) af(new_flag_handler.af(ctx));
	    if ((handled_flags & rflags_of_mask) != 0) of(new_flag_handler.of(ctx));
	    if ((handled_flags & rflags_pf_mask) != 0) pf(rflags_calculate_parity(result));
	    if ((handled_flags & rflags_zf_mask) != 0) zf(result == 0);
	    if ((handled_flags & rflags_sf_mask) != 0) sf(result < 0);
	    return result;
	}

	inline int64_t & rflags() const { return base_rflags(); }
	inline void rflags(int64_t value) { base_rflags() = value; }

    private:
	int64_t & base_rflags() { return static_cast<Base *>(this)->m_rflags; } 
	int64_t base_rflags() const { return static_cast<const Base *>(this)->m_rflags; } 
    };
}

#endif
