#include <jit++/exceptions.h>

namespace jitpp { 
    const char * unsupported_opcode_exception::what() const throw() { return "unsupported opcode"; }
    const char * invalid_opcode_exception::what() const throw() { return "invalid opcode"; }
    const char * uninterpretable_opcode_exception::what() const throw() { return "uninterpretable opcode"; }
    const char * interpreter_logic_error::what() const throw() { return "interpreter logic error"; }
}
