#include <jit++/common.h>
#include <jit++/interpreting/group_1.h>

namespace jitpp {
  namespace interpreting { 
    using namespace jitpp::interpreting::flags;
 
    bool sbb_af(const context & ctx) { 
        return (ctx.op1 ^ ctx.op2 ^ ctx.result) & 0x10;
    }
    bool sbb_of(const context & ctx) { 
        return (((((ctx.op1) ^ (ctx.op2)) & ((ctx.op2) ^ (ctx.result))) & 0x8000000000000000ULL) != 0);
    }
    static bool add_cf(const context & ctx) { 
	// VLOG(1) << "add_cf " << (ctx.result < ctx.op1 ? "true" : "false");
        return (uint64_t)ctx.result < (uint64_t)ctx.op1;
    }
    static bool add_of(const context & ctx) { 
        return (((~((ctx.op1) ^ (ctx.op2)) & ((ctx.op2) ^ (ctx.result))) & 0x8000000000000000ULL) != 0);
    }
    static bool adc_cf(const context & ctx) { 
        return (uint64_t)ctx.result <= (uint64_t)ctx.op1;
    }
    static bool sub_cf(const context & ctx) { 
        return (uint64_t)ctx.op1 < (uint64_t)ctx.op2;
    }
    const handler group_1::add_flags = { add_cf, sbb_af, add_of, OSZAPC,0 };
    const handler group_1::adc_flags = { adc_cf, sbb_af, add_of, OSZAPC,0 };
    const handler group_1::sub_flags = { sub_cf, sbb_af, sbb_of, OSZAPC,0 };
    const handler group_1::logic_flags = { bad_flag, bad_flag, bad_flag, SF | ZF | PF, OF | AF | CF };
  }
} // namespace jitpp

