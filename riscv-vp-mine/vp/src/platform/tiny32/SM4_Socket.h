#ifndef SM4_Socket_H
#define SM4_Socket_H

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "systemc.h"
#include <iostream>

#include "core/rv32/libtlmpwt.h"//libtlmpwt
#include "platform/pwt/pwt_module.hpp"//libtlmpwt

using namespace std;
using namespace tlm;
using namespace sc_core;

// class SM4_Socket:public sc_module{
class SM4_Socket:public sc_module, public PwtModule{//libtlmpwt
    public:
        sc_in<bool> clk_in;
        sc_in<bool> rst_in;//all reset
        sc_out<sc_uint<32> > data_opt_out;//used in control_proc() to control each process
        sc_out<sc_biguint<128> > data_out;//key or data

        sc_in<sc_uint<32> > data_state_in;//register state from SM4_CORE
        sc_in<sc_biguint<128> > data_in;//used to debug

        sc_signal<sc_uint<32> > control_in_reg;
        sc_signal<sc_uint<32> > x_in_reg[4];
        sc_signal<sc_biguint<128> > x_in;

        uint32_t state_out_reg;
        uint32_t y_out_reg[4];

        void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);

        void SM4_read(uint32_t addr, uint32_t *data);
        void SM4_write(uint32_t addr, uint32_t data);

        tlm_utils::simple_target_socket<SM4_Socket> socket;

        SM4_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle);

    private:
        uint32_t p_Max_address;
        uint32_t SM4_p_cycle;
};

#endif
