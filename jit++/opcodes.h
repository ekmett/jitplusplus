#ifndef INCLUDED_JITPP_OPCODES
#define INCLUDED_JITPP_OPCODES

#include <stdint.h>

namespace jitpp { 
    static const uint8_t modrm_flag_has_modrm = 1;
    static const uint8_t modrm_flag_extra_byte = 2; // [0f 38] or [0f 3a]
    extern const uint8_t modrm_flag_lut[512];
}

#endif
