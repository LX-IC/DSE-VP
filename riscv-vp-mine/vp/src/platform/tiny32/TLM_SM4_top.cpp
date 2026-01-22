#include "TLM_SM4_top.h"

TLM_SM4_top::TLM_SM4_top(sc_core::sc_module_name name)
:sc_module(name),
 clk_signal("clk_signal", 10, SC_NS, 0.5, 0, SC_NS, false),
 SM4_REG_AHB_top("SM4_REG_AHB_top"),
 AHB_Socket_bport1("AHB_Socket_bport1", 0x50003fff, p_cycle)
{
    SM4_REG_AHB_top.HCLK(clk_signal);
    SM4_REG_AHB_top.HRESETn(HRESETn_signal);
    SM4_REG_AHB_top.HWRITE(HWRITE_signal);
    SM4_REG_AHB_top.HSEL(HSEL_signal);
    SM4_REG_AHB_top.HTRANS(HTRANS_signal);
    SM4_REG_AHB_top.HSIZE(HSIZE_signal);
    SM4_REG_AHB_top.HADDR(HADDR_signal);
    SM4_REG_AHB_top.HWDATA(HWDATA_signal);
    SM4_REG_AHB_top.HREADY(HREADY_signal);
    SM4_REG_AHB_top.HRESP(HRESP_signal);
    SM4_REG_AHB_top.HRDATA(HRDATA_signal);

    AHB_Socket_bport1.HCLK(clk_signal);
    AHB_Socket_bport1.HRESETn(HRESETn_signal);
    AHB_Socket_bport1.HWRITE(HWRITE_signal);
    AHB_Socket_bport1.HSEL(HSEL_signal);
    AHB_Socket_bport1.HTRANS(HTRANS_signal);
    AHB_Socket_bport1.HSIZE(HSIZE_signal);
    AHB_Socket_bport1.HADDR(HADDR_signal);
    AHB_Socket_bport1.HWDATA(HWDATA_signal);
    AHB_Socket_bport1.HREADY(HREADY_signal);
    AHB_Socket_bport1.HRESP(HRESP_signal);
    AHB_Socket_bport1.HRDATA(HRDATA_signal);


    SC_THREAD(SM4_REG_AHB_init);
}

void TLM_SM4_top::SM4_REG_AHB_init(){
    HRESETn_signal.write(false);
    wait(20, SC_NS);
    HRESETn_signal.write(true);

    wait(10, SC_NS);
}
