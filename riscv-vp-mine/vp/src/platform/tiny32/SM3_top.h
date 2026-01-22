#ifndef SM3_TOP_H
#define SM3_TOP_H

#include "systemc.h"
#include "SM3.h"
#include "SM3_Socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;
using namespace tlm;
using namespace sc_core;

class SM3_top:public sc_module{
    public:
        SC_HAS_PROCESS(SM3_top);

        sc_clock clk_signal;

        sc_signal<bool> rst_signal;//all reset
        sc_signal<sc_uint<32> > SM3_rst_signal;//reset all
        sc_signal<sc_uint<32> > SM3_opt_signal;//SM3 start
        sc_signal<sc_biguint<512> > message_signal;

        sc_signal<sc_uint<32> > V_ready_signal;//which V is ready to out
        sc_signal<sc_biguint<256> > V_signal;

        void SM3_init();

        SM3 SM3_core;

        SM3_Socket SM3_Socket_bport0;

        SM3_top(sc_core::sc_module_name name);
};

#endif
