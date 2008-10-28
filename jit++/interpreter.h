#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

#include <jit++/decoder.h>
#include <jit++/tracer.h>
#include <jit++/flags.h>

namespace jitpp { 

    class interpreter : public flags::lazy_mixin<tracer>, public decoder { 
    public:
        void run();

    private:
        // we defer the syscall for these until they are used.
        mutable int64_t m_fs_base;
        mutable int64_t m_gs_base;

        mutable bool m_fs_base_known;
        mutable bool m_gs_base_known;

    public:
        int64_t fs_base() const;
        int64_t gs_base() const;
	int64_t seg_base() const; 

	void print_regs();
	void print_opcode(int64_t rip, int expected_size = 0);

        inline int32_t eip() const { return static_cast<int32_t>(rip()); }
        inline int64_t rip() const { return m_rip; }
        inline int64_t & rip() { return m_rip; }

	int64_t repetitions() const;
	int64_t mem(bool add_segment_base = true) const;

        inline int8_t & ah();
        inline int8_t ah() const;

	inline int64_t & rax() { return m_reg[0]; } 
	inline int64_t & rcx() { return m_reg[1]; } 
	inline int64_t & rdx() { return m_reg[2]; }
	inline int64_t & rbx() { return m_reg[3]; }
	inline int64_t & rsp() { return m_reg[4]; } 
	inline int64_t & rbp() { return m_reg[5]; } 
	inline int64_t & rsi() { return m_reg[6]; }
	inline int64_t & rdi() { return m_reg[7]; }
	inline int64_t & r8() { return m_reg[8]; }
	inline int64_t & r9() { return m_reg[9]; }
	inline int64_t & r10() { return m_reg[10]; }
	inline int64_t & r11() { return m_reg[11]; }
	inline int64_t & r12() { return m_reg[12]; }
	inline int64_t & r13() { return m_reg[13]; }
	inline int64_t & r14() { return m_reg[14]; }
	inline int64_t & r15() { return m_reg[15]; }

	inline int64_t rax() const { return m_reg[0]; } 
	inline int64_t rcx() const { return m_reg[1]; } 
	inline int64_t rdx() const { return m_reg[2]; }
	inline int64_t rbx() const { return m_reg[3]; }
	inline int64_t rsp() const { return m_reg[4]; } 
	inline int64_t rbp() const { return m_reg[5]; } 
	inline int64_t rsi() const { return m_reg[6]; }
	inline int64_t rdi() const { return m_reg[7]; }
	inline int64_t r8() const { return m_reg[8]; }
	inline int64_t r9() const { return m_reg[9]; }
	inline int64_t r10() const { return m_reg[10]; }
	inline int64_t r11() const { return m_reg[11]; }
	inline int64_t r12() const { return m_reg[12]; }
	inline int64_t r13() const { return m_reg[13]; }
	inline int64_t r14() const { return m_reg[14]; }
	inline int64_t r15() const { return m_reg[15]; }

	inline int32_t eax() const { return m_reg[0]; } 
	inline int32_t ecx() const { return m_reg[1]; } 
	inline int32_t edx() const { return m_reg[2]; }
	inline int32_t ebx() const { return m_reg[3]; }
	inline int32_t esp() const { return m_reg[4]; } 
	inline int32_t ebp() const { return m_reg[5]; } 
	inline int32_t esi() const { return m_reg[6]; }
	inline int32_t edi() const { return m_reg[7]; }

	inline int16_t ax() const { return m_reg[0]; } 
	inline int16_t cx() const { return m_reg[1]; } 
	inline int16_t dx() const { return m_reg[2]; }
	inline int16_t bx() const { return m_reg[3]; }
	inline int16_t sp() const { return m_reg[4]; } 
	inline int16_t bp() const { return m_reg[5]; } 
	inline int16_t si() const { return m_reg[6]; }
	inline int16_t di() const { return m_reg[7]; }

	inline int8_t al() const { return m_reg[0]; } 
	inline int8_t cl() const { return m_reg[1]; } 
	inline int8_t dl() const { return m_reg[2]; }
	inline int8_t bl() const { return m_reg[3]; }
    };

    inline int8_t & interpreter::ah() { return reinterpret_cast<int8_t*>(m_reg)[1]; } 
    inline int8_t interpreter::ah() const { return reinterpret_cast<const int8_t*>(m_reg)[1]; } 
}

#endif
