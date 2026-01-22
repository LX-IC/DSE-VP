#ifndef __INSN_H__
#define __INSN_H__

////////////////////////////////////////////////////////////
// custom3:
// Supported format: only R type here
// Supported instr:
//  1. custom3 sm3p0: Implements the P0 transformation function as used in the SM3 hash function
//     .insn r opcode, func3, func7, rd, rs1, rs2
//  2. custom3 sm3p1: Implements the P1 transformation function as used in the SM3 hash function
//     .insn r opcode, func3, func7, rd, rs1, rs2
////////////////////////////////////////////////////////////

#define __STATIC_FORCEINLINE __attribute__((always_inline)) static inline

// custom3 sm3p0
__STATIC_FORCEINLINE unsigned int custom3_sm3p0(unsigned int X)
{
    unsigned int P0;

    asm volatile (
       ".insn r 0x7b, 7, 1, %0, %1, x0"
           :"=r"(P0)
           :"r"(X)
     );
    return P0;
}

// custom3 sm3p1
__STATIC_FORCEINLINE unsigned int custom3_sm3p1(unsigned int X)
{
    unsigned int P1;

    asm volatile (
       ".insn r 0x7b, 7, 2, %0, %1, x0"
           :"=r"(P1)
           :"r"(X)
     );
    return P1;
}

unsigned int sm3_nice(unsigned char* message, unsigned int message_len);

#endif