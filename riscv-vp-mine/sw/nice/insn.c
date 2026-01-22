#include "insn.h"
#include <stdio.h>

// add
unsigned int add(unsigned int x[4], unsigned int y[4])
{
  volatile unsigned int i=0;
  volatile unsigned int x_temp[4]={0};
  volatile unsigned int y_temp[4]={0};
  volatile unsigned int z_temp[4]={0};

  for(i = 0; i < 4; i++) {
        x_temp[i] = x[i];
        y_temp[i] = y[i];
    }

  for(i = 0; i < 4; i++) {
        z_temp[i] = custom3_add(x_temp[i], y_temp[i]);
    }

  printf("\n");
  for (i = 0; i < 4; i++) printf("result %d : %08x\n", i+1, z_temp[i]);

  return 0;
}