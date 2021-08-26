/*

Copyright (C) 2020 Sergej Schumilo

This file is part of QEMU-PT (kAFL).

QEMU-PT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

QEMU-PT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QEMU-PT.  If not, see <http://www.gnu.org/licenses/>.

*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "kafl_user.h"

static inline uint64_t get_address(char* identifier)
{
    FILE * fp;
    char * line = NULL;
    ssize_t read;
    ssize_t len;
    char *tmp;
    uint64_t address = 0x0;
    uint8_t identifier_len = strlen(identifier);

    fp = fopen("/proc/kallsyms", "r");
    if (fp == NULL){
        return address;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        if(strlen(line) > identifier_len && !strcmp(line + strlen(line) - identifier_len, identifier)){
                address = strtoull(strtok(line, " "), NULL, 16);
                break;
        }
    }

    fclose(fp);
    if (line){
        free(line);
    }
    return address;
}

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

static bool req_data(const char* file){
  FILE* f = fopen(file, "w+");

  void* stream_data = mmap((void*)NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  bool success = true;
  uint64_t bytes = 0;
  uint64_t total = 0;

  do{
    strcpy(stream_data, file);
    bytes = kAFL_hypercall(HYPERCALL_KAFL_REQ_STREAM_DATA, (uint64_t)stream_data);

    if(bytes == 0xFFFFFFFFFFFFFFFFUL){
      printf("HYPERVISOR: ERROR\n");
      success = false;
      break;
    }

    fwrite(stream_data, 1, bytes, f);
    total += bytes;

  } while(bytes);

  fclose(f);
  return success;
}

int main(int argc, char** argv)
{
	uint64_t panic_handler = 0x0;
	uint64_t kasan_handler = 0x0;
	void* program_buffer;

	printf("<< kAFL QEMU/KVM Loader for Linux x86-64 >>\n");

	if(geteuid()){
        printf("Loader requires root privileges...\n");
        return 1;
    }
  
  system("modprobe kvm");
  system("modprobe kvm-intel ept=1");

  panic_handler = get_address("T panic\n");
	printf("Kernel Panic Handler Address:\t%lx\n", panic_handler);

  kasan_handler = get_address("t kasan_report_error\n");
	if (kasan_handler){
		printf("Kernel KASAN Handler Address:\t%lx\n", kasan_handler);
	}

  uint64_t start_vmm;
  uint64_t end_vmm;
  get_kvm_address(&start_vmm, &end_vmm);
  printf("kvm: %lx-%lx", start_vmm, end_vmm);

	/* this hypercall will generate a VM snapshot for the fuzzer and subsequently terminate QEMU */
	kAFL_hypercall(HYPERCALL_KAFL_LOCK, 0);

	/***** Fuzzer Entrypoint *****/
	
	/* submit panic address */
	kAFL_hypercall(HYPERCALL_KAFL_SUBMIT_PANIC, panic_handler);
  if (kasan_handler){
	  kAFL_hypercall(HYPERCALL_KAFL_SUBMIT_KASAN, kasan_handler);
  }

  if(!req_data("run.sh")){
    hprintf("%s not available\n", "run.sh");
    kAFL_hypercall(HYPERCALL_KAFL_USER_ABORT, 0);
  }
  if(!req_data("req_data")){
    hprintf("%s not available\n", "req_data");
    kAFL_hypercall(HYPERCALL_KAFL_USER_ABORT, 0);
  }

  system("chmod +x run.sh");
  system("chmod +x req_data");

  hprintf("starting run.sh ...\n");
  /* bye */ 
  system("./run.sh");

  kAFL_hypercall(HYPERCALL_KAFL_PANIC, 1);

	return 0;
}
