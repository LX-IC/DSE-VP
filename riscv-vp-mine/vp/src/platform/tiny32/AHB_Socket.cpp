#include "AHB_Socket.h"

// AHB_Socket::AHB_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle)
// :sc_module(name),
//  p_Max_address(Max_address),
//  p_cycle(cycle),
//  socket("socket")
// {
AHB_Socket::AHB_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle)
:sc_module(name),
 p_Max_address(Max_address),
 p_cycle(cycle),
 socket("socket"),
 PwtModule(this)
{
    #ifdef LIBTLMPWT
	set_activity(AHB_set_activity);//libtlmpwt
	#endif
    assert(p_Max_address >= 0 && p_cycle >= 0);
    socket.register_b_transport(this, &AHB_Socket::b_transport);
}

void AHB_Socket::b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay){
    tlm::tlm_command cmd = trans.get_command();
    uint32_t addr = (uint32_t)trans.get_address();
    uint32_t *data = (uint32_t*)trans.get_data_ptr();

    trans.set_response_status(tlm::TLM_OK_RESPONSE);

    if(cmd == tlm::TLM_WRITE_COMMAND) {
        AHB_write(addr, *data);
    }
    else if(cmd == tlm::TLM_READ_COMMAND) {
        AHB_read(addr, data);
    }
    else trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    #ifdef LIBTLMPWT
    sc_core::sc_time clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
	add_activity(AHB_add_activity, (delay.value() / clock_cycle.value())*mul_cycle);//libtlmpwt
	#endif
}

void AHB_Socket::AHB_write(uint32_t addr, uint32_t data){
    HSEL.write("1");
    HTRANS.write("0b10");
    HSIZE.write("0b010");
    HWRITE.write(true);

    wait(2*p_cycle, SC_NS);

    HADDR.write(addr);
    wait(p_cycle, SC_NS);
    HWDATA.write(data);
}

void AHB_Socket::AHB_read(uint32_t addr, uint32_t *data){
    HSEL.write("1");
    HTRANS.write("0b10");
    HSIZE.write("0b010");
    HWRITE.write(false);

    wait(2*p_cycle, SC_NS);

    HADDR.write(addr);
    wait(p_cycle, SC_NS);
    *data = HRDATA.read().to_uint();
}

