#ifndef INCLUDED_JITPP_TRACER_H
#define INCLUDED_JITPP_TRACER_H

#include <sys/types.h>
#include <stdint.h> // int64_t, etc
#include <exception>
#include <ucontext.h>

namespace jitpp { 
/*    
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
 */  
    static const int reg_rax = 0;
    static const int reg_rcx = 1;
    static const int reg_rdx = 2;
    static const int reg_rbx = 3;
    static const int reg_rsp = 4;
    static const int reg_rbp = 5;
    static const int reg_rsi = 6;
    static const int reg_rdi = 7;
    static const char * reg_name[16];
    
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
        int64_t m_reg[16]; 	    // registers resorted for faster mod r/m

    private:
        int64_t m_fs_base;	    // fs segment base register
	int64_t m_gs_base;	    // gs segment base register
        ucontext_t m_context;       // the context we are interpreting
	ucontext_t m_meta_context ; // the context for the interpreter (const)

	// helper function
        static void run_tracer(tracer *) throw();
    public:
	// output a disassembly of the current opcode
	void print_instruction() throw();

        // exposed registers in modrm order.
	int64_t * regs() throw() { return m_reg; } 
	const int64_t * regs() const throw() { return m_reg; }

	inline int64_t fs_base() throw() { return m_fs_base; } 
	inline int64_t gs_base() throw() { return m_gs_base; } 
    
        // the rflags/eflags/flags register
        int64_t & rflags() throw() { return m_context.uc_mcontext.gregs[REG_EFL]; }
        int64_t flags() const throw() { return m_context.uc_mcontext.gregs[REG_EFL]; }
    
        // the instruction pointer
        int64_t & rip() throw() { return m_context.uc_mcontext.gregs[REG_RIP]; }
        int64_t rip() const throw() { return m_context.uc_mcontext.gregs[REG_RIP]; }
        int32_t eip() const throw() { return *reinterpret_cast<const int32_t*>(m_context.uc_mcontext.gregs + REG_RIP); }

        // the instruction pointer
        int64_t & rsp() throw() { return m_context.uc_mcontext.gregs[REG_RSP]; }
        int64_t rsp() const throw() { return m_context.uc_mcontext.gregs[REG_RSP]; }
    } __attribute__((aligned(16)));
    
} // jitpp

#endif
