#ifndef INCLUDED_JITPP_DECODER_H
#define INCLUDED_JITPP_DECODER_H

#include <stdint.h>

namespace jitpp { 

    struct decoder { 
	int64_t  imm;        // any immediate value byte, word, dword
	int32_t  disp;       // displacement
	uint16_t parts;      // part_* indicates which optional parts are present
        uint16_t code;     // xx of 1xx if opcode is a 1 byte or has 0f xx form
	uint8_t  sse_prefix; // 0 = none, 1 = 66 only, 2 = f2, 3 = f3
        uint8_t  prefix;     // rex nybble, flags for other prefixes
        uint8_t  extra;      // 0f 3a and 0f 38 have an extra byte
        uint8_t  mod;	  
        uint8_t  reg;        // extended with rex_r
        uint8_t  rm;         // extended with rex_b
	uint8_t  log_scale;  // 1,2,4 or 8
	uint8_t  index;      // extended with rex_x
	uint8_t  base;       // extended with rex_b
	union { 
	    uint8_t  drex;   // drex byte value if present
	    uint8_t  imm2;   // stack slot count for enter
	};
	uint8_t  seg_prefix; 
	uint8_t  log_v;      // default operand size = 2^log_v bytes

        // disassemble the opcode starting at rip
        // returns address of subsequent instruction on success
	int64_t parse(int64_t rip);

	inline bool has_seg_prefix() const 	{ return seg_prefix != 0; }
	inline bool has_os_prefix() const 	{ return (prefix & prefix_66h_mask) != 0; } 
	inline bool has_as_prefix() const 	{ return (prefix & prefix_67h_mask) != 0; } 
	inline bool has_lock_prefix() const 	{ return (prefix & prefix_lock_mask) != 0; } 
	inline bool has_repne_prefix() const 	{ return sse_prefix == 2; } 
	inline bool has_rep_prefix() const 	{ return sse_prefix == 3; }
	inline bool has_rex() const 		{ return (prefix & prefix_rex_mask) != 0; } 
	inline bool has_rex_w() const 		{ return (prefix & prefix_rex_w_mask) != 0; } 
	inline bool has_rex_r() const 		{ return (prefix & prefix_rex_r_mask) != 0; } 
	inline bool has_rex_x() const 		{ return (prefix & prefix_rex_x_mask) != 0; } 
	inline bool has_rex_b() const 		{ return (prefix & prefix_rex_b_mask) != 0; } 
	inline bool has_extra() const 		{ return (parts & part_extra) != 0; } 
	inline bool has_modrm() const 		{ return (parts & part_modrm) != 0; } 
	inline bool has_sib() const 		{ return (parts & part_sib) != 0; } 
	inline bool has_drex() const 		{ return (parts & part_drex) != 0; } 
	inline bool has_disp() const 		{ return (parts & part_disp) != 0; } 
	inline bool has_imm() const 		{ return (parts & part_imm) != 0; } 
	inline bool has_imm2() const 		{ return (parts & part_imm2) != 0; } 
	inline bool is_rip_relative() const 	{ return !has_sib() && (mod == 0) && ((rm & 7) == 5); }
	inline bool has_memory_operand() const  { return mod != 3; } // assumes has_modrm
	inline bool address_size_is_64() const  { return (prefix & prefix_67h_mask) == 0; } 

	inline uint8_t v() const                { return 1 << log_v; }
	inline uint8_t scale() const		{ return 1 << log_scale; } 

	// augment with rex, input is a 3 bit wide value
	inline uint8_t rex_w(uint8_t b) const { return (prefix & prefix_rex_w_mask) | b; } 
	inline uint8_t rex_r(uint8_t b) const { return ((prefix & prefix_rex_r_mask) << 1) | b; } 
	inline uint8_t rex_x(uint8_t b) const { return ((prefix & prefix_rex_x_mask) << 2) | b; } 
	inline uint8_t rex_b(uint8_t b) const { return ((prefix & prefix_rex_b_mask) << 3) | b; } 

	// flags and lookups
        static const uint8_t prefix_legacy_mask = 0xf0;
        static const uint8_t prefix_repxx_mask  = 0x80;
        static const uint8_t prefix_lock_mask   = 0x40;
        static const uint8_t prefix_67h_mask    = 0x20;
        static const uint8_t prefix_66h_mask    = 0x10;

        static const uint8_t prefix_rex_mask    = 0x0f;
        static const uint8_t prefix_rex_w_mask  = 0x08;
        static const uint8_t prefix_rex_r_mask  = 0x04;
        static const uint8_t prefix_rex_x_mask  = 0x02;
        static const uint8_t prefix_rex_b_mask  = 0x01;

	// extra, modrm, sib, drex, disp, imm, imm2
	static const uint8_t part_modrm = 0x80; // == encoding_modrm_byte
	static const uint8_t part_extra = 0x40; // == encoding_extra_byte
	static const uint8_t part_drex  = 0x20; // == encoding_drex_byte
	static const uint8_t part_sib   = 0x10; // scale index & base are valid
	static const uint8_t part_disp  = 0x08;
	static const uint8_t part_imm   = 0x04;
	static const uint8_t part_imm2  = 0x02;

        static const uint8_t encoding_has_modrm  = 0x80; 
        static const uint8_t encoding_extra_byte = 0x40; // [0f 38] or [0f 3a]
	static const uint8_t encoding_drex_byte  = 0x20; // [0f 24] of [0f 25]
	static const uint8_t encoding_default_os_64 = 0x10; // 50-5f, ff/6 8f/0 

        static const uint8_t encoding_immediate_mask = 0x07;
	static const uint8_t encoding_no_imm = 0x00;
	static const uint8_t encoding_Ib     = 0x01;
	static const uint8_t encoding_Iz     = 0x02;
	static const uint8_t encoding_Iw     = 0x03;
	static const uint8_t encoding_IwIb   = 0x04; // ENTER
	static const uint8_t encoding_Iv     = 0x05; // z,w,q as appropriate to size
	static const uint8_t encoding_hard   = 0x06; // handle opcode by opcode hard code

        static const uint8_t encoding_lut[512];

    };
}

#endif
