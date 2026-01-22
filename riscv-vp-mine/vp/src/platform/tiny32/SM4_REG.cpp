#include "SM4_REG.h"

void SM4_REG::control_proc(){
    if(rst_in.read() == false) {
        SM4_CORE_rst = 0;
        KeyExp_en = false;
        EncDec_en = false;
        key = 0;
        data = 0;
    }
    else if(rst_in.read() == true && data_opt_in.read()== 0) {
        SM4_CORE_rst = 0;
    }
    else if(rst_in.read() == true && data_opt_in.read()== 1) {
        SM4_CORE_rst = 1;
        key = data_in.read();
        KeyExp_en = true;
    }
    else if(rst_in.read() == true && data_opt_in.read()== 2) {
        data = data_in.read();
        mode = true;
        EncDec_en = true;
    }
    else if(rst_in.read() == true && data_opt_in.read()== 3) {
        data = data_in.read();
        mode = false;
        EncDec_en = true;
    }
    else return;

}

void SM4_REG::state_proc(){
    sc_uint<32> data_state;

    if(SM4_CORE_rst == 0) {
        data_state = 0;
        data_state_out.write(data_state);
    }
    else if(SM4_CORE_rst == 1 && KeyExp_en == true && rk_ready == false) {
        data_state = 1;
        data_state_out.write(data_state);
    }
    else if(SM4_CORE_rst == 1 && KeyExp_en == true && rk_ready == true) {
        KeyExp_en = false;
        data_state = 2;
        data_state_out.write(data_state);
    }
    else if(SM4_CORE_rst == 1 && EncDec_en == true && rk_ready == true && mode == true && Encdata_ready == false) {
        data_state = 3;
        data_state_out.write(data_state);
    }
    else if(SM4_CORE_rst == 1 && EncDec_en == true && rk_ready == true && mode == true && Encdata_ready == true ) {
        EncDec_en = false;
        data_state = 4;
        data_state_out.write(data_state);
    }
    else if(SM4_CORE_rst == 1 && EncDec_en == true && rk_ready == true && mode == false && Encdata_ready == true && Decdata_ready == false) {
        data_state = 5;
        data_state_out.write(data_state);
    }
    else if(SM4_CORE_rst == 1 && EncDec_en == true && rk_ready == true && mode == false && Encdata_ready == true && Decdata_ready == true) {
        EncDec_en = false;
        data_state = 6;
        data_state_out.write(data_state);
    }
    else return;
}

