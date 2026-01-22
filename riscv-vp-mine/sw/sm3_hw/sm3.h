#ifndef __SM3_H__
#define __SM3_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char  Uns8;
typedef unsigned short Uns16;
typedef unsigned int   Uns32;

//SM3 control cmd
#define MOD_IN_RST   0x00000001
#define MOD_IN_OPT   0x00000001
#define MOD_IN_STO   0x00000000

//Address
#define addr_x_in_0  0x50002000
#define addr_x_in_1  0x50002004
#define addr_x_in_2  0x50002008
#define addr_x_in_3  0x5000200c
#define addr_x_in_4  0x50002010
#define addr_x_in_5  0x50002014
#define addr_x_in_6  0x50002018
#define addr_x_in_7  0x5000201c
#define addr_x_in_8  0x50002020
#define addr_x_in_9  0x50002024
#define addr_x_in_10 0x50002028
#define addr_x_in_11 0x5000202c
#define addr_x_in_12 0x50002030
#define addr_x_in_13 0x50002034
#define addr_x_in_14 0x50002038
#define addr_x_in_15 0x5000203c

#define addr_y_out_0 0x50002040
#define addr_y_out_1 0x50002044
#define addr_y_out_2 0x50002048
#define addr_y_out_3 0x5000204c
#define addr_y_out_4 0x50002050
#define addr_y_out_5 0x50002054
#define addr_y_out_6 0x50002058
#define addr_y_out_7 0x5000205c

#define addr_SM3_rst 0x50002060
#define addr_SM3_opt 0x50002064
#define addr_V_ready 0x50002068

#define LE2BE(X) (((X) & 0x000000ff) << 24) | (((X) & 0x0000ff00) << 8) | (((X) & 0x00ff0000) >> 8) | (((X) & 0xff000000) >> 24)

//LE:little-endian BE:big-endian
typedef enum {LE=0, BE} Endian;

//Determine whether the CPU storage mode is big-endian or little-endian
Endian get_endian(){
    Uns16 data = 0x1122;
    Uns8 *p = (Uns8 *)&data;

    return (*p < *(p+1)) ? BE : LE;
}

void writeReg32(Uns32 addr, Uns32 data, Endian mode){
    if(mode == LE)
        *(volatile Uns32*)addr = data;
    else if(mode == BE)
        *(volatile Uns32*)addr = LE2BE(data);
    else
        return;
}

Uns32 readReg32(Uns32 addr, Endian mode){
    if(mode == LE)
        return *(volatile Uns32*)addr;
    else if(mode == BE)
        return LE2BE(*(volatile Uns32*)addr);
    else
        return 0;
}

void sm3_test(){
    while(1) {
        Uns16 i;
        Uns32 addr;
        Uns32 rdata;
        Uns32 message_in[16];
        Uns32 V_out[8];

        //"abc"
        unsigned char* message = "abc";
        unsigned int message_len = 3;

        unsigned int num = (message_len + 1 + 8 + 64) / 64;
        unsigned char* message_buf = (unsigned char*)malloc(num * 64);
        memset(message_buf, 0, num * 64);
        memcpy(message_buf, message, message_len);
        message_buf[message_len] = 0x80;
        for(i = 0; i < 8; i++) {
            message_buf[num * 64 - i - 1] = ((unsigned long long)(message_len * 8) >> (i * 8)) & 0xFF;
        }

        for(i = 0; i < 16; i++) {
            message_in[i] = ((unsigned int)message_buf[i * 4 + 0] << 24) & 0xFF000000
                          | ((unsigned int)message_buf[i * 4 + 1] << 16) & 0x00FF0000
                          | ((unsigned int)message_buf[i * 4 + 2] <<  8) & 0x0000FF00
                          | ((unsigned int)message_buf[i * 4 + 3] <<  0) & 0x000000FF;
        }

        Endian mode = get_endian();
        printf("CPU mode = %d \n", mode);

        //rst
        writeReg32(addr_SM3_rst, MOD_IN_RST, mode);
        while(rdata != 0) {
            rdata = readReg32(addr_V_ready, mode);
        }
        writeReg32(addr_SM3_rst, MOD_IN_STO, mode);

        //message_in
        for(i = 0; i < 16; i++) {
            addr = addr_x_in_0 + (i * 4);
            writeReg32(addr, message_in[i], mode);
        }

        writeReg32(addr_SM3_opt, MOD_IN_OPT, mode);
        writeReg32(addr_SM3_opt, MOD_IN_STO, mode);

        while(rdata != 1) {
            rdata = readReg32(addr_V_ready, mode);
        }

        printf("SM3 complete! \n");

        printf("Message: \n");
        printf("%s\n", message);

        printf("Hash: \n");
        for(i = 0; i < 8; i++) {
             addr = addr_y_out_0 + (i * 4);
             V_out[i] = readReg32(addr, mode);

             printf("%08x ",V_out[i]);

             if(i == 7)
                 printf("\n");
          }

        //"abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
        message = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
        message_len = 64;

        num = (message_len + 1 + 8 + 64) / 64;
        message_buf = (unsigned char*)malloc(num * 64);
        memset(message_buf, 0, num * 64);
        memcpy(message_buf, message, message_len);
        message_buf[message_len] = 0x80;
        for(i = 0; i < 8; i++) {
            message_buf[num * 64 - i - 1] = ((unsigned long long)(message_len * 8) >> (i * 8)) & 0xFF;
        }

        for(i = 0; i < 16; i++) {
            message_in[i] = ((unsigned int)message_buf[i * 4 + 0] << 24) & 0xFF000000
                          | ((unsigned int)message_buf[i * 4 + 1] << 16) & 0x00FF0000
                          | ((unsigned int)message_buf[i * 4 + 2] <<  8) & 0x0000FF00
                          | ((unsigned int)message_buf[i * 4 + 3] <<  0) & 0x000000FF;
        }

        //rst
        writeReg32(addr_SM3_rst, MOD_IN_RST, mode);
        while(rdata != 0) {
            rdata = readReg32(addr_V_ready, mode);
        }
        writeReg32(addr_SM3_rst, MOD_IN_STO, mode);

        //message_in
        for(i = 0; i < 16; i++) {
            addr = addr_x_in_0 + (i * 4);
            writeReg32(addr, message_in[i], mode);
        }

        writeReg32(addr_SM3_opt, MOD_IN_OPT, mode);
        writeReg32(addr_SM3_opt, MOD_IN_STO, mode);

        while(rdata != 1) {
            rdata = readReg32(addr_V_ready, mode);
        }

        for(i = 0; i < 16; i++) {
            message_in[i] = ((unsigned int)message_buf[i * 4 + 64] << 24) & 0xFF000000
                          | ((unsigned int)message_buf[i * 4 + 65] << 16) & 0x00FF0000
                          | ((unsigned int)message_buf[i * 4 + 66] <<  8) & 0x0000FF00
                          | ((unsigned int)message_buf[i * 4 + 67] <<  0) & 0x000000FF;
        }

        //message_in
        for(i = 0; i < 16; i++) {
            addr = addr_x_in_0 + (i * 4);
            writeReg32(addr, message_in[i], mode);
        }

        writeReg32(addr_SM3_opt, MOD_IN_OPT, mode);
        writeReg32(addr_SM3_opt, MOD_IN_STO, mode);

        while(rdata != 2) {
            rdata = readReg32(addr_V_ready, mode);
        }

        printf("SM3 complete! \n");

        printf("Message: \n");
        printf("%s\n", message);

        printf("Hash: \n");
        for(i = 0; i < 8; i++) {
             addr = addr_y_out_0 + (i * 4);
             V_out[i] = readReg32(addr, mode);

             printf("%08x ",V_out[i]);

             if(i == 7)
                 printf("\n");
          }

          break;
    }
}

#endif
