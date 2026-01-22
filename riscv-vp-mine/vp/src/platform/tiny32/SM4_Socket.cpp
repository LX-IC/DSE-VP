#include "SM4_Socket.h"

// SM4_Socket::SM4_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle)
// :sc_module(name),
//  p_Max_address(Max_address),
//  SM4_p_cycle(cycle),
//  socket("socket")
// {
SM4_Socket::SM4_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle)
:sc_module(name),
 p_Max_address(Max_address),
 SM4_p_cycle(cycle),
 socket("socket"),
 PwtModule(this)
{
    #ifdef LIBTLMPWT
	set_activity(SM4_set_activity);//libtlmpwt
	#endif
    assert(p_Max_address >= 0 && SM4_p_cycle >= 0);
    socket.register_b_transport(this, &SM4_Socket::b_transport);
}

void SM4_Socket::b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay){
    tlm::tlm_command cmd = trans.get_command();
    uint32_t addr = (uint32_t)trans.get_address();
    uint32_t *data = (uint32_t*)trans.get_data_ptr();

    trans.set_response_status(tlm::TLM_OK_RESPONSE);

    if(cmd == tlm::TLM_WRITE_COMMAND) {
        SM4_write(addr, *data);
    }
    else if(cmd == tlm::TLM_READ_COMMAND) {
        SM4_read(addr, data);
    }
    else trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    #ifdef LIBTLMPWT
    sc_core::sc_time clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
	add_activity(SM4_add_activity, (delay.value() / clock_cycle.value())*mul_cycle);//libtlmpwt
	#endif
}

/*///////////////////////////////////////////
* SM4 register address:
*
* addr_x_in_0  = 32'h50003000;
* addr_x_in_1  = 32'h50003004;
* addr_x_in_2  = 32'h50003008;
* addr_x_in_3  = 32'h5000300c;
*
* addr_y_out_0 = 32'h50003010;
* addr_y_out_1 = 32'h50003014;
* addr_y_out_2 = 32'h50003018;
* addr_y_out_3 = 32'h5000301c;
*
* addr_control = 32'h50003020;
* addr_state   = 32'h50003024;
*
*///////////////////////////////////////////

void SM4_Socket::SM4_write(uint32_t addr, uint32_t data){

#ifdef DEBUG_SM4_Socket
    cout << hex << "write_addr: " << addr << endl;
    cout << hex << "write_data: " << data << endl;
#endif

    if(addr == 0x20)
        control_in_reg = data;
    else if(addr == 0x00)
        x_in_reg[0]    = data;
    else if(addr == 0x04)
        x_in_reg[1]    = data;
    else if(addr == 0x08)
        x_in_reg[2]    = data;
    else if(addr == 0x0c)
        x_in_reg[3]    = data;
    else return;

    wait(SM4_p_cycle, SC_NS);

#ifdef DEBUG_SM4_Socket
    cout << hex << "control_in_reg: " << control_in_reg << endl;
    cout << hex << "x_in_reg[0]   : " << x_in_reg[0]    << endl;
    cout << hex << "x_in_reg[1]   : " << x_in_reg[1]    << endl;
    cout << hex << "x_in_reg[2]   : " << x_in_reg[2]    << endl;
    cout << hex << "x_in_reg[3]   : " << x_in_reg[3]    << endl;
#endif

    x_in = (x_in_reg[0], x_in_reg[1], x_in_reg[2], x_in_reg[3]);

    data_opt_out.write(control_in_reg);
    data_out.write(x_in);
}

void SM4_Socket::SM4_read(uint32_t addr, uint32_t *data){

    state_out_reg = data_state_in.read().to_uint();

    y_out_reg[0]  = data_in.read().range(127,96).to_uint();
    y_out_reg[1]  = data_in.read().range( 95,64).to_uint();
    y_out_reg[2]  = data_in.read().range( 63,32).to_uint();
    y_out_reg[3]  = data_in.read().range( 31, 0).to_uint();

#ifdef DEBUG_SM4_Socket
    cout << hex << "state_out_reg: " << state_out_reg << endl;
    cout << hex << "y_out_reg[0] : " << y_out_reg[0]  << endl;
    cout << hex << "y_out_reg[1] : " << y_out_reg[1]  << endl;
    cout << hex << "y_out_reg[2] : " << y_out_reg[2]  << endl;
    cout << hex << "y_out_reg[3] : " << y_out_reg[3]  << endl;
#endif

    if(addr == 0x24)
        *data = state_out_reg;
    else if(addr == 0x10)
        *data = y_out_reg[0];
    else if(addr == 0x14)
        *data = y_out_reg[1];
    else if(addr == 0x18)
        *data = y_out_reg[2];
    else if(addr == 0x1c)
        *data = y_out_reg[3];
    else return;

#ifdef DEBUG_SM4_Socket
    cout << hex << "read_addr: " <<  addr << endl;
    cout << hex << "read_data: " << *data << endl;
#endif

}

