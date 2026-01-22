#ifndef __INSN_H__
#define __INSN_H__

////////////////////////////////////////////////////////////
// custom3:
// Supported format: only R type here
// Supported instr:
//  1. custom3 sm4ks T
//     .insn r opcode, func3, func7, rd, rs1, rs2
//  2. custom3 sm4ed T
//     .insn r opcode, func3, func7, rd, rs1, rs2
////////////////////////////////////////////////////////////

#define __STATIC_FORCEINLINE __attribute__((always_inline)) static inline

// custom3 sm4ks T
__STATIC_FORCEINLINE unsigned int custom3_sm4ks_T(unsigned int TK)
{
    unsigned int rk;

    asm volatile (
       ".insn r 0x7b, 7, 5, %0, %1, x0"
           :"=r"(rk)
           :"r"(TK)
     );
    return rk;
}

// custom3 sm4ed T
__STATIC_FORCEINLINE unsigned int custom3_sm4ed_T(unsigned int TX)
{
    unsigned int Y;

    asm volatile (
       ".insn r 0x7b, 7, 6, %0, %1, x0"
           :"=r"(Y)
           :"r"(TX)
     );
    return Y;
}

unsigned int sm4_nice(unsigned int key[4], unsigned int cipher[4]);

#endif