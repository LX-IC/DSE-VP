#ifndef __SM4_H__
#define __SM4_H__

#include <stdio.h>
#include <string.h>

typedef unsigned char  Uns8;
typedef unsigned short Uns16;
typedef unsigned int   Uns32;

//SM4_REG control cmd
#define MOD_IN_RST     0x00000000
#define MOD_IN_KEY     0x00000001
#define MOD_IN_ENC     0x00000002
#define MOD_IN_DEC     0x00000003

//Address
#define addr_x_in_0    0x50003000
#define addr_x_in_1    0x50003004
#define addr_x_in_2    0x50003008
#define addr_x_in_3    0x5000300c

#define addr_y_out_0   0x50003010
#define addr_y_out_1   0x50003014
#define addr_y_out_2   0x50003018
#define addr_y_out_3   0x5000301c

#define addr_control   0x50003020
#define addr_state     0x50003024

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

void sm4_test(){
    while(1) {
        Uns16 i;
        Uns32 addr;
        Uns32 rdata;
        Uns32 key[4];
        Uns32 plaintext[4];
        Uns32 ciphertext[4];

        key[0] = 0x01234567;
        key[1] = 0x89abcdef;
        key[2] = 0xfedcba98;
        key[3] = 0x76543210;

        plaintext[0] = 0x01234567;
        plaintext[1] = 0x89abcdef;
        plaintext[2] = 0xfedcba98;
        plaintext[3] = 0x76543210;

        Endian mode = get_endian();
        printf("CPU mode = %d \n", mode);

        //rst
        writeReg32(addr_control, MOD_IN_RST, mode);

        //key
        for(i = 0; i < 4; i++) {
            addr = addr_x_in_0 + (i * 4);
            writeReg32(addr, key[i], mode);
        }

        writeReg32(addr_control, MOD_IN_KEY, mode);

        while(rdata != 2) {
            rdata = readReg32(addr_state, mode);
        }

        printf("key extension complete! \n");

        //enc
        for(i = 0; i < 4; i++) {
            addr = addr_x_in_0 + (i * 4);
            writeReg32(addr, plaintext[i], mode);
        }

        writeReg32(addr_control, MOD_IN_ENC, mode);

        while(rdata != 4) {
            rdata = readReg32(addr_state, mode);
        }

        printf("encrypt complete! result: \n");

        for(i = 0; i < 4; i++) {
            addr = addr_y_out_0 + (i * 4);
            ciphertext[i] = readReg32(addr, mode);

            printf("%08x ",ciphertext[i]);

            if(i == 3)
                printf("\n");
         }

         //dec
         for(i = 0; i < 4; i++) {
             addr = addr_x_in_0 + (i * 4);
             writeReg32(addr, ciphertext[i], mode);
         }

         writeReg32(addr_control, MOD_IN_DEC, mode);

         while(rdata != 6) {
             rdata = readReg32(addr_state, mode);
         }

         printf("decrypt complete! result: \n");

         for(i = 0; i < 4; i++) {
             addr = addr_y_out_0 + (i * 4);
             plaintext[i] = readReg32(addr, mode);

             printf("%08x ",plaintext[i]);

             if(i == 3)
                 printf("\n");
          }

          break;
    }
}

#endif
