#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

#include <jit++/decoder.h>
#include <jit++/tracer.h>
#include <jit++/flags.h>

namespace jitpp { 

    class interpreter : public flags::lazy_mixin<tracer>, public decoder { 
    public:
        interpreter() {}
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

	void print_regs();
	void print_opcode(int64_t rip, int expected_size = 0);

        inline int32_t eip() const { return static_cast<int32_t>(rip()); }
        inline int64_t rip() const { return m_rip; }
        inline int64_t & rip() { return m_rip; }

	int64_t mem(bool add_segment_base = true) const;

        inline int8_t & ah();
        inline int8_t ah() const;

	inline int64_t & rsp() { return m_reg[4]; } 
    };

    inline int8_t & interpreter::ah() { return reinterpret_cast<int8_t*>(m_reg)[1]; } 
    inline int8_t interpreter::ah() const { return reinterpret_cast<const int8_t*>(m_reg)[1]; } 
}

#endif
