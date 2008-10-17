#ifndef INCLUDED_JITNEY_TRACER_H
#define INCLUDED_JITNEY_TRACER_H

#include <sys/types.h>
#include <stdint.h> // int64_t, etc
#include <exception>
#include <ucontext.h>

namespace jitney { 
    
    struct reg64 { 
	union { 
            int64_t m_data;
	    uint64_t m_udata; 
        };
    public:
        reg64() : m_data(0) {}
	reg64(int64_t data) : m_data(data) {}

        reg64 & operator = (int64_t data) { m_data = data; }
        reg64 & operator = (uint64_t data) { m_udata = data; }

        operator int64_t & () { return m_data; } 
        operator int64_t () const { return m_data; } 
        operator uint64_t & () { return m_udata; } 
        operator uint64_t () const { return m_udata; } 
        // template <typename T> operator T * () { return reinterpret_cast<T*>(m_data); } 
        template <typename T> operator T * () const { return reinterpret_cast<T*>(m_data); } 
    
        // we know the platform. make endianness assumptions
        int8_t & l() { return *reinterpret_cast<int8_t*>(&m_data); } 
        int8_t l() const { return *reinterpret_cast<const int8_t*>(&m_data); } 
        int8_t & h() { return *(reinterpret_cast<int8_t*>(&m_data)+1); } 
        int8_t h() const { return *(reinterpret_cast<const int8_t*>(&m_data)+1); } 
        int16_t & x() { return *reinterpret_cast<int16_t*>(&m_data); } 
        int16_t x() const { return *reinterpret_cast<const int16_t*>(&m_data); } 
    };
    
    namespace modrm { 
        static const int rax = 0;
        static const int rcx = 1;
        static const int rdx = 2;
        static const int rbx = 3;
        static const int rsp = 4;
        static const int rbp = 5;
        static const int rsi = 6;
        static const int rdi = 7;
    }
    
    // a tracer pauses the current fiber and moves execution to a new fiber with the original fiber's context
    // prepped in registers for interpretation or other manipulation
    class tracer {
    public:
        static size_t default_stack_size();
    
        tracer(size_t stack_size = default_stack_size());
        ~tracer();
        void start() throw();
    protected:
	// upon returning, execution resumes wherever we've evaluated the opcodes up to
        virtual void run() = 0;

    private:
        void *m_fs_base;	    // fs segment base register
	void *m_gs_base;	    // gs segment base register
        reg64 m_reg[16]; 	    // registers resorted for faster mod r/m
        ucontext_t m_context;       // the context we are interpreting
	ucontext_t m_meta_context ; // the context for the interpreter (const)

	// helper function
        static void run_tracer(tracer *) throw();
    public:
	// output a disassembly of the current opcode
	void print_instruction() throw();

        // exposed registers in modrm order.
	reg64 * regs() throw() { return m_reg; } 
	const reg64 * regs() const throw() { return m_reg; }

	inline void * fs_base() throw() { return m_fs_base; } 
	inline void * gs_base() throw() { return m_gs_base; } 
    
        // the rflags/eflags/flags register
        reg64 & rflags() throw() { return reinterpret_cast<reg64 &>(m_context.uc_mcontext.gregs[REG_EFL]); }
        const reg64 rflags() const throw() { return m_context.uc_mcontext.gregs[REG_EFL]; }
    
        // the instruction pointer
        reg64 & rip() throw() { return reinterpret_cast<reg64 &>(m_context.uc_mcontext.gregs[REG_RIP]); }
        const reg64 rip() const throw() { return m_context.uc_mcontext.gregs[REG_RIP]; }
    
        // segments are NOT saved by swapcontext, but we need the base for modrm calculations
        void * fs_base() const throw() { return m_fs_base; } 
        void * gs_base() const throw() { return m_gs_base; } 
    } __attribute__((aligned(16)));
    
} // jitney

#endif
