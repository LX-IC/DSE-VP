#ifndef RISCV_ISA_MEMORY_H
#define RISCV_ISA_MEMORY_H

#include <stdint.h>
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>

#include "bus.h"
#include "load_if.h"

#include <tlm_utils/simple_target_socket.h>
#include <systemc>

#include "core/rv32/libtlmpwt.h"//libtlmpwt
#include "platform/pwt/pwt_module.hpp"//libtlmpwt

#ifdef NVMAIN
#include "mem_trace.h"
#endif

// struct SimpleMemory : public sc_core::sc_module, public load_if {
struct SimpleMemory : public sc_core::sc_module, public load_if, public PwtModule {//libtlmpwt
	tlm_utils::simple_target_socket<SimpleMemory> tsock;

	uint8_t *data;
	uint32_t size;
	bool read_only;
	#ifdef NVMAIN
	Mem_trace mem_trace;
	#endif

	// SimpleMemory(sc_core::sc_module_name, uint32_t size, bool read_only = false)
	//     : data(new uint8_t[size]()), size(size), read_only(read_only) {
	SimpleMemory(sc_core::sc_module_name, uint32_t size, bool read_only = false)
	    : data(new uint8_t[size]()), size(size), read_only(read_only), PwtModule(this) {//libtlmpwt
		#ifdef LIBTLMPWT
		set_activity(SimpleMemory_set_activity);//libtlmpwt
		#endif
		tsock.register_b_transport(this, &SimpleMemory::transport);
		tsock.register_get_direct_mem_ptr(this, &SimpleMemory::get_direct_mem_ptr);
		tsock.register_transport_dbg(this, &SimpleMemory::transport_dbg);
	}

	~SimpleMemory(void) {
		#ifdef NVMAIN
		for (auto entry : mem_trace.mem_log) {
        	delete entry;
		}
		mem_trace.mem_log.clear();
		#endif
		delete[] data;
	}

	void load_data(const char *src, uint64_t dst_addr, size_t n) override {
		assert(dst_addr + n <= size);
		memcpy(&data[dst_addr], src, n);
	}

	void load_zero(uint64_t dst_addr, size_t n) override {
		assert(dst_addr + n <= size);
		memset(&data[dst_addr], 0, n);
	}

	void load_binary_file(const std::string &filename, unsigned addr) {
		boost::iostreams::mapped_file_source f(filename);
		assert(f.is_open());
		write_data(addr, (const uint8_t *)f.data(), f.size());
	}

	void write_data(unsigned addr, const uint8_t *src, unsigned num_bytes) {
		assert(addr + num_bytes <= size);

		#ifdef NVMAIN
		static bitset<128> w_data = 0;
		sc_core::sc_time time = sc_core::sc_time_stamp();
		sc_core::sc_time clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
        uint32_t cycle = time.value()/clock_cycle.value();
        for (uint32_t i = 0; i < num_bytes; i++) {
        	w_data = w_data << 8;
        	w_data |= src[i];
        }
        mem_trace.push_mem_log (cycle, "W", addr, w_data);
        w_data = 0;
		#endif

		memcpy(data + addr, src, num_bytes);
	}

	void read_data(unsigned addr, uint8_t *dst, unsigned num_bytes) {
		assert(addr + num_bytes <= size);

		#ifdef NVMAIN
		static bitset<128> r_data=0;
		sc_core::sc_time time = sc_core::sc_time_stamp();
		sc_core::sc_time clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
        uint32_t cycle = time.value()/clock_cycle.value();
        for (uint32_t i = 0; i < num_bytes; i++) {
            r_data = r_data << 8;
            r_data |= data[addr+i];
        }
        mem_trace.push_mem_log (cycle, "R", addr, r_data);
        r_data = 0;
		#endif

		memcpy(dst, data + addr, num_bytes);
	}

	void transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
		transport_dbg(trans);
		delay += sc_core::sc_time(10, sc_core::SC_NS);
		#ifdef LIBTLMPWT
		sc_core::sc_time clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
		add_activity(SimpleMemory_add_activity, (delay.value() / clock_cycle.value()));//libtlmpwt
		#endif
	}

	unsigned transport_dbg(tlm::tlm_generic_payload &trans) {
		tlm::tlm_command cmd = trans.get_command();
		unsigned addr = trans.get_address();
		auto *ptr = trans.get_data_ptr();
		auto len = trans.get_data_length();

		assert(addr < size);

		if (cmd == tlm::TLM_WRITE_COMMAND) {
			write_data(addr, ptr, len);
		} else if (cmd == tlm::TLM_READ_COMMAND) {
			read_data(addr, ptr, len);
		} else {
			sc_assert(false && "unsupported tlm command");
		}

		return len;
	}

	bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi &dmi) {
		(void)trans;
		dmi.set_start_address(0);
		dmi.set_end_address(size);
		dmi.set_dmi_ptr(data);
		if (read_only)
			dmi.allow_read();
		else
			dmi.allow_read_write();
		return true;
	}
};

#endif  // RISCV_ISA_MEMORY_H
