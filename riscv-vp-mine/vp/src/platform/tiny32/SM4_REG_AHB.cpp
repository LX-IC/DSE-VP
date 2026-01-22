#include "SM4_REG_AHB.h"

void SM4_REG_AHB::assign_ahb_init(){
    HRESP = 0;
    HREADY = 1;
}

void SM4_REG_AHB::assign_ahb_trans_valid(){
    ahb_trans_valid = HSEL.read() && HREADY && (HTRANS.read() == 0b10);
}

void SM4_REG_AHB::ahb_addr(){
    if(HRESETn.read() == false) {
        SM4_addr = 0xfff;
        HSIZE_reg = 0b000;
    }
    else if(HRESETn.read() == true && ahb_trans_valid) {
        SM4_addr = HADDR.read().range(11,0);
        HSIZE_reg = HSIZE.read();
    }
    else {
        SM4_addr = 0xfff;
        HSIZE_reg = 0b000;
    }
}

void SM4_REG_AHB::assign_ahb_write_en(){
    ahb_write_en = ahb_trans_valid && HWRITE.read();
}

void SM4_REG_AHB::assign_ahb_read_en(){
    ahb_read_en = ahb_trans_valid && (!HWRITE.read());
}

void SM4_REG_AHB::write_enable(){
    if(HRESETn.read() == false) {
        write_en = 0;
    }
    else if(HRESETn.read() == true && ahb_write_en) {
        write_en = 1;
    }
    else {
        write_en = 0;
    }
}

void SM4_REG_AHB::read_enable(){
    if(HRESETn.read() == false) {
        read_en = 0;
    }
    else if(HRESETn.read() == true && ahb_read_en) {
        read_en = 1;
    }
    else {
        read_en = 0;
    }
}

void SM4_REG_AHB::control_signal_input(){
    if(HRESETn.read() == false) {
        control_signal = 0xffffffff;
    }
    else if(HRESETn.read() == true && (SM4_addr.read() == 0x020) && write_en && (HSIZE_reg.read() == 0b010)) {
        control_signal = HWDATA.read();
    }
    else return;
}

void SM4_REG_AHB::data_input(){
    if(HRESETn.read() == false) {
        for(int i = 0; i < 4; i++) {
            x_in[i] = 0;
        }
    }
    else if(HRESETn.read() == true && (SM4_addr.read().range(11,4) == 0b00000000) && write_en && (HSIZE_reg.read() == 0b010)) {
        x_in[SM4_addr.read().range(3,2)] = HWDATA.read();
    }
    else return;
}

void SM4_REG_AHB::assign_data_in(){
    sc_biguint<128> x_in_reg;

    x_in_reg.range(127,96) = x_in[0];
    x_in_reg.range( 95,64) = x_in[1];
    x_in_reg.range( 63,32) = x_in[2];
    x_in_reg.range( 31, 0) = x_in[3];

    data_in = x_in_reg;
}

void SM4_REG_AHB::assign_data_out(){
    sc_uint<32> y_out_reg[4];

    y_out_reg[0] = data_out.read().range(127,96);
    y_out_reg[1] = data_out.read().range( 95,64);
    y_out_reg[2] = data_out.read().range( 63,32);
    y_out_reg[3] = data_out.read().range( 31, 0);

    y_out[0] = y_out_reg[0];
    y_out[1] = y_out_reg[1];
    y_out[2] = y_out_reg[2];
    y_out[3] = y_out_reg[3];
}

void SM4_REG_AHB::data_output(){
    if(HRESETn.read() == false) {
        hrdata = 0xffffffff;
    }
    else if(HRESETn.read() == true && read_en && (HSIZE.read() == 0b010)) {
        if(HADDR.read().range(11,0) == 0x020)
            hrdata = control_signal;
        else if(HADDR.read().range(11,0) == 0x024)
            hrdata = state_signal;
        else if(HADDR.read().range(11,4)==0b00000000)
            hrdata = x_in[HADDR.read().range(3,2)];
        else if(HADDR.read().range(11,4)==0b00000001)
            hrdata = y_out[HADDR.read().range(3,2)];
        else hrdata = 0xffffffff;
    }
    else return;
}

void SM4_REG_AHB::assign_HRDATA(){
    HRDATA = hrdata;
}

