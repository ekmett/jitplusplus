#ifndef INCLUDED_JITPP_EXCEPTION_H
#define INCLUDED_JITPP_EXCEPTION_H

#include <sys/types.h> // size_t
#include <exception>   // std::exception

namespace jitpp { 
    class tracer_exception : public std::exception {
    public:
        virtual ~tracer_exception() throw() {}
        virtual const char * what() const throw() = 0;
    };

    // what() displays the exception
    class unsupported_opcode_exception : public tracer_exception {
    public:
        unsupported_opcode_exception(const void * rip) throw();
        static const size_t max_length = 160;
        const char * what() const throw();
    private:
        const void * m_rip;
        mutable char m_what[max_length];
    };
}

#endif

