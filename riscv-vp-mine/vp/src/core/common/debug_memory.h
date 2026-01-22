#ifndef RISCV_ISA_DEBUG_MEMORY_H
#define RISCV_ISA_DEBUG_MEMORY_H

#include <string>
#include <type_traits>

#include <tlm_utils/simple_initiator_socket.h>
#include <systemc>

#include "core_defs.h"
#include "trap.h"

#include "core/rv32/libtlmpwt.h"//libtlmpwt
#include "platform/pwt/pwt_module.hpp"//libtlmpwt

// struct DebugMemoryInterface : public sc_core::sc_module {
struct DebugMemoryInterface : public sc_core::sc_module, public PwtModule {//libtlmpwt
	tlm_utils::simple_initiator_socket<DebugMemoryInterface> isock;

	// DebugMemoryInterface(sc_core::sc_module_name) {}
	DebugMemoryInterface(sc_core::sc_module_name) : PwtModule(this) {//libtlmpwt
		#ifdef LIBTLMPWT
		set_activity(DebugMemoryInterface_set_activity);//libtlmpwt
		#endif
	}

	unsigned _do_dbg_transaction(tlm::tlm_command cmd, uint64_t addr, uint8_t *data, unsigned num_bytes);

	std::string read_memory(uint64_t start, unsigned nbytes);

	void write_memory(uint64_t start, unsigned nbytes, const std::string &data);
};

#endif  // RISCV_ISA_GDB_STUB_H
