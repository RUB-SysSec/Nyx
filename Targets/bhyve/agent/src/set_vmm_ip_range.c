#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "kafl_user.h"
#include <stdlib.h>

static inline bool get_vmm_address(uint64_t* start, uint64_t* end){
    FILE * fp;
    char * line = NULL;
    uint64_t address = 0x0;

		system("kldstat | grep vmm | cut -d' ' -f7 > /tmp/vmm_start.txt");
		system("kldstat | grep vmm | cut -d' ' -f8 > /tmp/vmm_size.txt");

    fp = fopen("/tmp/vmm_start.txt", "r");
    if (fp == NULL){
        return false;
    }

		line = malloc(20);
		memset(line, 0x0, 20);
		fread(line, 18, 1, fp);
		*start = strtoull(strtok(line, " "), NULL, 16);
		free(line);
    fclose(fp);

    fp = fopen("/tmp/vmm_size.txt", "r");
    if (fp == NULL){
        return false;
    }

    line = malloc(20);
		memset(line, 0x0, 20);
		fread(line, 18, 1, fp);
		uint64_t size = strtoull(strtok(line, " "), NULL, 16);
		free(line);
    fclose(fp);


    //printf("%s: %lx %lx (%lx) %lx\n", __func__, *start, *start+size, size, *start+(size + (0x1000-(size)&0xFFF)));

    *end = *start+(size + (0x1000-(size)&0xFFF)); //*start+size;
		return true; 
}

int main(int argc, char** argv){

  uint64_t start_vmm;
  uint64_t end_vmm;
  get_vmm_address(&start_vmm, &end_vmm);
  printf("vmm: %lx-%lx", start_vmm, end_vmm);

  /* lets use our autoconfiguration hypercall */
  uint64_t* range = malloc(sizeof(uint64_t)*3);
  memset(range, 0x0, sizeof(uint64_t)*3);
  /* userspace */
  range[0] = start_vmm;
  range[1] = end_vmm;
  range[2] = 0; /* range zero */

  kAFL_hypercall(HYPERCALL_KAFL_RANGE_SUBMIT, (uintptr_t)range);
}