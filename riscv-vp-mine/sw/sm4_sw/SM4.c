#include "SM4.h"

int main() {

    //data
    int i;
    unsigned int key[4] = {
        0x01234567,
        0x89abcdef,
        0xfedcba98,
        0x76543210
    };
    unsigned int plaintext[4] = {
        0x01234567,
        0x89abcdef,
        0xfedcba98,
        0x76543210
    };
    unsigned int rk_output[32];
    unsigned int En_output[4];
    unsigned int De_output[4];

    SM4_KeyExp(key, rk_output);
    SM4_Encrypt(rk_output, plaintext, En_output);
    SM4_Decrypt(rk_output, En_output, De_output);

    for(i = 0; i < 4; i++) {
        if(De_output[i] != plaintext[i]) {
            printf("Self-check error ! \n");
            return 1;
        }
    }

    printf("Encry data is:   %x_%x_%x_%x \n", plaintext[0], plaintext[1], plaintext[2], plaintext[3]);
    printf("Encry result is: %x_%x_%x_%x \n", En_output[0], En_output[1], En_output[2], En_output[3]);
    printf("Decry data is:   %x_%x_%x_%x \n", En_output[0], En_output[1], En_output[2], En_output[3]);
    printf("Decry result is: %x_%x_%x_%x \n", De_output[0], De_output[1], De_output[2], De_output[3]);

    printf("\n");
    printf("Self-check success ! \n");

/*
    //picture
    unsigned int key[4] = {
        0x01234567,
        0x89abcdef,
        0xfedcba98,
        0x76543210
    };
    unsigned int plaintext[4] = {0};
    unsigned int ciphertext[4] = {0};

    unsigned int rk_output[32];
    unsigned int En_output[4];
    unsigned int De_output[4];

    SM4_KeyExp(key, rk_output);

    int get;
    unsigned int tmp;
    unsigned char buf[16];
    int put;
    int i;
    int j = 0;

    //en
    char *plain_flieUrl = "/home/x/riscv-vp-mine/sw/sm4_sw/view.png";
    char *En_flieUrl = "/home/x/riscv-vp-mine/sw/sm4_sw/view1.png";

    FILE *plain_flie = fopen(plain_flieUrl, "rb");
    FILE *En_flie = fopen(En_flieUrl, "wb");

    if (!plain_flie || !En_flie) {
        printf("Failed to open, please check: %s", plain_flieUrl);
        return 1;
    }

    while((get = fgetc(plain_flie)) != EOF) {
        tmp = get;
        plaintext[0] = tmp;
        SM4_Encrypt(rk_output, plaintext, En_output);

        for(i = 0; i < 4; i++) {
            buf[i * 4    ] = (En_output[i] >> 24) & 0xFF;
            buf[i * 4 + 1] = (En_output[i] >> 16) & 0xFF;
            buf[i * 4 + 2] = (En_output[i] >>  8) & 0xFF;
            buf[i * 4 + 3] =  En_output[i]        & 0xFF;
        }

        for(i = 0; i < 16; i++) {
            put = buf[i];
            fputc(put, En_flie);
        }
    }

    fclose(plain_flie);
    fclose(En_flie);

    //de
    char *cipher_flieUrl = "/home/x/riscv-vp-mine/sw/sm4_sw/view1.png";
    char *De_flieUrl = "/home/x/riscv-vp-mine/sw/sm4_sw/view2.png";

    FILE *cipher_flie = fopen(cipher_flieUrl, "rb");
    FILE *De_flie = fopen(De_flieUrl, "wb");

    if (!cipher_flie || !De_flie) {
        printf("Failed to open, please check: %s", cipher_flieUrl);
        return 1;
    }

    while((get = fgetc(cipher_flie)) != EOF) {
        tmp = get;
        buf[j] = tmp;

        if(j == 15) {
            for(i = 0; i < 4; i++) {
                ciphertext[i] = buf[i * 4] << 24 | buf[i * 4 + 1] << 16 | buf[i * 4 + 2] << 8 | buf[i * 4 + 3];
            }
            SM4_Decrypt(rk_output, ciphertext, De_output);
            put = De_output[0] & 0xFF;
            fputc(put, De_flie);
            j = 0;
        }
        else
            j++;
    }

    fclose(cipher_flie);
    fclose(De_flie);
*/
    return 0;
}

