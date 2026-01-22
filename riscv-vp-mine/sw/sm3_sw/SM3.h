#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SM3_Rotl32(buf, n) (((buf)<<n)|((buf)>>(32-n)))

unsigned int IV[8] = {
 0x7380166F,
 0x4914B2B9,
 0x172442D7,
 0xDA8A0600,
 0xA96F30BC,
 0x163138AA,
 0xE38DEE4D,
 0xB0FB0E4E
};

unsigned int T[64] = { 0 };
void T_init() {
    int j;

    for(j = 0; j < 16; j++)
        T[j] = 0x79CC4519;

    for(j = 16; j < 64; j++)
        T[j] = 0x7A879D8A;

    return;
}

unsigned int FF(unsigned int X, unsigned int Y, unsigned int Z, unsigned int j) {
    if(j>=0 && j<=15)
        return (X ^ Y ^ Z);
    else if(j>=16 && j<=63)
        return ((X & Y) | (X & Z) | (Y & Z));

    return 0;
}

unsigned int GG(unsigned int X, unsigned int Y, unsigned int Z, unsigned int j) {
    if(j>=0 && j<=15)
        return (X ^ Y ^ Z);
    else if(j>=16 && j<=63)
        return ((X & Y) | ((~X) & Z));

    return 0;
}

unsigned int P0(unsigned int X) {
    return (X ^ SM3_Rotl32(X, 9) ^ SM3_Rotl32(X, 17));
}

unsigned int P1(unsigned int X) {
    return (X ^ SM3_Rotl32(X, 15) ^ SM3_Rotl32(X, 23));
}
/*
unsigned int V[8] = { 0 };
for(int i = 0; i < 8; i++) {
    V[i] = IV[i];
}//初始化V0
for(i = 0; i < num; i++){
    message_ext(&message_buf[i * 64], W68, W64);//对填充后的消息按512比特进行分组然后扩展
    CF(unsigned int* V, unsigned int* W68, unsigned int* W64);//压缩
}
*/
unsigned int message_ext(unsigned char* message_buf, unsigned int W68[], unsigned int W64[]) {
    int j = 0;

    for(j = 0; j < 16; j++) {
        W68[j] = ((unsigned int)message_buf[j * 4 + 0] << 24) & 0xFF000000
               | ((unsigned int)message_buf[j * 4 + 1] << 16) & 0x00FF0000
               | ((unsigned int)message_buf[j * 4 + 2] <<  8) & 0x0000FF00
               | ((unsigned int)message_buf[j * 4 + 3] <<  0) & 0x000000FF;
    }//将消息分组B划分为16个字W0-W15

    for(j = 16; j < 68; j++) {
        W68[j] = P1(W68[j - 16] ^ W68[j - 9] ^ (SM3_Rotl32(W68[j - 3], 15))) ^ (SM3_Rotl32(W68[j - 13], 7)) ^ W68[j - 6];
    }//W16-W67

    for(j = 0; j < 64; j++) {
        W64[j] = W68[j] ^ W68[j + 4];
    }//W64

    return 0;
}

typedef enum {
    A=0,
    B=1,
    C=2,
    D=3,
    E=4,
    F=5,
    G=6,
    H=7
} type;

unsigned int CF(unsigned int V[], unsigned int W68[], unsigned int W64[]) {
    int j = 0;
    unsigned int SS1 = 0, SS2 = 0, TT1 = 0, TT2 = 0;
    unsigned int V_buf[8] = { 0 };

    for(j = 0; j < 8; j++) {
        V_buf[j] = V[j];
    }

    for(j = 0; j < 64; j++) {
        SS1 = SM3_Rotl32((SM3_Rotl32(V_buf[A], 12) + V_buf[E] + SM3_Rotl32(T[j], j % 32)), 7);
        SS2 = SS1 ^ (SM3_Rotl32(V_buf[A], 12));
        TT1 = FF(V_buf[A], V_buf[B], V_buf[C], j) + V_buf[D] + SS2 + W64[j];
        TT2 = GG(V_buf[E], V_buf[F], V_buf[G], j) + V_buf[H] + SS1 + W68[j];
        V_buf[D] = V_buf[C];
        V_buf[C] = SM3_Rotl32(V_buf[B], 9);
        V_buf[B] = V_buf[A];
        V_buf[A] = TT1;
        V_buf[H] = V_buf[G];
        V_buf[G] = SM3_Rotl32(V_buf[F], 19);
        V_buf[F] = V_buf[E];
        V_buf[E] = P0(TT2);
    }

    for(j = 0; j < 8; j++) {
        V[j] = V_buf[j] ^ V[j];
    }

    return 0;
}

unsigned int SM3(unsigned char* message, unsigned int message_len, unsigned int V[]) {
    int i = 0;
    unsigned int num = (message_len + 1 + 8 + 64) / 64;//这里的len是指string的长度，例如“abc”的长度是3,左右各乘8可得比特数，+1指在消息末尾+1,为8个比特，+8指将message_len用64比特表示并加到填充的消息末尾，+64则使其/64不为0,也就是填充后的消息长度最少是64x8=512位
    unsigned char* message_buf = (unsigned char*)malloc(num * 64);//如果num为1,则生成1个有64个元素,每个元素为8比特的数组，即512比特的内存空间
    memset(message_buf, 0, num * 64);//将生成的内存空间初始化为0
    memcpy(message_buf, message, message_len);//将消息的ASCII码写入512比特的起始位置，例如“abc”的ASCII码是0x616263，占3x8=4x6=24位
    message_buf[message_len] = 0x80;//将代表+1的8个比特写在512比特空间的已写入消息的后面

    for(i = 0; i < 8; i++) {
        message_buf[num * 64 - i - 1] = ((unsigned long long)(message_len * 8) >> (i * 8)) & 0xFF;
    }//将代表消息长度的64个比特写入512比特的末尾位置，i为8的原因是8x8=64，用unsigned long long的原因是有可能消息长度很大
    //message_fill

    unsigned int W68[68] = { 0 };
    unsigned int W64[64] = { 0 };

    T_init();//初始化T

    for(int i = 0; i < 8; i++) {
        V[i] = IV[i];
    }//初始化V0

    for(i = 0; i < num; i++){
        message_ext(&message_buf[i * 64], W68, W64);//对填充后的消息按512比特进行分组然后扩展
        CF(V, W68, W64);//压缩
    }

    return 0;
}

