
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "kafl_user.h"
#include <stdlib.h>

int main(int argc, char** argv){

  if(argc == 4){
    uint64_t start = 0;
    uint64_t end = 0;
    uint64_t range_num = 0;

    start = strtoul(argv[1], NULL, 16);
	  end = strtoul(argv[2], NULL, 16);
	  range_num = strtoul(argv[3], NULL, 16);

    printf("RANGE: %lx - %lx\n", start, end), 

    assert(range_num <= 3);
    assert(start != end && start < end);

    /* lets use our autoconfiguration hypercall */
    uint64_t* range = malloc(sizeof(uint64_t)*3);
    memset(range, 0x0, sizeof(uint64_t)*3);
    /* userspace */
    range[0] = start;
    range[1] = end;
    range[2] = range_num; /* range one */
    kAFL_hypercall(HYPERCALL_KAFL_RANGE_SUBMIT, (uintptr_t)range);

  }
  else{
    printf("USAGE: IP_A IP_B RANGE\n");
  }
  return 1;
}