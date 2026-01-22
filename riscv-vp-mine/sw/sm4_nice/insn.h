#ifndef __INSN_H__
#define __INSN_H__

////////////////////////////////////////////////////////////
// custom3:
// Supported format: only R type here
// Supported instr:
//  1. custom3 sm4ks: Accelerates the Key Schedule operation of the SM4 block cipher
//     .insn r opcode, func3, func7, rd, rs1, rs2
//  2. custom3 sm4ed: Accelerates the block encrypt/decrypt operation of the SM4 block cipher
//     .insn r opcode, func3, func7, rd, rs1, rs2
////////////////////////////////////////////////////////////

#define __STATIC_FORCEINLINE __attribute__((always_inline)) static inline

// custom3 sm4ks
__STATIC_FORCEINLINE unsigned int custom3_sm4ks(unsigned int K, unsigned int TK)
{
    unsigned int rk;

    asm volatile (
       ".insn r 0x7b, 7, 3, %0, %1, %2"
           :"=r"(rk)
           :"r"(K), "r"(TK)
     );
    return rk;
}

// custom3 sm4ed
__STATIC_FORCEINLINE unsigned int custom3_sm4ed(unsigned int X, unsigned int TX)
{
    unsigned int Y;

    asm volatile (
       ".insn r 0x7b, 7, 4, %0, %1, %2"
           :"=r"(Y)
           :"r"(X), "r"(TX)
     );
    return Y;
}

unsigned int sm4_nice(unsigned int key[4], unsigned int cipher[4]);

#endif