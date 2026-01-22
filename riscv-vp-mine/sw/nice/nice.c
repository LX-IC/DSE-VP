#include "insn.h"

int main(void)
{
   unsigned int x[4] = {
        0x0,
        0x1,
        0x2,
        0x3
    };
   unsigned int y[4] = {
        0x0,
        0x1,
        0x2,
        0x3
    };

    add(x, y);

    return 0;
}
