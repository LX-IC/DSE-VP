#ifndef SM4_TOP_H
#define SM4_TOP_H

#include "systemc.h"
#include "SM4_REG.h"
#include "SM4_Socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;
using namespace tlm;
using namespace sc_core;

class SM4_top:public sc_module{
    public:
        SC_HAS_PROCESS(SM4_top);

        sc_clock clk_signal;

        sc_signal<bool> rst_signal;//all reset
        sc_signal<sc_uint<32> > data_opt_signal;//used in control_proc() to control each process
        sc_signal<sc_biguint<128> > data_signal0;//key or data

        sc_signal<sc_uint<32> > data_state_signal;//register state from SM4_CORE
        sc_signal<sc_biguint<128> > data_signal1;//used to debug

        void SM4_REG_init();

        SM4_REG SM4_REG_top;

        SM4_Socket SM4_Socket_bport1;

        SM4_top(sc_core::sc_module_name name);
};

#endif
