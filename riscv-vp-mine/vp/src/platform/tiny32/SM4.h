#ifndef SM4_H
#define SM4_H

#include "define.h"
#include "systemc.h"

#ifdef USING_SM4_AC

SC_MODULE(SM4){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;
    sc_in<bool> KeyExp_en_in;
    sc_in<bool> EncDec_en_in;
    sc_in<bool> mode_in;//true: enc false: dec
    sc_in<sc_biguint<128> > key_in;
    sc_in<sc_biguint<128> > data_in;

    sc_out<bool> rk_ready_out;
    sc_out<bool> Encdata_ready_out;
    sc_out<bool> Decdata_ready_out;
    sc_out<sc_biguint<128> > data_out;

    void KeyExp();
    void EncDec();

#ifdef DEBUG_SM4
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM4){
        SC_METHOD(KeyExp);
        sensitive << clk_in.pos();
        sensitive << rst_in.neg();

        SC_METHOD(EncDec);
        sensitive << clk_in.pos();
        sensitive << rst_in.neg();

#ifdef DEBUG_SM4
        trace_file = sc_create_vcd_trace_file("SM4");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, KeyExp_en_in, "KeyExp_en_in");
        sc_trace(trace_file, EncDec_en_in, "EncDec_en_in");
        sc_trace(trace_file, mode_in, "mode_in");
        sc_trace(trace_file, key_in, "key_in");
        sc_trace(trace_file, data_in, "data_in");
        sc_trace(trace_file, rk_ready_out, "rk_ready_out");
        sc_trace(trace_file, Encdata_ready_out, "Encdata_ready_out");
        sc_trace(trace_file, Decdata_ready_out, "Decdata_ready_out");
        sc_trace(trace_file, data_out, "data_out");
#endif

    }
};

#elif defined USING_SM4_AP

SC_MODULE(SM4){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;
    sc_in<bool> KeyExp_en_in;
    sc_in<bool> EncDec_en_in;
    sc_in<bool> mode_in;//true: enc false: dec
    sc_in<sc_biguint<128> > key_in;
    sc_in<sc_biguint<128> > data_in;

    sc_out<bool> rk_ready_out;
    sc_out<bool> Encdata_ready_out;
    sc_out<bool> Decdata_ready_out;
    sc_out<sc_biguint<128> > data_out;

    void KeyExp();
    void EncDec();

#ifdef DEBUG_SM4
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM4){
        SC_METHOD(KeyExp);
        sensitive << clk_in.pos();
        sensitive << rst_in.neg();

        SC_METHOD(EncDec);
        sensitive << clk_in.pos();
        sensitive << rst_in.neg();

#ifdef DEBUG_SM4
        trace_file = sc_create_vcd_trace_file("SM4");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, KeyExp_en_in, "KeyExp_en_in");
        sc_trace(trace_file, EncDec_en_in, "EncDec_en_in");
        sc_trace(trace_file, mode_in, "mode_in");
        sc_trace(trace_file, key_in, "key_in");
        sc_trace(trace_file, data_in, "data_in");
        sc_trace(trace_file, rk_ready_out, "rk_ready_out");
        sc_trace(trace_file, Encdata_ready_out, "Encdata_ready_out");
        sc_trace(trace_file, Decdata_ready_out, "Decdata_ready_out");
        sc_trace(trace_file, data_out, "data_out");
#endif

    }
};

#elif defined USING_SM4_NO

SC_MODULE(SM4){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;
    sc_in<bool> KeyExp_en_in;
    sc_in<bool> EncDec_en_in;
    sc_in<bool> mode_in;//true: enc false: dec
    sc_in<sc_biguint<128> > key_in;
    sc_in<sc_biguint<128> > data_in;

    sc_out<bool> rk_ready_out;
    sc_out<bool> Encdata_ready_out;
    sc_out<bool> Decdata_ready_out;
    sc_out<sc_biguint<128> > data_out;

    void KeyExp();
    void EncDec();

#ifdef DEBUG_SM4
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM4){
        SC_METHOD(KeyExp);
        sensitive << KeyExp_en_in;

        SC_METHOD(EncDec);
        sensitive << EncDec_en_in;

#ifdef DEBUG_SM4
        trace_file = sc_create_vcd_trace_file("SM4");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, KeyExp_en_in, "KeyExp_en_in");
        sc_trace(trace_file, EncDec_en_in, "EncDec_en_in");
        sc_trace(trace_file, mode_in, "mode_in");
        sc_trace(trace_file, key_in, "key_in");
        sc_trace(trace_file, data_in, "data_in");
        sc_trace(trace_file, rk_ready_out, "rk_ready_out");
        sc_trace(trace_file, Encdata_ready_out, "Encdata_ready_out");
        sc_trace(trace_file, Decdata_ready_out, "Decdata_ready_out");
        sc_trace(trace_file, data_out, "data_out");
#endif

    }
};

#endif

#endif
