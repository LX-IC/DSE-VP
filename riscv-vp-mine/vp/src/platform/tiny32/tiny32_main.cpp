#include <cstdlib>
#include <ctime>

#include "core/common/real_clint.h"
#include "elf_loader.h"
#include "debug_memory.h"
#include "iss.h"
#include "mem.h"
#include "memory.h"
#include "syscall.h"
#include "platform/common/options.h"

#include "define.h"

#ifdef USING_AHB
#include "TLM_SM3_top.h"
#include "TLM_SM4_top.h"
#else
#include "SM3_top.h"
#include "SM4_top.h"
#endif

#include "gdb-mc/gdb_server.h"
#include "gdb-mc/gdb_runner.h"

#include <boost/io/ios_state.hpp>
#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>

#include "core/rv32/libtlmpwt.h"//libtlmpwt
#include "platform/pwt/pwt_module.hpp"//libtlmpwt
#include "platform/pwt/atmi_wrapper.hpp"//libtlmpwt
#include "platform/pwt/atmi_wrapper.cpp"//libtlmpwt
#include "platform/utilities/parameter.hpp"//libtlmpwt

using namespace rv32;
namespace po = boost::program_options;

#ifdef LIBTLMPWT
extern int qt_start(int argc, char *argv[], const sc_time &simtime, AtmiWrapper *atmi_wrapper);//libtlmpwt

ParameterInt simtime("sim-time",
                     "Duration of simualtions (in simualted ms)"
                     ", -1=infinite",
                     -1);//libtlmpwt
#endif

struct TinyOptions : public Options {
public:
	typedef unsigned int addr_t;

	addr_t mem_size = 1024 * 1024 * 32;  // 32 MB ram, to place it before the CLINT and run the base examples (assume
	                                     // memory start at zero) without modifications
	addr_t mem_start_addr = 0x00000000;
	addr_t mem_end_addr = mem_start_addr + mem_size - 1;
	addr_t clint_start_addr = 0x02000000;
	addr_t clint_end_addr = 0x0200ffff;
	addr_t sys_start_addr = 0x02010000;
	addr_t sys_end_addr = 0x020103ff;

	#ifdef USING_SM3
	addr_t SM3_start_addr = 0x50002000;
	addr_t SM3_end_addr = 0x50002068;
	#elif defined USING_SM4
	addr_t SM4_start_addr = 0x50003000;
	addr_t SM4_end_addr = 0x50003024;
	#elif defined USING_SM3_SM4
	addr_t SM3_start_addr = 0x50002000;
	addr_t SM3_end_addr = 0x50002068;
	addr_t SM4_start_addr = 0x50003000;
	addr_t SM4_end_addr = 0x50003024;
	#endif

	bool quiet = false;
	bool use_E_base_isa = false;
	bool libtlmpwt = false;
	bool nvmain = false;

	//DMI
	#ifdef DMI
	bool use_instr_dmi = true;
	bool use_data_dmi = true;
	#endif

	// TQ
	#ifdef TQ
	unsigned int tlm_global_quantum = TQ_value;
	#endif

	TinyOptions(void) {
		// clang-format off
		add_options()
			("quiet", po::bool_switch(&quiet), "do not output register values on exit")
			("memory-start", po::value<unsigned int>(&mem_start_addr), "set memory start address")
			("memory-size", po::value<unsigned int>(&mem_size), "set memory size")
			("use-E-base-isa", po::bool_switch(&use_E_base_isa), "use the E instead of the I integer base ISA")
			("libtlmpwt", po::bool_switch(&libtlmpwt), "use libtlmpwt")
			("nvmain", po::bool_switch(&nvmain), "use nvmain");
        	// clang-format on
        }

	void parse(int argc, char **argv) override {
		Options::parse(argc, argv);
		mem_end_addr = mem_start_addr + mem_size - 1;
	}
};

int sc_main(int argc, char **argv) {
	TinyOptions opt;
	opt.parse(argc, argv);

	std::srand(std::time(nullptr));  // use current time as seed for random generator

	tlm::tlm_global_quantum::instance().set(sc_core::sc_time(opt.tlm_global_quantum, sc_core::SC_NS));

	ISS core("Core", 0, opt.use_E_base_isa);//libtlmpwt
    MMU mmu("MMU", core);//libtlmpwt
	CombinedMemoryInterface core_mem_if("MemoryInterface0", core, &mmu);
	SimpleMemory mem("SimpleMemory", opt.mem_size);
	ELFLoader loader("ELFLoader", opt.input_program.c_str());

	#ifdef USING_SM3
	SimpleBus<2, 4> bus("SimpleBus");
	#elif defined USING_SM4
	SimpleBus<2, 4> bus("SimpleBus");
	#elif defined USING_SM3_SM4
	SimpleBus<2, 5> bus("SimpleBus");
	#else
	SimpleBus<2, 3> bus("SimpleBus");
	#endif

	SyscallHandler sys("SyscallHandler");

	#ifdef USING_SM3

	#ifdef USING_AHB
	TLM_SM3_top SM3("SM3");
	#else
	SM3_top SM3("SM3");
    #endif

	#elif defined USING_SM4

    #ifdef USING_AHB
	TLM_SM4_top SM4("SM4");
	#else
	SM4_top SM4("SM4");
    #endif

	#elif defined USING_SM3_SM4

	#ifdef USING_AHB
	TLM_SM3_top SM3("SM3");
	TLM_SM4_top SM4("SM4");
	#else
	SM3_top SM3("SM3");
	SM4_top SM4("SM4");
    #endif

	#endif

	DebugMemoryInterface dbg_if("DebugMemoryInterface");

	std::vector<clint_interrupt_target*> clint_targets {&core};
	RealCLINT clint("CLINT", clint_targets);

	#ifdef DMI
	DMI_PWT dmi_pwt("DMI_PWT");
	MemoryDMI dmi = MemoryDMI::create_start_size_mapping(mem.data, opt.mem_start_addr, mem.size);
	#ifdef CACHE_TIMING
	InstrMemoryProxyWithHiFive1CacheTiming instr_mem(dmi);
	instr_mem.pipeline = &core.pipeline;
	#else
	InstrMemoryProxy instr_mem("InstrMemoryProxy", dmi, core);
	#endif
	#endif

	std::shared_ptr<BusLock> bus_lock = std::make_shared<BusLock>();
	core_mem_if.bus_lock = bus_lock;

	instr_memory_if *instr_mem_if = &core_mem_if;
	data_memory_if *data_mem_if = &core_mem_if;

	#ifdef DMI
	if (opt.use_instr_dmi)
		instr_mem_if = &instr_mem;
	if (opt.use_data_dmi) {
		core_mem_if.dmi_ranges.emplace_back(dmi);
	}
	#endif

	loader.load_executable_image(mem, mem.size, opt.mem_start_addr);
	core.init(instr_mem_if, data_mem_if, &clint, loader.get_entrypoint(), rv32_align_address(opt.mem_end_addr));
	sys.init(mem.data, opt.mem_start_addr, loader.get_heap_addr());
	sys.register_core(&core);

	if (opt.intercept_syscalls)
		core.sys = &sys;

	// setup port mapping
	bus.ports[0] = new PortMapping(opt.mem_start_addr, opt.mem_end_addr);
	bus.ports[1] = new PortMapping(opt.clint_start_addr, opt.clint_end_addr);
	bus.ports[2] = new PortMapping(opt.sys_start_addr, opt.sys_end_addr);

	#ifdef USING_SM3
	bus.ports[3] = new PortMapping(opt.SM3_start_addr, opt.SM3_end_addr);
	#elif defined USING_SM4
	bus.ports[3] = new PortMapping(opt.SM4_start_addr, opt.SM4_end_addr);
	#elif defined USING_SM3_SM4
	bus.ports[3] = new PortMapping(opt.SM3_start_addr, opt.SM3_end_addr);
	bus.ports[4] = new PortMapping(opt.SM4_start_addr, opt.SM4_end_addr);
	#endif

	// connect TLM sockets
	core_mem_if.isock.bind(bus.tsocks[0]);
	dbg_if.isock.bind(bus.tsocks[1]);
	bus.isocks[0].bind(mem.tsock);
	bus.isocks[1].bind(clint.tsock);
	bus.isocks[2].bind(sys.tsock);

	#ifdef USING_SM3

	#ifdef USING_AHB
	bus.isocks[3].bind(SM3.AHB_Socket_bport0.socket);
	#else
	bus.isocks[3].bind(SM3.SM3_Socket_bport0.socket);
	#endif

	#elif defined USING_SM4

    #ifdef USING_AHB
	bus.isocks[3].bind(SM4.AHB_Socket_bport1.socket);
	#else
	bus.isocks[3].bind(SM4.SM4_Socket_bport1.socket);
	#endif

	#elif defined USING_SM3_SM4

	#ifdef USING_AHB
	bus.isocks[3].bind(SM3.AHB_Socket_bport0.socket);
	bus.isocks[4].bind(SM4.AHB_Socket_bport1.socket);
	#else
	bus.isocks[3].bind(SM3.SM3_Socket_bport0.socket);
	bus.isocks[4].bind(SM4.SM4_Socket_bport1.socket);
	#endif

	#endif

	// switch for printing instructions
	core.trace = opt.trace_mode;

	std::vector<debug_target_if *> threads;
	threads.push_back(&core);

	if (opt.use_debug_runner) {
		auto server = new GDBServer("GDBServer", threads, &dbg_if, opt.debug_port);
		new GDBServerRunner("GDBRunner", server, &core);
	} else {
		new DirectCoreRunner(core);
	}

	//libtlmpwt
	if (opt.libtlmpwt) {
		#ifdef LIBTLMPWT
		#ifdef DMI
		AtmiWrapper *atmi_wrapper(new AtmiWrapper("ATMI_WRAPPER", 15));
		atmi_wrapper->config_rectangle(&dmi_pwt,               0.5, 0.2, 0.6, 0.3);
		atmi_wrapper->config_rectangle(&instr_mem,             0.6, 0.2, 0.7, 0.3);
		#else
		AtmiWrapper *atmi_wrapper(new AtmiWrapper("ATMI_WRAPPER", 13));
		#endif
		atmi_wrapper->config_rectangle(&core,                    0,   0, 0.1, 0.1);
		atmi_wrapper->config_rectangle(&core.sm3_nice,         0.1,   0, 0.2, 0.1);
		atmi_wrapper->config_rectangle(&core.sm4_nice,         0.2,   0, 0.3, 0.1);
		atmi_wrapper->config_rectangle(&mmu,                   0.3,   0, 0.4, 0.1);
		atmi_wrapper->config_rectangle(&core_mem_if,           0.4,   0, 0.5, 0.1);
		atmi_wrapper->config_rectangle(&mem,                   0.5,   0, 0.6, 0.1);
		atmi_wrapper->config_rectangle(&loader,                0.6,   0, 0.7, 0.1);
		atmi_wrapper->config_rectangle(&bus,                     0, 0.1, 0.7, 0.2);
		atmi_wrapper->config_rectangle(&sys,                     0, 0.2, 0.1, 0.3);
		#ifdef USING_AHB
		atmi_wrapper->config_rectangle(&SM3.AHB_Socket_bport0, 0.1, 0.2, 0.2, 0.3);
		atmi_wrapper->config_rectangle(&SM4.AHB_Socket_bport1, 0.2, 0.2, 0.3, 0.3);
		#else
		atmi_wrapper->config_rectangle(&SM3.SM3_Socket_bport0, 0.1, 0.2, 0.2, 0.3);
		atmi_wrapper->config_rectangle(&SM4.SM4_Socket_bport1, 0.2, 0.2, 0.3, 0.3);
		#endif
		atmi_wrapper->config_rectangle(&dbg_if,                0.3, 0.2, 0.4, 0.3);
		atmi_wrapper->config_rectangle(&clint,                 0.4, 0.2, 0.5, 0.3);
		qt_start(argc, argv, sc_time(simtime.get(), SC_MS), atmi_wrapper);
		#endif
	}
	//libtlmpwt

	if (opt.quiet)
		 sc_core::sc_report_handler::set_verbosity_level(sc_core::SC_NONE);

	sc_core::sc_start();
	if (!opt.quiet) {
		core.show();
	}

	if (opt.nvmain) {
		#ifdef NVMAIN
		std::string path = "nvmain.trace";
		mem.mem_trace.save_mem_trace(path);
		#endif
	}

	return 0;
}
