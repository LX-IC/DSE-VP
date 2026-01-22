/*///////////////////////////////////////////
* AHB-Lite SM3 register address:
*
* addr_x_in_0  = 32'h50002000;
* addr_x_in_1  = 32'h50002004;
* addr_x_in_2  = 32'h50002008;
* addr_x_in_3  = 32'h5000200c;
* addr_x_in_4  = 32'h50002010;
* addr_x_in_5  = 32'h50002014;
* addr_x_in_6  = 32'h50002018;
* addr_x_in_7  = 32'h5000201c;
* addr_x_in_8  = 32'h50002020;
* addr_x_in_9  = 32'h50002024;
* addr_x_in_10 = 32'h50002028;
* addr_x_in_11 = 32'h5000202c;
* addr_x_in_12 = 32'h50002030;
* addr_x_in_13 = 32'h50002034;
* addr_x_in_14 = 32'h50002038;
* addr_x_in_15 = 32'h5000203c;
*
* addr_y_out_0 = 32'h50002040;
* addr_y_out_1 = 32'h50002044;
* addr_y_out_2 = 32'h50002048;
* addr_y_out_3 = 32'h5000204c;
* addr_y_out_4 = 32'h50002050;
* addr_y_out_5 = 32'h50002054;
* addr_y_out_6 = 32'h50002058;
* addr_y_out_7 = 32'h5000205c;
*
* addr_SM3_rst = 32'h50002060;
* addr_SM3_opt = 32'h50002064;
* addr_V_ready = 32'h50002068;
*
*In the program, the low 12 bits are selected as the discriminant flag bit (range(11,0))
*///////////////////////////////////////////
#ifndef SM3_REG_AHB_H
#define SM3_REG_AHB_H

#include "systemc.h"
#include "SM3.h"

//Slave
SC_MODULE(SM3_REG_AHB){
    //Arbitration Signals
    //sc_in<bool> HMASTLOOK;//Latch (Unused)
    //General Signals
    //sc_in<sc_uint<3> > HBURST;//Mass Tranfer (4,6,8 Burst) (Unused)
    //sc_in<sc_uint<4> > HPROT;//Protection and Control (Unused)

    sc_in<bool> HCLK;//Bus Clock .pos
    sc_in<bool> HRESETn;//Reset .neg
    sc_in<bool> HWRITE;//Direction of Transfer (1:Write 0:Read)
    sc_in<bool> HSEL;//Slave Select Signal
    sc_in<sc_uint<2> > HTRANS;//Transmission Type (IDLE,BUSY,NONSEQ,SEQ)
    sc_in<sc_uint<3> > HSIZE;//Transmission Bandwidth (Max:1024)
    sc_in<sc_uint<32> > HADDR;//Address Bus
    sc_in<sc_uint<32> > HWDATA;//Write Data Bus (Master to Slave)

    sc_out<bool> HREADY;//Transmission Complete (1:Slave Output Transmission Ends 0:Slave Transmission Period Needs to be Extended)
    sc_out<bool> HRESP;//Response after Transmission (Slave to Master OKAY,ERROR,RETRY,SPLIT)
    sc_out<sc_uint<32> > HRDATA;//Read Data Bus (Slave to Master)

    sc_signal<bool> ahb_trans_valid;
    sc_signal<bool> ahb_write_en;
    sc_signal<bool> ahb_read_en;
    sc_signal<bool> write_en;
    sc_signal<bool> read_en;

    sc_signal<sc_uint<12> > SM3_addr;
    sc_signal<sc_uint<3> > HSIZE_reg;

    sc_signal<sc_uint<32> > SM3_rst_signal, SM3_opt_signal;
    sc_signal<sc_uint<32> > V_ready_signal;

    sc_signal<sc_uint<32> > x_in[16];
    sc_signal<sc_biguint<512> > data_in;

    sc_signal<sc_uint<32> > y_out[8];
    sc_signal<sc_biguint<256> > data_out;
    sc_signal<sc_uint<32> > hrdata;

    //The name "assign_" represents no clock signal
    void assign_ahb_init();//initialize HRESP and HREADY
    void assign_ahb_trans_valid();//determine whether the data can be transmitted
    void ahb_addr();//get the address and data size

    void assign_ahb_write_en();//determine whether the data can be written
    void assign_ahb_read_en();//determine whether the data can be read
    void write_enable();//with reset signal
    void read_enable();//with reset signal

    void control_signal_input();//get and input control signal (M to S)

    void data_input();//get data (M to S)
    void assign_data_in();//process and input data into SM3_REG_CORE

    void assign_data_out();//get and process data from SM3_REG_CORE
    void data_output();//process all the data
    void assign_HRDATA();//output data (S to M)

    SM3 *SM3_CORE;

    // sc_trace_file* trace_file;

    SC_CTOR(SM3_REG_AHB){
        SM3_CORE = new SM3("SM30");
        SM3_CORE->clk_in(HCLK);
        SM3_CORE->rst_in(HRESETn);
        SM3_CORE->SM3_rst_in(SM3_rst_signal);
        SM3_CORE->SM3_opt_in(SM3_opt_signal);
        SM3_CORE->message_in(data_in);
        SM3_CORE->V_ready_out(V_ready_signal);
        SM3_CORE->V_out(data_out);

        SC_METHOD(assign_ahb_init);

        SC_METHOD(assign_ahb_trans_valid);
        sensitive << HSEL << HREADY << HTRANS;

        SC_METHOD(ahb_addr);
        sensitive << HCLK.pos();
        sensitive << HRESETn.neg();

        SC_METHOD(assign_ahb_write_en);
        sensitive << HWRITE << ahb_trans_valid;

        SC_METHOD(assign_ahb_read_en);
        sensitive << HWRITE << ahb_trans_valid;

        SC_METHOD(write_enable);
        sensitive << HCLK.pos();
        sensitive << HRESETn.neg();

        SC_METHOD(read_enable);
        sensitive << HCLK.pos();
        sensitive << HRESETn.neg();

        SC_METHOD(control_signal_input);
        sensitive << HCLK.pos();
        sensitive << HRESETn.neg();

        SC_METHOD(data_input);
        sensitive << HCLK.pos();
        sensitive << HRESETn.neg();

        SC_METHOD(assign_data_in);
        sensitive << x_in[0] << x_in[1] << x_in[2] << x_in[3] << x_in[4] << x_in[5] << x_in[6] << x_in[7] << x_in[8] << x_in[9] << x_in[10] << x_in[11] << x_in[12] << x_in[13] << x_in[14] << x_in[15];

        SC_METHOD(assign_data_out);
        sensitive << data_out;

        SC_METHOD(data_output);
        sensitive << HCLK.pos();
        sensitive << HRESETn.neg();

        SC_METHOD(assign_HRDATA);
        sensitive << hrdata;
/*
	    trace_file = sc_create_vcd_trace_file("SM3_REG_AHB");

        sc_trace(trace_file, HCLK, "HCLK");
        sc_trace(trace_file, HRESETn, "HRESETn");
        sc_trace(trace_file, HWRITE, "HWRITE");
        sc_trace(trace_file, HSEL, "HSEL");
        sc_trace(trace_file, HTRANS, "HTRANS");
        sc_trace(trace_file, HSIZE, "HSIZE");
        sc_trace(trace_file, HADDR, "HADDR");
        sc_trace(trace_file, HWDATA, "HWDATA");
        sc_trace(trace_file, HREADY, "HREADY");
        sc_trace(trace_file, HRESP, "HRESP");
        sc_trace(trace_file, HRDATA, "HRDATA");

        sc_trace(trace_file, ahb_trans_valid, "ahb_trans_valid");
        sc_trace(trace_file, ahb_write_en, "ahb_write_en");
        sc_trace(trace_file, ahb_read_en, "ahb_read_en");
        sc_trace(trace_file, write_en, "write_en");
        sc_trace(trace_file, read_en, "read_en");

        sc_trace(trace_file, SM3_addr, "SM3_addr");
        sc_trace(trace_file, HSIZE_reg, "HSIZE_reg");

        sc_trace(trace_file, SM3_rst_signal, "SM3_rst_signal");
        sc_trace(trace_file, SM3_opt_signal, "SM3_opt_signal");
        sc_trace(trace_file, V_ready_signal, "V_ready_signal");

        sc_trace(trace_file, x_in[0], "x_in0");
        sc_trace(trace_file, x_in[1], "x_in1");
        sc_trace(trace_file, x_in[2], "x_in2");
        sc_trace(trace_file, x_in[3], "x_in3");
        sc_trace(trace_file, x_in[4], "x_in4");
        sc_trace(trace_file, x_in[5], "x_in5");
        sc_trace(trace_file, x_in[6], "x_in6");
        sc_trace(trace_file, x_in[7], "x_in7");
        sc_trace(trace_file, x_in[8], "x_in8");
        sc_trace(trace_file, x_in[9], "x_in9");
        sc_trace(trace_file, x_in[10], "x_in10");
        sc_trace(trace_file, x_in[11], "x_in11");
        sc_trace(trace_file, x_in[12], "x_in12");
        sc_trace(trace_file, x_in[13], "x_in13");
        sc_trace(trace_file, x_in[14], "x_in14");
        sc_trace(trace_file, x_in[15], "x_in15");
        sc_trace(trace_file, data_in, "data_in");

        sc_trace(trace_file, y_out[0], "y_out0");
        sc_trace(trace_file, y_out[1], "y_out1");
        sc_trace(trace_file, y_out[2], "y_out2");
        sc_trace(trace_file, y_out[3], "y_out3");
        sc_trace(trace_file, y_out[4], "y_out4");
        sc_trace(trace_file, y_out[5], "y_out5");
        sc_trace(trace_file, y_out[6], "y_out6");
        sc_trace(trace_file, y_out[7], "y_out7");
        sc_trace(trace_file, data_out, "data_out");
        sc_trace(trace_file, hrdata, "hrdata");
*/
    }
};

#endif
