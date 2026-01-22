#ifndef LIBTLMPWT_H
#define LIBTLMPWT_H

#define NVMAIN
#define LIBTLMPWT

#define ISS_set_activity 0
#define ISS_add_activity 1

#define GenericMMU_set_activity 0
#define GenericMMU_add_activity 1

#define CombinedMemoryInterface_set_activity 0
#define CombinedMemoryInterface_add_activity 0.5

#define SimpleMemory_set_activity 0
#define SimpleMemory_add_activity 0.1

#define GenericElfLoader_set_activity 0
#define GenericElfLoader_add_activity 1
#define GenericElfLoader_add_cycle 1

#define SimpleBus_set_activity 0
#define SimpleBus_add_activity 0.1

#define SyscallHandler_set_activity 0
#define SyscallHandler_add_activity 1

#define DebugMemoryInterface_set_activity 0
#define DebugMemoryInterface_add_activity 1
#define DebugMemoryInterface_add_cycle 1

#define RealCLINT_set_activity 0
#define RealCLINT_add_activity 1

#define DMI_PWT_set_activity 0
#define DMI_PWT_add_activity 0

#define InstrMemoryProxy_set_activity 0
#define InstrMemoryProxy_add_activity 0.1

#define SM3_set_activity 0
#define SM3_add_activity 1

#define SM4_set_activity 0
#define SM4_add_activity 1

#define AHB_set_activity 0
#define AHB_add_activity 1

#define mul_cycle 100//SM3,SM4,AHB

#define SM3_NICE_set_activity 0
#define SM3_NICE_add_activity 1
#define SM3_NICE_add_cycle 6

#define SM4_NICE_set_activity 0
#define SM4_NICE_add_activity 1
#define SM4_NICE_F_add_cycle 8
#define SM4_NICE_T_add_cycle 6
#define SM4_NICE_Nonlinear_add_cycle 6
#define SM4_NICE_Linear_add_cycle 4

#endif //LIBTLMPWT_H