#include "SM3_top.h"

SM3_top::SM3_top(sc_core::sc_module_name name)
:sc_module(name),
 clk_signal("clk_signal", SM3_p_cycle_config, SC_NS, 0.5, 0, SC_NS, false),
 SM3_core("SM3_core"),
 SM3_Socket_bport0("SM3_Socket_bport0", 0x50002fff, SM3_p_cycle_config)
{
    SM3_core.clk_in(clk_signal);
    SM3_core.rst_in(rst_signal);
    SM3_core.SM3_rst_in(SM3_rst_signal);
    SM3_core.SM3_opt_in(SM3_opt_signal);
    SM3_core.message_in(message_signal);
    SM3_core.V_ready_out(V_ready_signal);
    SM3_core.V_out(V_signal);

    SM3_Socket_bport0.clk_in(clk_signal);
    SM3_Socket_bport0.rst_in(rst_signal);
    SM3_Socket_bport0.SM3_rst_out(SM3_rst_signal);
    SM3_Socket_bport0.SM3_opt_out(SM3_opt_signal);
    SM3_Socket_bport0.message_out(message_signal);
    SM3_Socket_bport0.V_ready_in(V_ready_signal);
    SM3_Socket_bport0.V_in(V_signal);

    SC_THREAD(SM3_init);
}

void SM3_top::SM3_init(){
    rst_signal.write(false);
    wait(20, SC_NS);
    rst_signal.write(true);

    wait(10, SC_NS);
}
