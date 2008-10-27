#include <jit++/group_1.h>

namespace jitpp { 
   using namespace jitpp::flags;

   bool group_1_base::sbb_af(const context & ctx) { 
       return (ctx.op1 ^ ctx.op2 ^ ctx.result) & 0x10;
   }
   bool group_1_base::sbb_of(const context & ctx) { 
       return (((((ctx.op1) ^ (ctx.op2)) & ((ctx.op2) ^ (ctx.result))) & 0x8000000000000000ULL) != 0);
   }
   static bool add_cf(const context & ctx) { 
       return ctx.result < ctx.op1;
   }
   static bool add_of(const context & ctx) { 
       return (((~((ctx.op1) ^ (ctx.op2)) & ((ctx.op2) ^ (ctx.result))) & 0x8000000000000000ULL) != 0);
   }
   static bool adc_cf(const context & ctx) { 
       return ctx.result <= ctx.op1;
   }
   static bool sub_cf(const context & ctx) { 
       return ctx.op1 < ctx.op2;
   }
   const handler group_1_base::add_flags = { add_cf, sbb_af, add_of, OSZAPC,0 };
   const handler group_1_base::adc_flags = { adc_cf, sbb_af, add_of, OSZAPC,0 };
   const handler group_1_base::sub_flags = { sub_cf, sbb_af, sbb_of, OSZAPC,0 };
   const handler group_1_base::logic_flags = { bad_flag, bad_flag, bad_flag, SF | ZF | PF, OF | AF | CF };

} // namespace jitpp

