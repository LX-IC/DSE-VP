#ifndef TLM_SM3_TOP_H
#define TLM_SM3_TOP_H

#include "systemc.h"
#include "SM3_REG_AHB.h"
#include "AHB_Socket.h"
#include "tlm_utils/simple_target_socket.h"

#define p_cycle 10

using namespace std;
using namespace tlm;
using namespace sc_core;

class TLM_SM3_top:public sc_module{
    public:
        SC_HAS_PROCESS(TLM_SM3_top);
        //sc_in<bool> HMASTLOOK;//Latch (init)

        //sc_in<sc_uint<3> > HBURST;//Mass Tranfer (4,6,8 Burst) (init)
        //sc_in<sc_uint<4> > HPROT;//Protection and Control (init)

        sc_clock clk_signal;

        sc_signal<bool> HRESETn_signal;//Reset .neg
        sc_signal<bool> HWRITE_signal;//Direction of Transfer (1:Write 0:Read)
        sc_signal<bool> HSEL_signal;//Slave Select Signal
        sc_signal<sc_uint<2> > HTRANS_signal;//Transmission Type (IDLE,BUSY,NONSEQ,SEQ)
        sc_signal<sc_uint<3> > HSIZE_signal;//Transmission Bandwidth (Max:1024)
        sc_signal<sc_uint<32> > HADDR_signal;//Address Bus
        sc_signal<sc_uint<32> > HWDATA_signal;//Write Data Bus (Master to Slave)

        sc_signal<bool> HREADY_signal;//Transmission Complete (1:Slave Output Transmission Ends 0:Slave Transmission Period Needs to be Extended)
        sc_signal<bool> HRESP_signal;//Response after Transmission (Slave to Master OKAY,ERROR,RETRY,SPLIT)
        sc_signal<sc_uint<32> > HRDATA_signal;//Read Data Bus (Slave to Master)

        void SM3_REG_AHB_init();

        SM3_REG_AHB SM3_REG_AHB_top;

        AHB_Socket AHB_Socket_bport0;

        TLM_SM3_top(sc_core::sc_module_name name);
};

#endif
