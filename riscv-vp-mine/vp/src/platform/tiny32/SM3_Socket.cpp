#include "SM3_Socket.h"

// SM3_Socket::SM3_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle)
// :sc_module(name),
//  p_Max_address(Max_address),
//  SM3_p_cycle(cycle),
//  socket("socket")
// {
SM3_Socket::SM3_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle)
:sc_module(name),
 p_Max_address(Max_address),
 SM3_p_cycle(cycle),
 socket("socket"),
 PwtModule(this)
{
    #ifdef LIBTLMPWT
	set_activity(SM3_set_activity);//libtlmpwt
	#endif
    assert(p_Max_address >= 0 && SM3_p_cycle >= 0);
    socket.register_b_transport(this, &SM3_Socket::b_transport);
}

void SM3_Socket::b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay){
    tlm::tlm_command cmd = trans.get_command();
    uint32_t addr = (uint32_t)trans.get_address();
    uint32_t *data = (uint32_t*)trans.get_data_ptr();

    trans.set_response_status(tlm::TLM_OK_RESPONSE);

    if(cmd == tlm::TLM_WRITE_COMMAND) {
        SM3_write(addr, *data);
    }
    else if(cmd == tlm::TLM_READ_COMMAND) {
        SM3_read(addr, data);
    }
    else trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    #ifdef LIBTLMPWT
    sc_core::sc_time clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
	add_activity(SM3_add_activity, (delay.value() / clock_cycle.value())*mul_cycle);//libtlmpwt
	#endif
}

/*///////////////////////////////////////////
* SM3 register address:
*
* addr_x_in_0  = 32'h50002000;
* addr_x_in_1  = 32'h50002004;
* addr_x_in_2  = 32'h50002008;
* addr_x_in_3  = 32'h5000200c;
* addr_x_in_4  = 32'h50002010;
* addr_x_in_5  = 32'h50002014;
* addr_x_in_6  = 32'h50002018;
* addr_x_in_7  = 32'h5000201c;
* addr_x_in_8  = 32'h50002020;
* addr_x_in_9  = 32'h50002024;
* addr_x_in_10 = 32'h50002028;
* addr_x_in_11 = 32'h5000202c;
* addr_x_in_12 = 32'h50002030;
* addr_x_in_13 = 32'h50002034;
* addr_x_in_14 = 32'h50002038;
* addr_x_in_15 = 32'h5000203c;
*
* addr_y_out_0 = 32'h50002040;
* addr_y_out_1 = 32'h50002044;
* addr_y_out_2 = 32'h50002048;
* addr_y_out_3 = 32'h5000204c;
* addr_y_out_4 = 32'h50002050;
* addr_y_out_5 = 32'h50002054;
* addr_y_out_6 = 32'h50002058;
* addr_y_out_7 = 32'h5000205c;
*
* addr_SM3_rst = 32'h50002060;
* addr_SM3_opt = 32'h50002064;
* addr_V_ready = 32'h50002068;
*
*///////////////////////////////////////////

void SM3_Socket::SM3_write(uint32_t addr, uint32_t data){

#ifdef DEBUG_SM3_Socket
    cout << hex << "write_addr: " << addr << endl;
    cout << hex << "write_data: " << data << endl;
#endif

    if(addr == 0x60)
        SM3_rst_in_reg = data;
    else if(addr == 0x64)
        SM3_opt_in_reg = data;
    else if(addr == 0x00)
        x_in_reg[ 0]   = data;
    else if(addr == 0x04)
        x_in_reg[ 1]   = data;
    else if(addr == 0x08)
        x_in_reg[ 2]   = data;
    else if(addr == 0x0c)
        x_in_reg[ 3]   = data;
    else if(addr == 0x10)
        x_in_reg[ 4]   = data;
    else if(addr == 0x14)
        x_in_reg[ 5]   = data;
    else if(addr == 0x18)
        x_in_reg[ 6]   = data;
    else if(addr == 0x1c)
        x_in_reg[ 7]   = data;
    else if(addr == 0x20)
        x_in_reg[ 8]   = data;
    else if(addr == 0x24)
        x_in_reg[ 9]   = data;
    else if(addr == 0x28)
        x_in_reg[10]   = data;
    else if(addr == 0x2c)
        x_in_reg[11]   = data;
    else if(addr == 0x30)
        x_in_reg[12]   = data;
    else if(addr == 0x34)
        x_in_reg[13]   = data;
    else if(addr == 0x38)
        x_in_reg[14]   = data;
    else if(addr == 0x3c)
        x_in_reg[15]   = data;
    else return;

    wait(SM3_p_cycle, SC_NS);

#ifdef DEBUG_SM3_Socket
    cout << hex << "SM3_rst_in_reg: " << SM3_rst_in_reg << endl;
    cout << hex << "SM3_opt_in_reg: " << SM3_opt_in_reg << endl;
    cout << hex << "x_in_reg[ 0]  : " << x_in_reg[ 0]   << endl;
    cout << hex << "x_in_reg[ 1]  : " << x_in_reg[ 1]   << endl;
    cout << hex << "x_in_reg[ 2]  : " << x_in_reg[ 2]   << endl;
    cout << hex << "x_in_reg[ 3]  : " << x_in_reg[ 3]   << endl;
    cout << hex << "x_in_reg[ 4]  : " << x_in_reg[ 4]   << endl;
    cout << hex << "x_in_reg[ 5]  : " << x_in_reg[ 5]   << endl;
    cout << hex << "x_in_reg[ 6]  : " << x_in_reg[ 6]   << endl;
    cout << hex << "x_in_reg[ 7]  : " << x_in_reg[ 7]   << endl;
    cout << hex << "x_in_reg[ 8]  : " << x_in_reg[ 8]   << endl;
    cout << hex << "x_in_reg[ 9]  : " << x_in_reg[ 9]   << endl;
    cout << hex << "x_in_reg[10]  : " << x_in_reg[10]   << endl;
    cout << hex << "x_in_reg[11]  : " << x_in_reg[11]   << endl;
    cout << hex << "x_in_reg[12]  : " << x_in_reg[12]   << endl;
    cout << hex << "x_in_reg[13]  : " << x_in_reg[13]   << endl;
    cout << hex << "x_in_reg[14]  : " << x_in_reg[14]   << endl;
    cout << hex << "x_in_reg[15]  : " << x_in_reg[15]   << endl;
#endif

    x_in = (x_in_reg[ 0], x_in_reg[ 1], x_in_reg[ 2], x_in_reg[ 3],
            x_in_reg[ 4], x_in_reg[ 5], x_in_reg[ 6], x_in_reg[ 7],
            x_in_reg[ 8], x_in_reg[ 9], x_in_reg[10], x_in_reg[11],
            x_in_reg[12], x_in_reg[13], x_in_reg[14], x_in_reg[15]);

    SM3_rst_out.write(SM3_rst_in_reg);
    SM3_opt_out.write(SM3_opt_in_reg);
    message_out.write(x_in);
}

void SM3_Socket::SM3_read(uint32_t addr, uint32_t *data){

    V_ready_out_reg = V_ready_in.read().to_uint();

    y_out_reg[0] = V_in.read().range(255,224).to_uint();
    y_out_reg[1] = V_in.read().range(223,192).to_uint();
    y_out_reg[2] = V_in.read().range(191,160).to_uint();
    y_out_reg[3] = V_in.read().range(159,128).to_uint();
    y_out_reg[4] = V_in.read().range(127, 96).to_uint();
    y_out_reg[5] = V_in.read().range( 95, 64).to_uint();
    y_out_reg[6] = V_in.read().range( 63, 32).to_uint();
    y_out_reg[7] = V_in.read().range( 31,  0).to_uint();

#ifdef DEBUG_SM3_Socket
    cout << hex << "V_ready_out_reg: " << V_ready_out_reg << endl;
    cout << hex << "y_out_reg[0]   : " << y_out_reg[0]    << endl;
    cout << hex << "y_out_reg[1]   : " << y_out_reg[1]    << endl;
    cout << hex << "y_out_reg[2]   : " << y_out_reg[2]    << endl;
    cout << hex << "y_out_reg[3]   : " << y_out_reg[3]    << endl;
    cout << hex << "y_out_reg[4]   : " << y_out_reg[4]    << endl;
    cout << hex << "y_out_reg[5]   : " << y_out_reg[5]    << endl;
    cout << hex << "y_out_reg[6]   : " << y_out_reg[6]    << endl;
    cout << hex << "y_out_reg[7]   : " << y_out_reg[7]    << endl;
#endif

    if(addr == 0x68)
        *data = V_ready_out_reg;
    else if(addr == 0x40)
        *data = y_out_reg[0];
    else if(addr == 0x44)
        *data = y_out_reg[1];
    else if(addr == 0x48)
        *data = y_out_reg[2];
    else if(addr == 0x4c)
        *data = y_out_reg[3];
    else if(addr == 0x50)
        *data = y_out_reg[4];
    else if(addr == 0x54)
        *data = y_out_reg[5];
    else if(addr == 0x58)
        *data = y_out_reg[6];
    else if(addr == 0x5c)
        *data = y_out_reg[7];
    else return;

#ifdef DEBUG_SM3_Socket
    cout << hex << "read_addr: " <<  addr << endl;
    cout << hex << "read_data: " << *data << endl;
#endif

}

