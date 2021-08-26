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

static inline uint64_t get_panic_address(void)
{
    FILE * fp;
    char * line = NULL;
    uint64_t address = 0x0;

		system("kldload /boot/kernel/ksyms.ko");
		system("readelf --syms /dev/ksyms | grep vpanic$ | cut -d' ' -f3 > /tmp/panic_address");

    fp = fopen("/tmp/panic_address", "r");
    if (fp == NULL){
        return address;
    }

		line = malloc(20);
		memset(line, 0x0, 20);
		fread(line, 18, 1, fp);
		address = strtoull(strtok(line, " "), NULL, 16);
		free(line);
		return address; 
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

	printf("<< kAFL Bhyve Loader for FREEBSD x86-64 >>\n");

	if(geteuid()){
        printf("Loader requires root privileges...\n");
        return 1;
    }
  
  system("kldload /boot/kernel/vmm.ko");

	panic_handler = get_panic_address();
	printf("Kernel Panic Handler Address:\t%lx\n", panic_handler);

	/* this hypercall will generate a VM snapshot for the fuzzer and subsequently terminate QEMU */
	kAFL_hypercall(HYPERCALL_KAFL_LOCK, 0);

	/***** Fuzzer Entrypoint *****/
	
	/* submit panic address */
	kAFL_hypercall(HYPERCALL_KAFL_SUBMIT_PANIC, panic_handler);

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
