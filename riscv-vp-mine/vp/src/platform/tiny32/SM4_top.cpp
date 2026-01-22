#include "SM4_top.h"

SM4_top::SM4_top(sc_core::sc_module_name name)
:sc_module(name),
 clk_signal("clk_signal", SM4_p_cycle_config, SC_NS, 0.5, 0, SC_NS, false),
 SM4_REG_top("SM4_REG_top"),
 SM4_Socket_bport1("SM4_Socket_bport1", 0x50003fff, SM4_p_cycle_config)
{
    SM4_REG_top.clk_in(clk_signal);
    SM4_REG_top.rst_in(rst_signal);
    SM4_REG_top.data_opt_in(data_opt_signal);
    SM4_REG_top.data_in(data_signal0);
    SM4_REG_top.data_state_out(data_state_signal);
    SM4_REG_top.data_out(data_signal1);

    SM4_Socket_bport1.clk_in(clk_signal);
    SM4_Socket_bport1.rst_in(rst_signal);
    SM4_Socket_bport1.data_opt_out(data_opt_signal);
    SM4_Socket_bport1.data_out(data_signal0);
    SM4_Socket_bport1.data_state_in(data_state_signal);
    SM4_Socket_bport1.data_in(data_signal1);

    SC_THREAD(SM4_REG_init);
}

void SM4_top::SM4_REG_init(){
    rst_signal.write(false);
    wait(20, SC_NS);
    rst_signal.write(true);

    wait(10, SC_NS);
}
