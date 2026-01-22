#ifndef MEM_TRACE_H
#define MEM_TRACE_H

#include <list>
#include <bitset>
#include <systemc>
#include <fstream>
#include <iomanip>
#include <stdint.h>

using namespace std;

typedef struct Mem_list {
    uint32_t        cycle;
    std::string        op;
    uint64_t         addr;
    std::bitset<128> data;
} mem_l;

struct Mem_trace {
    std::string path;
	std::list<mem_l*> mem_log;

    void push_mem_log(uint32_t cycle, std::string op, uint64_t addr, std::bitset<128> data) {
        mem_l* entry = new mem_l;

        entry->cycle = cycle;
        entry->op    = op;
        entry->addr  = addr;
        entry->data  = data;

        mem_log.push_back(entry);
    }

	void save_mem_trace(std::string path) {
	    std::ofstream file(path);
        if (!mem_log.empty()) {
	        for (auto entry : mem_log) {
                file << std::setw(6) << std::setfill('0') << dec << entry->cycle << " "
                << entry->op <<" "
                << "0x" << std::setw(8) << std::setfill('0') << hex << entry->addr << " "
                << entry->data << " "
                << "0" << std::endl;
            }
	    }
        else {
	        std::cout << "mem_log is empty" << std::endl;
	    }
	}
};

#endif  // MEM_TRACE_H