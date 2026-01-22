#include "SM3.h"

int main(int argc, char *argv[]) {
    unsigned int i;
    unsigned char* message = "abc";
    unsigned int message_len = 3;
    unsigned int V[8] = { 0 };

    SM3(message, message_len, V);

    printf("Message:\n");
    printf("%s\n", message);

    printf("Hash:\n");
    for(i = 0; i < 8; i++) {
        printf("%08x ",V[i]);
    }
    printf("\n");

    message = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
    message_len = 64;

    SM3(message, message_len, V);

    printf("Message:\n");
    printf("%s\n", message);

    printf("Hash:\n");
    for(i = 0; i < 8; i++) {
        printf("%08x ",V[i]);
    }
    printf("\n");

    return 0;
}

