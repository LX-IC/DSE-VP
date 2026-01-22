#ifndef SM3_Socket_H
#define SM3_Socket_H

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "systemc.h"
#include <iostream>

#include "core/rv32/libtlmpwt.h"//libtlmpwt
#include "platform/pwt/pwt_module.hpp"//libtlmpwt

using namespace std;
using namespace tlm;
using namespace sc_core;

// class SM3_Socket:public sc_module{
class SM3_Socket:public sc_module, public PwtModule{//libtlmpwt
    public:
        sc_in<bool> clk_in;
        sc_in<bool> rst_in;//all reset
        sc_out<sc_uint<32> > SM3_rst_out;//reset all
        sc_out<sc_uint<32> > SM3_opt_out;//SM3 start
        sc_out<sc_biguint<512> > message_out;

        sc_in<sc_uint<32> > V_ready_in;//which V is ready to out
        sc_in<sc_biguint<256> > V_in;

        sc_signal<sc_uint<32> > SM3_rst_in_reg;
        sc_signal<sc_uint<32> > SM3_opt_in_reg;
        sc_signal<sc_uint<32> > x_in_reg[16];
        sc_signal<sc_biguint<512> > x_in;

        uint32_t V_ready_out_reg;
        uint32_t y_out_reg[8];

        void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);

        void SM3_read(uint32_t addr, uint32_t *data);
        void SM3_write(uint32_t addr, uint32_t data);

        tlm_utils::simple_target_socket<SM3_Socket> socket;

        SM3_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle);

    private:
        uint32_t p_Max_address;
        uint32_t SM3_p_cycle;
};

#endif
