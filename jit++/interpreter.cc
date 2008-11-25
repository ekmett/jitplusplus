#include <jit++/interpreter.h>
#include <jit++/interpreting/impl.h>

namespace jitpp { 
    interpreter::interpreter() : impl(new interpreter_impl()) {}
    void interpreter::start() { impl->start(); } 
    void interpreter::stop() { impl->stop(); } 
    interpreter::~interpreter() { delete impl; } 
}
