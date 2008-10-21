#include <jit++/exception.h>
#include <udis86.h>

namespace jitpp { 
    unsupported_opcode_exception::unsupported_opcode_exception(const void * rip) : m_rip(rip) { 
	m_what[0] = '\0'; 
    } 
    const char * unsupported_opcode_exception::what() const {
	if (m_what[0] != '\0') return m_what;
	ud_t ud;
	ud_init(&ud);
	ud_set_input_buffer(&ud,reinterpret_cast<uint8_t*>(const_cast<void *>(m_rip)),15);
	ud_set_mode(&ud,64);
	ud_set_pc(&ud,reinterpret_cast<uint64_t>(m_rip));
	ud_set_syntax(&ud,UD_SYN_ATT);
	size_t bytes = ud_disassemble(&ud);
	// we can also emit relevant register contents since this is mid trace
	snprintf(m_what,max_length,"%018p: %-32.32s %s\n", (uint64_t)ud_insn_off(&ud),ud_insn_hex(&ud),ud_insn_asm(&ud));
	m_what[max_length-1] = '\0';
	return m_what;
    }
} // namespace jitpp
