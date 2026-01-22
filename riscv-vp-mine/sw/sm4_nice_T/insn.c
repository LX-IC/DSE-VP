#include "insn.h"
#include <stdio.h>

unsigned int SM4_FK[4] = {
  0xA3B1BAC6,
  0x56AA3350,
  0x677D9197,
  0xB27022DC
};

unsigned int SM4_CK[32] = {
  0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
  0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
  0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
  0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
  0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
  0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
  0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
  0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
};

unsigned int sm4_nice(unsigned int key[4], unsigned int cipher[4])
{
  volatile unsigned int i=0;
  volatile unsigned int K_temp;
  volatile unsigned int K[36]={0};
  volatile unsigned int X_temp;
  volatile unsigned int X[36]={0};
  volatile unsigned int Y_temp;
  volatile unsigned int Y[36]={0};
  volatile unsigned int rk[32]={0};

  for(i = 0; i < 4; i++) {
    K[i] = key[i] ^ SM4_FK[i];
  }

  for(i = 0; i < 32; i++) {
    K_temp = K[i + 1] ^ K[i + 2] ^ K[i + 3] ^ SM4_CK[i];
    K[i + 4] = K[i] ^ custom3_sm4ks_T(K_temp);
    rk[i] = K[i + 4];
  }

  printf("key extension complete! \n");

  for(i = 0; i < 4; i++) {
    X[i] = cipher[i];
  }

  for(i = 0; i < 32; i++) {
    X_temp = X[i + 1] ^ X[i + 2] ^ X[i + 3] ^ rk[i];
    X[i + 4] = X[i] ^ custom3_sm4ed_T(X_temp);
  }

  printf("encrypt complete! result: \n");

  for(i = 0; i < 4; i++) {
    printf("%08x ", X[35-i]);
    if(i == 3)
      printf("\n");
  }

  for(i = 0; i < 4; i++) {
    Y[i] = X[35-i];
  }

  for(i = 0; i < 32; i++) {
    Y_temp = Y[i + 1] ^ Y[i + 2] ^ Y[i + 3] ^ rk[31-i];
    Y[i + 4] = Y[i] ^ custom3_sm4ed_T(Y_temp);
  }

  printf("decrypt complete! result: \n");

  for(i = 0; i < 4; i++) {
    printf("%08x ", Y[35-i]);
    if(i == 3)
      printf("\n");
  }

  return 0;
}