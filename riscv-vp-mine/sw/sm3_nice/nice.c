#include "insn.h"

int main(void)
{
    unsigned char* message = "abc";
    unsigned int message_len = 3;

    sm3_nice(message, message_len);

    message = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
    message_len = 64;

    sm3_nice(message, message_len);

    return 0;
}
