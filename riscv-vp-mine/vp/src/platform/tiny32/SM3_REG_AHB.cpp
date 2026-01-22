#include "SM3_REG_AHB.h"

void SM3_REG_AHB::assign_ahb_init(){
    HRESP = 0;
    HREADY = 1;
}

void SM3_REG_AHB::assign_ahb_trans_valid(){
    ahb_trans_valid = HSEL.read() && HREADY && (HTRANS.read() == 0b10);
}

void SM3_REG_AHB::ahb_addr(){
    if(HRESETn.read() == false) {
        SM3_addr = 0xfff;
        HSIZE_reg = 0b000;
    }
    else if(HRESETn.read() == true && ahb_trans_valid) {
        SM3_addr = HADDR.read().range(11,0);
        HSIZE_reg = HSIZE.read();
    }
    else {
        SM3_addr = 0xfff;
        HSIZE_reg = 0b000;
    }
}

void SM3_REG_AHB::assign_ahb_write_en(){
    ahb_write_en = ahb_trans_valid && HWRITE.read();
}

void SM3_REG_AHB::assign_ahb_read_en(){
    ahb_read_en = ahb_trans_valid && (!HWRITE.read());
}

void SM3_REG_AHB::write_enable(){
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

void SM3_REG_AHB::read_enable(){
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

void SM3_REG_AHB::control_signal_input(){
    if(HRESETn.read() == false) {
        SM3_rst_signal = 0xffffffff;
        SM3_opt_signal = 0xffffffff;
    }
    else if(HRESETn.read() == true && (SM3_addr.read() == 0x060) && write_en && (HSIZE_reg.read() == 0b010)) {
        SM3_rst_signal = HWDATA.read();
    }
    else if(HRESETn.read() == true && (SM3_addr.read() == 0x064) && write_en && (HSIZE_reg.read() == 0b010)) {
        SM3_opt_signal = HWDATA.read();
    }
    else return;
}

void SM3_REG_AHB::data_input(){
    if(HRESETn.read() == false) {
        for(int i = 0; i < 4; i++) {
            x_in[i] = 0;
        }
    }
    else if(HRESETn.read() == true && (SM3_addr.read().range(11,6) == 0b000000) && write_en && (HSIZE_reg.read() == 0b010)) {
        x_in[SM3_addr.read().range(5,2)] = HWDATA.read();
    }
    else return;
}

void SM3_REG_AHB::assign_data_in(){
    sc_biguint<512> x_in_reg;

    x_in_reg.range(511,480) = x_in[ 0];
    x_in_reg.range(479,448) = x_in[ 1];
    x_in_reg.range(447,416) = x_in[ 2];
    x_in_reg.range(415,384) = x_in[ 3];
    x_in_reg.range(383,352) = x_in[ 4];
    x_in_reg.range(351,320) = x_in[ 5];
    x_in_reg.range(319,288) = x_in[ 6];
    x_in_reg.range(287,256) = x_in[ 7];
    x_in_reg.range(255,224) = x_in[ 8];
    x_in_reg.range(223,192) = x_in[ 9];
    x_in_reg.range(191,160) = x_in[10];
    x_in_reg.range(159,128) = x_in[11];
    x_in_reg.range(127, 96) = x_in[12];
    x_in_reg.range( 95, 64) = x_in[13];
    x_in_reg.range( 63, 32) = x_in[14];
    x_in_reg.range( 31,  0) = x_in[15];

    data_in = x_in_reg;
}

void SM3_REG_AHB::assign_data_out(){
    sc_uint<32> y_out_reg[8];

    y_out_reg[0] = data_out.read().range(255,224);
    y_out_reg[1] = data_out.read().range(223,192);
    y_out_reg[2] = data_out.read().range(191,160);
    y_out_reg[3] = data_out.read().range(159,128);
    y_out_reg[4] = data_out.read().range(127, 96);
    y_out_reg[5] = data_out.read().range( 95, 64);
    y_out_reg[6] = data_out.read().range( 63, 32);
    y_out_reg[7] = data_out.read().range( 31,  0);

    y_out[0] = y_out_reg[0];
    y_out[1] = y_out_reg[1];
    y_out[2] = y_out_reg[2];
    y_out[3] = y_out_reg[3];
    y_out[4] = y_out_reg[4];
    y_out[5] = y_out_reg[5];
    y_out[6] = y_out_reg[6];
    y_out[7] = y_out_reg[7];

}

void SM3_REG_AHB::data_output(){
    if(HRESETn.read() == false) {
        hrdata = 0xffffffff;
    }
    else if(HRESETn.read() == true && read_en && (HSIZE.read() == 0b010)) {
        if(HADDR.read().range(11,0) == 0x060)
            hrdata = SM3_rst_signal;
        else if(HADDR.read().range(11,0) == 0x064)
            hrdata = SM3_opt_signal;
        else if(HADDR.read().range(11,0) == 0x068)
            hrdata = V_ready_signal;
        else if(HADDR.read().range(11,6)==0b000000)
            hrdata = x_in[HADDR.read().range(5,2)];
        else if(HADDR.read().range(11,5)==0b0000010)
            hrdata = y_out[HADDR.read().range(4,2)];
        else hrdata = 0xffffffff;
    }
    else return;
}

void SM3_REG_AHB::assign_HRDATA(){
    HRDATA = hrdata;
}

