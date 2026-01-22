#ifndef SM4_REG_H
#define SM4_REG_H

#include "systemc.h"
#include "SM4.h"

#ifdef USING_SM4_AC

SC_MODULE(SM4_REG){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;//all reset
    sc_in<sc_uint<32> > data_opt_in;//used in control_proc() to control each process
    sc_in<sc_biguint<128> > data_in;//key or data

    sc_out<sc_uint<32> > data_state_out;//register state from SM4_CORE
    sc_out<sc_biguint<128> > data_out;//used to debug

    sc_signal<bool> SM4_CORE_rst;//SM4_CORE reset
    sc_signal<bool> KeyExp_en;
    sc_signal<bool> EncDec_en;
    sc_signal<bool> mode;//true: enc false: dec
    sc_signal<sc_biguint<128> > key;//when data_in is key
    sc_signal<sc_biguint<128> > data;//when data_in is data
    sc_signal<bool> rk_ready;
    sc_signal<bool> Encdata_ready;
    sc_signal<bool> Decdata_ready;

    void control_proc();
    void state_proc();

    SM4 *SM4_CORE;

#ifdef DEBUG_SM4_REG
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM4_REG){
        SM4_CORE = new SM4("SM40");
        SM4_CORE->clk_in(clk_in),
        SM4_CORE->rst_in(SM4_CORE_rst),
        SM4_CORE->KeyExp_en_in(KeyExp_en),
        SM4_CORE->EncDec_en_in(EncDec_en),
        SM4_CORE->mode_in(mode),
        SM4_CORE->key_in(key),
        SM4_CORE->data_in(data),
        SM4_CORE->rk_ready_out(rk_ready),
        SM4_CORE->Encdata_ready_out(Encdata_ready),
        SM4_CORE->Decdata_ready_out(Decdata_ready),
        SM4_CORE->data_out(data_out);

        SC_METHOD(control_proc);
        sensitive << clk_in.pos();
        sensitive << rst_in.neg();

        SC_METHOD(state_proc);
        sensitive << SM4_CORE_rst << KeyExp_en << rk_ready << EncDec_en << mode << Encdata_ready << Decdata_ready;

#ifdef DEBUG_SM4_REG
        trace_file = sc_create_vcd_trace_file("SM4_REG");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, data_opt_in, "data_opt_in");
        sc_trace(trace_file, data_in, "data_in");
        sc_trace(trace_file, data_state_out, "data_state_out");
        sc_trace(trace_file, data_out, "data_out");
        sc_trace(trace_file, SM4_CORE_rst, "SM4_CORE_rst");
        sc_trace(trace_file, KeyExp_en, "KeyExp_en");
        sc_trace(trace_file, EncDec_en, "EncDec_en");
        sc_trace(trace_file, mode, "mode");
        sc_trace(trace_file, key, "key");
        sc_trace(trace_file, data, "data");
        sc_trace(trace_file, rk_ready, "rk_ready");
        sc_trace(trace_file, Encdata_ready, "Encdata_ready");
        sc_trace(trace_file, Decdata_ready, "Decdata_ready");
#endif

    }
};

#elif defined USING_SM4_AP

SC_MODULE(SM4_REG){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;//all reset
    sc_in<sc_uint<32> > data_opt_in;//used in control_proc() to control each process
    sc_in<sc_biguint<128> > data_in;//key or data

    sc_out<sc_uint<32> > data_state_out;//register state from SM4_CORE
    sc_out<sc_biguint<128> > data_out;//used to debug

    sc_signal<bool> SM4_CORE_rst;//SM4_CORE reset
    sc_signal<bool> KeyExp_en;
    sc_signal<bool> EncDec_en;
    sc_signal<bool> mode;//true: enc false: dec
    sc_signal<sc_biguint<128> > key;//when data_in is key
    sc_signal<sc_biguint<128> > data;//when data_in is data
    sc_signal<bool> rk_ready;
    sc_signal<bool> Encdata_ready;
    sc_signal<bool> Decdata_ready;

    void control_proc();
    void state_proc();

    SM4 *SM4_CORE;

#ifdef DEBUG_SM4_REG
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM4_REG){
        SM4_CORE = new SM4("SM40");
        SM4_CORE->clk_in(clk_in),
        SM4_CORE->rst_in(SM4_CORE_rst),
        SM4_CORE->KeyExp_en_in(KeyExp_en),
        SM4_CORE->EncDec_en_in(EncDec_en),
        SM4_CORE->mode_in(mode),
        SM4_CORE->key_in(key),
        SM4_CORE->data_in(data),
        SM4_CORE->rk_ready_out(rk_ready),
        SM4_CORE->Encdata_ready_out(Encdata_ready),
        SM4_CORE->Decdata_ready_out(Decdata_ready),
        SM4_CORE->data_out(data_out);

        SC_METHOD(control_proc);
        sensitive << clk_in.pos();
        sensitive << rst_in.neg();

        SC_METHOD(state_proc);
        sensitive << SM4_CORE_rst << KeyExp_en << rk_ready << EncDec_en << mode << Encdata_ready << Decdata_ready;

#ifdef DEBUG_SM4_REG
        trace_file = sc_create_vcd_trace_file("SM4_REG");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, data_opt_in, "data_opt_in");
        sc_trace(trace_file, data_in, "data_in");
        sc_trace(trace_file, data_state_out, "data_state_out");
        sc_trace(trace_file, data_out, "data_out");
        sc_trace(trace_file, SM4_CORE_rst, "SM4_CORE_rst");
        sc_trace(trace_file, KeyExp_en, "KeyExp_en");
        sc_trace(trace_file, EncDec_en, "EncDec_en");
        sc_trace(trace_file, mode, "mode");
        sc_trace(trace_file, key, "key");
        sc_trace(trace_file, data, "data");
        sc_trace(trace_file, rk_ready, "rk_ready");
        sc_trace(trace_file, Encdata_ready, "Encdata_ready");
        sc_trace(trace_file, Decdata_ready, "Decdata_ready");
#endif

    }
};

#elif defined USING_SM4_NO

SC_MODULE(SM4_REG){
    sc_in<bool> clk_in;
    sc_in<bool> rst_in;//all reset
    sc_in<sc_uint<32> > data_opt_in;//used in control_proc() to control each process
    sc_in<sc_biguint<128> > data_in;//key or data

    sc_out<sc_uint<32> > data_state_out;//register state from SM4_CORE
    sc_out<sc_biguint<128> > data_out;//used to debug

    sc_signal<bool> SM4_CORE_rst;//SM4_CORE reset
    sc_signal<bool> KeyExp_en;
    sc_signal<bool> EncDec_en;
    sc_signal<bool> mode;//true: enc false: dec
    sc_signal<sc_biguint<128> > key;//when data_in is key
    sc_signal<sc_biguint<128> > data;//when data_in is data
    sc_signal<bool> rk_ready;
    sc_signal<bool> Encdata_ready;
    sc_signal<bool> Decdata_ready;

    void control_proc();
    void state_proc();

    SM4 *SM4_CORE;

#ifdef DEBUG_SM4_REG
    sc_trace_file* trace_file;
#endif

    SC_CTOR(SM4_REG){
        SM4_CORE = new SM4("SM40");
        SM4_CORE->clk_in(clk_in),
        SM4_CORE->rst_in(SM4_CORE_rst),
        SM4_CORE->KeyExp_en_in(KeyExp_en),
        SM4_CORE->EncDec_en_in(EncDec_en),
        SM4_CORE->mode_in(mode),
        SM4_CORE->key_in(key),
        SM4_CORE->data_in(data),
        SM4_CORE->rk_ready_out(rk_ready),
        SM4_CORE->Encdata_ready_out(Encdata_ready),
        SM4_CORE->Decdata_ready_out(Decdata_ready),
        SM4_CORE->data_out(data_out);

        SC_METHOD(control_proc);
        sensitive << rst_in.neg() << data_opt_in;

        SC_METHOD(state_proc);
        sensitive << SM4_CORE_rst << KeyExp_en << rk_ready << EncDec_en << mode << Encdata_ready << Decdata_ready;

#ifdef DEBUG_SM4_REG
        trace_file = sc_create_vcd_trace_file("SM4_REG");

        sc_trace(trace_file, clk_in, "clk_in");
        sc_trace(trace_file, rst_in, "rst_in");
        sc_trace(trace_file, data_opt_in, "data_opt_in");
        sc_trace(trace_file, data_in, "data_in");
        sc_trace(trace_file, data_state_out, "data_state_out");
        sc_trace(trace_file, data_out, "data_out");
        sc_trace(trace_file, SM4_CORE_rst, "SM4_CORE_rst");
        sc_trace(trace_file, KeyExp_en, "KeyExp_en");
        sc_trace(trace_file, EncDec_en, "EncDec_en");
        sc_trace(trace_file, mode, "mode");
        sc_trace(trace_file, key, "key");
        sc_trace(trace_file, data, "data");
        sc_trace(trace_file, rk_ready, "rk_ready");
        sc_trace(trace_file, Encdata_ready, "Encdata_ready");
        sc_trace(trace_file, Decdata_ready, "Decdata_ready");
#endif

    }
};

#endif

#endif
