#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "kafl_user.h"
#include <stdlib.h>

static inline bool get_kvm_address(uint64_t* start, uint64_t* end){
    FILE * fp;
    char * line = NULL;
    uint64_t address = 0x0;

    uint64_t kvm_start = 0;
    uint64_t kvm_size = 0;
    uint64_t kvm_intel_start = 0;
    uint64_t kvm_intel_size = 0;

    system("cat /proc/modules | grep 'kvm ' | cut -d' ' -f6 > /tmp/kvm_start.txt");
    system("cat /proc/modules | grep 'kvm ' | cut -d' ' -f2 > /tmp/kvm_size.txt");

    system("cat /proc/modules | grep 'kvm_intel ' | cut -d' ' -f6 > /tmp/kvm_intel_start.txt");
    system("cat /proc/modules | grep 'kvm_intel ' | cut -d' ' -f2 > /tmp/kvm_intel_size.txt");


    fp = fopen("/tmp/kvm_start.txt", "r");
    if (fp == NULL){
        return false;
    }

		line = malloc(20);
		memset(line, 0x0, 20);
		fread(line, 18, 1, fp);
		kvm_start = strtoull(strtok(line, " "), NULL, 16);
		free(line);
    fclose(fp);

    fp = fopen("/tmp/kvm_size.txt", "r");
    if (fp == NULL){
        return false;
    }

		line = malloc(20);
		memset(line, 0x0, 20);
		fread(line, 18, 1, fp);
		kvm_size = strtoull(strtok(line, " "), NULL, 10);
		free(line);
    fclose(fp);


    fp = fopen("/tmp/kvm_intel_start.txt", "r");
    if (fp == NULL){
        return false;
    }

		line = malloc(20);
		memset(line, 0x0, 20);
		fread(line, 18, 1, fp);
		kvm_intel_start = strtoull(strtok(line, " "), NULL, 16);
		free(line);
    fclose(fp);

    fp = fopen("/tmp/kvm_intel_size.txt", "r");
    if (fp == NULL){
        return false;
    }

		line = malloc(20);
		memset(line, 0x0, 20);
		fread(line, 18, 1, fp);
		kvm_intel_size = strtoull(strtok(line, " "), NULL, 10);
		free(line);
    fclose(fp);

    printf("kvm:       %lx - %lx\n", kvm_start, kvm_start+kvm_size);

    printf("kvm-intel: %lx - %lx\n", kvm_intel_start, kvm_intel_start+kvm_intel_size);

    if(kvm_start > kvm_intel_start){
      *start = kvm_intel_start;
      *end = kvm_start+kvm_size;
    }
    else{
      *start = kvm_start;
      *end = kvm_intel_start+kvm_intel_size;
    }

    printf("START: %lx\n", *start);
    printf("END: %lx\n", *end);

		return true; 
}

int main(int argc, char** argv){

  uint64_t start_vmm;
  uint64_t end_vmm;
  get_kvm_address(&start_vmm, &end_vmm);
  printf("kvm: %lx-%lx", start_vmm, end_vmm);

  /* lets use our autoconfiguration hypercall */
  uint64_t* range = malloc(sizeof(uint64_t)*3);
  memset(range, 0x0, sizeof(uint64_t)*3);
  /* userspace */
  range[0] = start_vmm;
  range[1] = end_vmm;
  range[2] = 0; /* range zero */

  kAFL_hypercall(HYPERCALL_KAFL_RANGE_SUBMIT, (uintptr_t)range);
}