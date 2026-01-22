#ifndef __INSN_H__
#define __INSN_H__

////////////////////////////////////////////////////////////
// custom3:
// Supported format: only R type here
// Supported instr:
//  1. custom3 add:
//     .insn r opcode, func3, func7, rd, rs1, rs2
////////////////////////////////////////////////////////////

#define __STATIC_FORCEINLINE __attribute__((always_inline)) static inline

// custom3 add
__STATIC_FORCEINLINE unsigned int custom3_add(unsigned int a, unsigned int b)
{
    unsigned int c;

    asm volatile (
       ".insn r 0x7b, 7, 0, %0, %1, %2"
           :"=r"(c)
           :"r"(a), "r"(b)
     );
    return c;
}

// add
unsigned int add(unsigned int x[4], unsigned int y[4]);

#endif

