#include <jit++/flags.h>
#include <jit++/common.h>

namespace jitpp { 
    namespace flags { 
        const uint8_t parity_lut[256] = {
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1 
        };

        bool add_cf(const context & ctx) { 
            return ctx.result < ctx.op1;
        }
        bool arith_af(const context & ctx) { 
            return (ctx.op1 ^ ctx.op2 ^ ctx.result) & 0x10;
        }
        bool add_of(const context & ctx) { 
            return (((~((ctx.op1) ^ (ctx.op2)) & ((ctx.op2) ^ (ctx.result))) & 0x8000000000000000ULL) != 0);
        }
        bool adc_cf(const context & ctx) { 
            return ctx.result <= ctx.op1;
        }
        bool sub_cf(const context & ctx) { 
            return ctx.op1 < ctx.op2;
        }
        bool sub_of(const context & ctx) { 
            return (((((ctx.op1) ^ (ctx.op2)) & ((ctx.op2) ^ (ctx.result))) & 0x8000000000000000ULL) != 0);
        }
        bool neg_cf(const context & ctx) { 
            return ctx.result != 0;
        }
        bool neg_af(const context & ctx) { 
            return (ctx.result & 0xf) != 0;
        }
        bool inc_af(const context & ctx) { 
            return (ctx.result & 0xf)  == 0;
        }
        bool dec_af(const context & ctx) { 
            return (ctx.result & 0xf) == 0xf;
        }
        bool bad_flag(const context &) { 
            LOG(DFATAL) << "unexpected lazy flag";
            return false;
        }
        
        const handler add = { add_cf, arith_af, add_of, OSZAPC,0 };
        const handler adc = { adc_cf, arith_af, add_of, OSZAPC,0 };
        const handler sub = { sub_cf, arith_af, sub_of, OSZAPC,0 };
        const handler logic = { bad_flag, bad_flag, bad_flag, SF | ZF | PF, OF | AF | CF };
    } // namespace rflags
} // namespace jitpp

