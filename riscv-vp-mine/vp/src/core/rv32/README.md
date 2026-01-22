timing.h exists for pipeline timing model

#ifdef PIPELINE_TIMING
enable pipeline timing model
#else
unable pipeline timing model
#endif

write/read/u_read/shamt:solve register latency time
do_load:solve memory latency time/push register latency time information
reserve_store:push memory latency time information

write:
LUI/AUIPC

write/read:
ADDI/SLTI/XORI/ORI/ANDI/ADD/SUB/SLT/XOR/OR/AND

write/u_read:
SLTIU/SLTU

write/read/shamt:
SLL/SRA/SLLI/SRAI

write/u_read/shamt:
SRL/SRLI

read/reserve_store:
SB/SH/SW

write/read/do_load:
LB/LH/LW/LBU/LHU

num_total_cycles:
MCYCLE_ADDR/MCYCLEH_ADDR
