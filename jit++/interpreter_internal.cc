#include <jit++/interpreter_internal.h>
#include <jit++/exceptions.h>

static const char * os64_reg_names[16] = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
static const char * os32_reg_names[16] = {"eax","ecx","edx","ebx","esp","ebp","esi","edi","r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"};
static const char * os16_reg_names[16] = {"ax","cx","dx","bx","sp","bp","si","di","r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w"};
static const char * byte_reg_names_rex[16] = {"al","cl","dl","bl","spl","bpl","sil","dil","r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"};
static const char * byte_reg_names_norex[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};

namespace jitpp { 
    const char * os64::reg_name(int r) { return os64_reg_names[r]; }
    const char * os32::reg_name(int r) { return os32_reg_names[r]; }
    const char * os16::reg_name(int r) { return os16_reg_names[r]; }
    const char * byte_reg_name (int r, bool has_rex) { 
        return has_rex ? byte_reg_names_rex[r] : byte_reg_names_norex[r];
    }

    void illegal() { throw invalid_opcode_exception(); }
    void unsupported() { throw unsupported_opcode_exception(); }
    void uninterpretable() { throw uninterpretable_opcode_exception(); }
    void logic_error() { throw interpreter_logic_error(); }
} // namespace jitpp

