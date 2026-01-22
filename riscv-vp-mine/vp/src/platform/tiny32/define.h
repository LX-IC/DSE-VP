#ifndef DEFINE_H
#define DEFINE_H

// #define USING_SM3     //enable SM3
// #define USING_SM4     //enable SM4
#define USING_SM3_SM4 //enable SM3 and SM4

/* At anytime, one of the three modes of SM3 must be enabled. */
#define USING_SM3_AC  //period accurate
// #define USING_SM3_AP  //period approximate
// #define USING_SM3_NO  //period none

/* At anytime, one of the three modes of SM4 must be enabled. */
#define USING_SM4_AC  //period accurate
// #define USING_SM4_AP  //period approximate
// #define USING_SM4_NO  //period none

#define SM3_p_cycle_config 10 //only for SM3_Socket
#define SM4_p_cycle_config 10 //only for SM4_Socket

// #define USING_AHB     //enable AHB_Socket, use with AC mode

// #define DEBUG_SM3
// #define DEBUG_SM3_Socket

// #define DEBUG_SM4
// #define DEBUG_SM4_REG
// #define DEBUG_SM4_Socket

// #define DMI //enable Direct Memory Interface

// #define TQ //enable Time Quantum
// #define TQ_value 10 //10 NS = 1 cycle

#endif