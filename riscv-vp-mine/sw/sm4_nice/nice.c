#include "insn.h"

int main(void)
{
    unsigned int key[4] = {
        0x01234567,
        0x89abcdef,
        0xfedcba98,
        0x76543210
    };
    unsigned int cipher[4] = {
        0x01234567,
        0x89abcdef,
        0xfedcba98,
        0x76543210
    };

    sm4_nice(key, cipher);

    return 0;
}
