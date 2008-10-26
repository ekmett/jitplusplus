#ifndef INCLUDED_JITPP_EXCEPTIONS_H
#define INCLUDED_JITPP_EXCEPTIONS_H

#include <exception>

namespace jitpp { 

    class interpreter_exception : public std::exception { 
    public:
        virtual bool fatal() const throw() = 0;
	virtual const char * what() const throw() = 0;
    };

    // we haven't yet added support for this opcode.
    class unsupported_opcode_exception : public interpreter_exception { 
    public:
        bool fatal() const throw() { return false; } 
	const char * what() const throw();
    };

    // we are parsing an invalid opcode. this is probably a bug, but it can be handled by the main program
    class invalid_opcode_exception : public interpreter_exception { 
    public:
        bool fatal() const throw() { return false; } 
	const char * what() const throw();
    };

    // we are parsing a valid opcode that we will probably never support
    class uninterpretable_opcode_exception : public interpreter_exception { 
    public:
        bool fatal() const throw() { return false; } 
	const char * what() const throw();
    };

    // something we never expected to see has happened
    class interpreter_logic_error : public interpreter_exception { 
    public:
        virtual bool fatal() const throw() { return true; } 
	virtual const char * what() const throw();
    };
}

#endif
