#ifndef INCLUDED_JITPP_FLAGS_H
#define INCLUDED_JITPP_FLAGS_H

#include <jit++/tracer.h>

namespace jitpp { 
    namespace flags { 

        static const int32_t CF = 0x0001;
        static const int32_t PF = 0x0004;
        static const int32_t AF = 0x0010;
        static const int32_t ZF = 0x0040;
        static const int32_t SF = 0x0080;
        static const int32_t TF = 0x0100;
        static const int64_t IF = 0x0200;
        static const int32_t DF = 0x0400;
        static const int32_t OF = 0x0800;
    
        static const int32_t OSZAPC = OF | SF | ZF | AF | PF | CF; 
        static const int32_t OSZAP = OF | SF | ZF | AF | PF;
        static const int64_t OSZPC = OF | SF | ZF | PF | CF;
    
        // plain old data
        struct context { 
            int64_t op1, op2, result;
        };
    
        // plain old data
        struct handler {
    	    bool(*m_cf)(const context &);
    	    bool(*m_af)(const context &);
    	    bool(*m_of)(const context &);
            const int32_t m_handled_flags;
            const int32_t m_cleared_flags;

            inline bool cf(const context & ctx) const { return m_cf(ctx); } 
            inline bool af(const context & ctx) const { return m_af(ctx); } 
            inline bool of(const context & ctx) const { return m_of(ctx); }
            inline int32_t handled_flags() const { return m_handled_flags; } 
	    inline int32_t cleared_flags() const { return m_cleared_flags; } 
        };
    
        extern const uint8_t parity_lut[256];
        inline bool calculate_parity(uint8_t b) { return parity_lut[b] != 0; }
    
        template <typename Base> class lazy_mixin : public Base { 
        public:
            lazy_mixin() : m_handler(0), m_lazy_flags(0) {}
        private:
            const handler * m_handler;
            context m_context;
    
            // intentionally discard constness
            int64_t & base_rflags() const { return const_cast<Base *>(static_cast<const Base *>(this))->m_rflags; } 

            inline bool is_lazy(int32_t mask) const { return (m_lazy_flags & mask) != 0; } 
            inline bool has_flag(int32_t mask) const { return (base_rflags() & mask) != 0; } 
    
            mutable int64_t m_lazy_flags;
    
        public:
            inline int64_t lazy_flags() const { return m_lazy_flags; } 
            inline bool cf() const { return is_lazy(CF) ? force_lazy(CF, m_handler->cf(m_context)) : has_flag(CF); }
            inline bool pf() const { return is_lazy(PF) ? force_lazy(PF, calculate_parity(m_context.result)) : has_flag(PF); }
            inline bool af() const { return is_lazy(AF) ? force_lazy(AF, m_handler->af(m_context)) : has_flag(AF); }
            inline bool zf() const { return is_lazy(ZF) ? force_lazy(ZF, m_context.result == 0) : has_flag(ZF); }
            inline bool sf() const { return is_lazy(SF) ? force_lazy(SF, m_context.result < 0) : has_flag(ZF); }
            inline bool of() const { return is_lazy(OF) ? force_lazy(OF, m_handler->of(m_context)) : has_flag(OF); }
            inline bool tf() const { return has_flag(TF); }
            inline bool df() const { return has_flag(DF); }
    
            inline bool set_flag(int32_t mask, bool value = true) { 
                m_lazy_flags &= ~mask;
                if (value) base_rflags() |= mask; 
                else base_rflags() &= ~mask;
                return value;
            }

	    inline bool set_flags(int32_t mask, int32_t value) { 
                m_lazy_flags &= ~mask;
		base_rflags() = (base_rflags() & ~mask) | (value & mask);
	    } 
    
            inline bool cf(bool value) { return set_flag(CF,value); }
            inline bool pf(bool value) { return set_flag(PF,value); }
            inline bool af(bool value) { return set_flag(AF,value); }
            inline bool zf(bool value) { return set_flag(ZF,value); }
            inline bool sf(bool value) { return set_flag(SF,value); }
            inline bool of(bool value) { return set_flag(OF,value); }
            inline bool tf(bool value) { if (value) base_rflags() |= TF; else base_rflags() &= ~TF; return value; }
            inline bool df(bool value) { if (value) base_rflags() |= DF; else base_rflags() &= ~DF; return value; }
    
        private:
            inline bool force_lazy(int64_t mask, bool value = true) const { 
                m_lazy_flags &= ~mask;
                if (value) base_rflags() |= mask; 
                else base_rflags() &= ~mask;
                return value;
            }
    
        public:
            // set carry, parity, aux, zero, sign and overflow all at once, note, new_flag_handler must not go out of scope before rflags() is requested!
            int64_t handle_rflags(const handler & h, int64_t result, int64_t op1 = 0, int64_t op2 = 0) {
		 base_rflags() &= ~h.cleared_flags();
                 m_lazy_flags &= ~(h.handled_flags() | h.cleared_flags()) & OSZAPC;
                 // force any flags that don't overlap with the new flag handler to evaluate
                 rflags();
            
                 // install the new flag handler
                 m_lazy_flags = h.handled_flags();
                 m_handler = &h;
                 m_context.op1 = op1;
                 m_context.op2 = op2;
                 m_context.result = result;
                 return result;
            }
            inline int64_t & rflags() { 
                 if (m_lazy_flags != 0) { 
                     // force flags
                     cf(); pf(); af(); zf(); sf(); of();
                 }
                 return base_rflags();
            }
                 
            inline void rflags(int64_t value) { 
                 m_lazy_flags = 0;
                 base_rflags() = value;
            }
        };
    
        // evaluate all flags eagerly
        template <typename Base> class strict_mixin : public Base { 
        public:
    	    inline bool cf() { return (base_rflags() & CF) != 0; }
            inline bool pf() { return (base_rflags() & PF) != 0; }
            inline bool af() { return (base_rflags() & AF) != 0; }
            inline bool zf() { return (base_rflags() & ZF) != 0; }
            inline bool sf() { return (base_rflags() & SF) != 0; }
            inline bool of() { return (base_rflags() & OF) != 0; }
            inline bool tf() { return (base_rflags() & TF) != 0; }
            inline bool df() { return (base_rflags() & DF) != 0; }
    
            inline bool cf(bool value) { return set_rflag(CF,value); }
            inline bool pf(bool value) { return set_rflag(PF,value); }
            inline bool af(bool value) { return set_rflag(AF,value); }
            inline bool zf(bool value) { return set_rflag(ZF,value); }
            inline bool sf(bool value) { return set_rflag(SF,value); }
            inline bool of(bool value) { return set_rflag(OF,value); }
            inline bool tf(bool value) { return set_rflag(TF,value); }
            inline bool df(bool value) { return set_rflag(DF,value); }
    
            bool set_rflag(int64_t mask, bool value = true) { 
                if (value) base_rflags() |= mask;
    	        else base_rflags() &= ~mask;
    	        return value;
    	    }
    
            inline int64_t handle_rflags(const handler & h, int64_t result, int64_t op1 = 0, int64_t op2 = 0) {
                context ctx;
                ctx.op1 = op1;
                ctx.op2 = op2;
                ctx.result = result;
		base_rflags() &= ~h.cleared_flags();
                int64_t f = h.handled_flags();
                if ((f & CF) != 0) cf(h.cf(ctx));
                if ((f & AF) != 0) af(h.af(ctx));
                if ((f & OF) != 0) of(h.of(ctx));
                if ((f & PF) != 0) pf(calculate_parity(result));
                if ((f & ZF) != 0) zf(result == 0);
                if ((f & SF) != 0) sf(result < 0);
                return result;
            }
    
            inline int64_t & rflags() const { return base_rflags(); }
            inline void rflags(int64_t value) { base_rflags() = value; }
    
        private:
            int64_t & base_rflags() { return static_cast<Base *>(this)->m_rflags; } 
            int64_t base_rflags() const { return static_cast<const Base *>(this)->m_rflags; } 
        };

        bool neg_cf(const context &);
        bool neg_af(const context &);
        bool inc_af(const context &);
        bool dec_af(const context &);
	bool bad_flag(const context &);
        
        template <typename T> struct typed { 
            static const int size = sizeof(T)*8;
            static const int shift_mask = (size == 64) ? 0x3f : 0x1f;
        
            static inline bool sbb_cf(const context & ctx) { 
                return (ctx.op1 < ctx.result || (static_cast<T>(ctx.op2) == static_cast<T>(0xffffffffffffffffULL)));
            }

            static inline bool dec_of(const context & ctx) { 
                return ctx.result == (1ULL << ((sizeof(T) * 8) - 1)) - 1;
            }
            static inline bool neg_inc_of(const context & ctx) { 
                return ctx.result == 1ULL << (size - 1);
            }

            static const handler neg, inc, dec;
        };
        
        template <typename T> const handler typed<T>::neg = { neg_cf,   neg_af, 	neg_inc_of, 	OSZAPC, 0 };
        template <typename T> const handler typed<T>::inc = { bad_flag, inc_af, 	neg_inc_of, 	OSZAP, 	0 };
        template <typename T> const handler typed<T>::dec = { bad_flag, dec_af, 	dec_of, 	OSZAP, 	0 };
        
        template <typename T> inline bool test_cc(T & i, uint8_t cc) {
            switch (cc) { 
            case 0x0: return i.of();
            case 0x1: return !i.of();
            case 0x2: return i.cf();
            case 0x3: return !i.cf();
            case 0x4: return i.zf();
            case 0x5: return !i.zf();
            case 0x6: return i.cf() || i.zf();
            case 0x7: return !(i.cf() || i.zf());
            case 0x8: return i.sf();
            case 0x9: return !i.sf();
            case 0xa: return i.pf();
            case 0xb: return !i.pf();
            case 0xc: return i.sf() != i.of();
            case 0xd: return i.sf() == i.of();
            case 0xe: return i.zf() || (i.sf() != i.of());
            case 0xf: return i.zf() && (i.sf() == i.of());
            }
        }
    } // namespace flags;
} // namespace jitpp

#endif
