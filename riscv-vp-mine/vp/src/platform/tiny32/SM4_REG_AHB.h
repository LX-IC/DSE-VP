/*///////////////////////////////////////////
* AHB-Lite SM4 register address:
*
* addr_x_in_0 = 32'h50003000;
* addr_x_in_1 = 32'h50003004;
* addr_x_in_2 = 32'h50003008;
* addr_x_in_3 = 32'h5000300c;
*
* addr_y_out_0 = 32'h50003010
* addr_y_out_1 = 32'h50003014;
* addr_y_out_2 = 32'h50003018;
* addr_y_out_3 = 32'h5000301c;
*
* addr_control = 32'h50003020;
* addr_state   = 32'h50003024;
*
*In the program, the low 12 bits are selected as the discriminant flag bit (range(11,0))
*///////////////////////////////////////////
#ifndef SM4_REG_AHB_H
#define SM4_REG_AHB_H

#include "systemc.h"
#include "SM4_REG.h"

//Slave
SC_MODULE(SM4_REG_AHB){
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

    sc_signal<sc_uint<12> > SM4_addr;
    sc_signal<sc_uint<3> > HSIZE_reg;

    sc_signal<sc_uint<32> > control_signal;
    sc_signal<sc_uint<32> > state_signal;

    sc_signal<sc_uint<32> > x_in[4];
    sc_signal<sc_biguint<128> > data_in;

    sc_signal<sc_uint<32> > y_out[4];
    sc_signal<sc_biguint<128> > data_out;
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
    void assign_data_in();//process and input data into SM4_REG_CORE

    void assign_data_out();//get and process data from SM4_REG_CORE
    void data_output();//process all the data
    void assign_HRDATA();//output data (S to M)

    SM4_REG *SM4_REG_CORE;

    // sc_trace_file* trace_file;

    SC_CTOR(SM4_REG_AHB){
        SM4_REG_CORE = new SM4_REG("SM4_REG0");
        SM4_REG_CORE->clk_in(HCLK);
        SM4_REG_CORE->rst_in(HRESETn);
        SM4_REG_CORE->data_opt_in(control_signal);
        SM4_REG_CORE->data_in(data_in);
        SM4_REG_CORE->data_state_out(state_signal);
        SM4_REG_CORE->data_out(data_out);

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
        sensitive << x_in[0] << x_in[1] << x_in[2] << x_in[3];

        SC_METHOD(assign_data_out);
        sensitive << data_out;

        SC_METHOD(data_output);
        sensitive << HCLK.pos();
        sensitive << HRESETn.neg();

        SC_METHOD(assign_HRDATA);
        sensitive << hrdata;
/*
        trace_file = sc_create_vcd_trace_file("SM4_REG_AHB");

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

        sc_trace(trace_file, SM4_addr, "SM4_addr");
        sc_trace(trace_file, HSIZE_reg, "HSIZE_reg");

        sc_trace(trace_file, control_signal, "control_signal");
        sc_trace(trace_file, state_signal, "state_signal");

        sc_trace(trace_file, x_in[0], "x_in0");
        sc_trace(trace_file, x_in[1], "x_in1");
        sc_trace(trace_file, x_in[2], "x_in2");
        sc_trace(trace_file, x_in[3], "x_in3");
        sc_trace(trace_file, data_in, "data_in");

        sc_trace(trace_file, y_out[0], "y_out0");
        sc_trace(trace_file, y_out[1], "y_out1");
        sc_trace(trace_file, y_out[2], "y_out2");
        sc_trace(trace_file, y_out[3], "y_out3");
        sc_trace(trace_file, data_out, "data_out");
        sc_trace(trace_file, hrdata, "hrdata");
*/
    }
};

#endif
