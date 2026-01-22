#ifndef AHB_Socket_H
#define AHB_Socket_H

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "systemc.h"

#include "core/rv32/libtlmpwt.h"//libtlmpwt
#include "platform/pwt/pwt_module.hpp"//libtlmpwt

using namespace std;
using namespace tlm;
using namespace sc_core;

// class AHB_Socket:public sc_module{
class AHB_Socket:public sc_module, public PwtModule{//libtlmpwt
    public:
        //sc_in<bool> HMASTLOOK;//Latch (init)

        //sc_in<sc_uint<3> > HBURST;//Mass Tranfer (4,6,8 Burst) (init)
        //sc_in<sc_uint<4> > HPROT;//Protection and Control (init)

        sc_in<bool> HCLK;//Bus Clock .pos
        sc_out<bool> HRESETn;//Reset .neg
        sc_out<bool> HWRITE;//Direction of Transfer (1:Write 0:Read)
        sc_out<bool> HSEL;//Slave Select Signal
        sc_out<sc_uint<2> > HTRANS;//Transmission Type (IDLE,BUSY,NONSEQ,SEQ)
        sc_out<sc_uint<3> > HSIZE;//Transmission Bandwidth (Max:1024)
        sc_out<sc_uint<32> > HADDR;//Address Bus
        sc_out<sc_uint<32> > HWDATA;//Write Data Bus (Master to Slave)

        sc_in<bool> HREADY;//Transmission Complete (1:Slave Output Transmission Ends 0:Slave Transmission Period Needs to be Extended)
        sc_in<bool> HRESP;//Response after Transmission (Slave to Master OKAY,ERROR,RETRY,SPLIT)
        sc_in<sc_uint<32> > HRDATA;//Read Data Bus (Slave to Master)

        void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);

        void AHB_read(uint32_t addr, uint32_t *data);
        void AHB_write(uint32_t addr, uint32_t data);

        tlm_utils::simple_target_socket<AHB_Socket> socket;

        AHB_Socket(sc_core::sc_module_name name, uint32_t Max_address, uint32_t cycle);

    private:
        uint32_t p_Max_address;
        uint32_t p_cycle;
};

#endif
