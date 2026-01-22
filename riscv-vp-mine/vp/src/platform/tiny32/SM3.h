#ifndef SM3_H
#define SM3_H

#include "define.h"
#include "systemc.h"

#ifdef USING_SM3_AC

SC_MODULE(SM3){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;//reset all
    sc_in<sc_uint<32> > SM3_rst_in;//reset all
    sc_in<sc_uint<32> > SM3_opt_in;//SM3 start
    sc_in<sc_biguint<512> > message_in;

    sc_out<sc_uint<32> > V_ready_out;//which V is ready to out
    sc_out<sc_biguint<256> > V_out;

    void SM3_hash();

#ifdef DEBUG_SM3
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM3){
        SC_METHOD(SM3_hash);
        sensitive << clk_in.pos();
        sensitive << rst_in.neg();

#ifdef DEBUG_SM3
        trace_file = sc_create_vcd_trace_file("SM3");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, SM3_rst_in, "SM3_rst_in");
        sc_trace(trace_file, SM3_opt_in, "SM3_opt_in");
        sc_trace(trace_file, message_in, "message_in");
        sc_trace(trace_file, V_ready_out, "V_ready_out");
        sc_trace(trace_file, V_out, "V_out");
#endif

    }
};

#elif defined USING_SM3_AP

SC_MODULE(SM3){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;//reset all
    sc_in<sc_uint<32> > SM3_rst_in;//reset all
    sc_in<sc_uint<32> > SM3_opt_in;//SM3 start
    sc_in<sc_biguint<512> > message_in;

    sc_out<sc_uint<32> > V_ready_out;//which V is ready to out
    sc_out<sc_biguint<256> > V_out;

    void SM3_hash();

#ifdef DEBUG_SM3
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM3){
        SC_METHOD(SM3_hash);
        sensitive << clk_in.pos();
        sensitive << rst_in.neg();

#ifdef DEBUG_SM3
        trace_file = sc_create_vcd_trace_file("SM3");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, SM3_rst_in, "SM3_rst_in");
        sc_trace(trace_file, SM3_opt_in, "SM3_opt_in");
        sc_trace(trace_file, message_in, "message_in");
        sc_trace(trace_file, V_ready_out, "V_ready_out");
        sc_trace(trace_file, V_out, "V_out");
#endif

    }
};

#elif defined USING_SM3_NO

SC_MODULE(SM3){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;//reset all
    sc_in<sc_uint<32> > SM3_rst_in;//reset all
    sc_in<sc_uint<32> > SM3_opt_in;//SM3 start
    sc_in<sc_biguint<512> > message_in;

    sc_out<sc_uint<32> > V_ready_out;//which V is ready to out
    sc_out<sc_biguint<256> > V_out;

    void SM3_hash();

#ifdef DEBUG_SM3
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM3){
        SC_METHOD(SM3_hash);
        sensitive << rst_in.neg() << SM3_rst_in << SM3_opt_in;

#ifdef DEBUG_SM3
        trace_file = sc_create_vcd_trace_file("SM3");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, SM3_rst_in, "SM3_rst_in");
        sc_trace(trace_file, SM3_opt_in, "SM3_opt_in");
        sc_trace(trace_file, message_in, "message_in");
        sc_trace(trace_file, V_ready_out, "V_ready_out");
        sc_trace(trace_file, V_out, "V_out");
#endif

    }
};

#endif

#endif
