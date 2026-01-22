#include "TLM_SM3_top.h"

TLM_SM3_top::TLM_SM3_top(sc_core::sc_module_name name)
:sc_module(name),
 clk_signal("clk_signal", 10, SC_NS, 0.5, 0, SC_NS, false),
 SM3_REG_AHB_top("SM3_REG_AHB_top"),
 AHB_Socket_bport0("AHB_Socket_bport0", 0x50002fff, p_cycle)
{
    SM3_REG_AHB_top.HCLK(clk_signal);
    SM3_REG_AHB_top.HRESETn(HRESETn_signal);
    SM3_REG_AHB_top.HWRITE(HWRITE_signal);
    SM3_REG_AHB_top.HSEL(HSEL_signal);
    SM3_REG_AHB_top.HTRANS(HTRANS_signal);
    SM3_REG_AHB_top.HSIZE(HSIZE_signal);
    SM3_REG_AHB_top.HADDR(HADDR_signal);
    SM3_REG_AHB_top.HWDATA(HWDATA_signal);
    SM3_REG_AHB_top.HREADY(HREADY_signal);
    SM3_REG_AHB_top.HRESP(HRESP_signal);
    SM3_REG_AHB_top.HRDATA(HRDATA_signal);

    AHB_Socket_bport0.HCLK(clk_signal);
    AHB_Socket_bport0.HRESETn(HRESETn_signal);
    AHB_Socket_bport0.HWRITE(HWRITE_signal);
    AHB_Socket_bport0.HSEL(HSEL_signal);
    AHB_Socket_bport0.HTRANS(HTRANS_signal);
    AHB_Socket_bport0.HSIZE(HSIZE_signal);
    AHB_Socket_bport0.HADDR(HADDR_signal);
    AHB_Socket_bport0.HWDATA(HWDATA_signal);
    AHB_Socket_bport0.HREADY(HREADY_signal);
    AHB_Socket_bport0.HRESP(HRESP_signal);
    AHB_Socket_bport0.HRDATA(HRDATA_signal);


    SC_THREAD(SM3_REG_AHB_init);
}

void TLM_SM3_top::SM3_REG_AHB_init(){
    HRESETn_signal.write(false);
    wait(20, SC_NS);
    HRESETn_signal.write(true);

    wait(10, SC_NS);
}
