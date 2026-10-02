#ifndef INCLUDED_JITPP_INTERPRETING_BASE_H
#define INCLUDED_JITPP_INTERPRETING_BASE_H

#include <jit++/interpreter.h>

namespace jitpp {

    template <typename T> inline const char * interpreter::reg_name(int r) const {
    typedef typename os<T>::value os;
    return os::reg_name(r);
    }
    template <> inline const char * interpreter::reg_name<int8_t>(int r) const {
        return byte_reg_name(r,has_rex());
    }



    // TODO: make illegal and logic_error into cold calls?





    static const char * reg_spaces = "                   ";
    static const char * mem_spaces = "                  ";

    template <> inline int64_t interpreter::get_reg<>(int r) const { return m_reg[r]; }
    template <> inline int32_t interpreter::get_reg<>(int r) const { return m_reg[r]; }
    template <> inline int16_t interpreter::get_reg<>(int r) const { return m_reg[r]; }
    template <> inline int8_t interpreter::get_reg<>(int r) const {
    int8_t result = has_rex()
        ?  *reinterpret_cast<const int8_t*>(m_reg + r)
            : *(reinterpret_cast<const int8_t*>(m_reg + (r & 3)) + (((r & 4) != 0) ? 1 : 0));
    VLOG(1) << reg_spaces << reg_name<int8_t>(r) << " -> " << std::hex << (int64_t)result;
    return result;
    }

    template <> inline void interpreter::set_reg<>(int r, int64_t v) {
    VLOG(1) << reg_spaces << reg_name<int64_t>(r) << " := " << std::hex << (int64_t)v;
    m_reg[r] = v;
    }
    template <> inline void interpreter::set_reg<>(int r, int32_t v) {
    // zero extended, not sign extended!
    VLOG(1) << reg_spaces << reg_name<int32_t>(r) << " := " << std::hex << (int64_t)v;
    m_reg[r] = (uint64_t)(uint32_t)v;
    }
    template <> inline void interpreter::set_reg<>(int r, int16_t v) {
    VLOG(1) << reg_spaces << reg_name<int16_t>(r) << " := " << std::hex << (int64_t)v;
    *reinterpret_cast<int16_t*>(m_reg + r) = v;
    }
    template <> inline void interpreter::set_reg<>(int r, int8_t v) {
    VLOG(1) << reg_spaces << reg_name<int8_t>(r) << " := " << std::hex << (int64_t)v;
        if (has_rex())
        *reinterpret_cast<int8_t*>(m_reg + r) = v;
        else
        *(reinterpret_cast<int8_t*>(m_reg + (r & 3)) + (r & 4 != 0 ? 1 : 0)) = v;
    }

    // M (r/m field of mod R/M byte selects a memory operand (mod == 3)
    template <typename T> inline T interpreter::M() const {
    int64_t addr = mem();
        T result = *reinterpret_cast<T*>(addr);
    VLOG(1) << mem_spaces << "*" << std::hex << addr << " -> " << std::hex << (int64_t)result;
    return result;
    }

    template <typename T> inline void interpreter::M(T v) {
    int64_t addr = mem();
    VLOG(1) << mem_spaces << "*" << std::hex << addr << " := " << std::hex << (int64_t)v;
        *reinterpret_cast<T*>(addr) = v;
    }

    // G (reg field of mod R/M byte selects a general register)
    template <typename T> inline T interpreter::G() const{
        return get_reg<T>(reg);
    }
    template <typename T> inline void interpreter::G(T v) {
        set_reg<T>(reg,v);
    }

    // R (r/m field of mod R/M byte selects a general register)
    template <typename T> inline T interpreter::R() const {
        return get_reg<T>(rm);
    }
    template <typename T> inline void interpreter::R(T v) {
        set_reg<T>(rm,v);
    }

    // E (r/m follows opcode and specifies operand, either memory or register)
    template <typename T> inline T interpreter::E() const {
            return mod == 3 ? R<T>() : M<T>();
    }
    template <typename T> inline void interpreter::E(T v) {
        if (mod == 3) R<T>(v);
        else M<T>(v);
    }

    template <typename T> void interpreter::push(T v) {
        rsp() -= sizeof(T);
        *reinterpret_cast<T*>(rsp()) = v;
    }

    template <typename T> T interpreter::pop() {
        T result = *reinterpret_cast<T*>(rsp());
        rsp() += sizeof(T);
        return result;
    }

    inline int8_t & interpreter::ah() { return reinterpret_cast<int8_t*>(m_reg)[1]; }
    inline int8_t interpreter::ah() const { return reinterpret_cast<const int8_t*>(m_reg)[1]; }

} // namespace jitpp

#endif // INCLUDED_JITPP_INTERPRETING_BASE_H
