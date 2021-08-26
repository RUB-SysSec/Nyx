#define _GNU_SOURCE

#include <sys/mman.h>
#include <dlfcn.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>

#define hprintf printf
#include "interpreter.h"

char* data_mmap=0;
size_t data_mmap_size=0;

int run_vm() {
    uint64_t* offsets = (uint64_t*)data_mmap;
    hprintf("checksum: %lx, %lx\n",offsets[0], INTERPRETER_CHECKSUM);
    assert(offsets[0] == INTERPRETER_CHECKSUM);
    assert(offsets[1] < 0xffffff);
    assert(offsets[2] < 0xffffff);
    assert(offsets[3] < 0xffffff);
    assert(offsets[4] < 0xffffff);
    uint64_t graph_size = offsets[1];
    uint64_t data_size = offsets[2];
    printf("graph_size: %d\n", graph_size);
    printf("data_size: %d\n", graph_size);
    printf("graph_offset: %d\n", offsets[3]);
    printf("data_offset: %d\n", offsets[4]);
    uint16_t* graph_ptr = (uint16_t*)(data_mmap+offsets[3]);
    uint8_t* data_ptr = (uint8_t*)(data_mmap+offsets[4]);
    assert(offsets[3]+graph_size*sizeof(uint16_t) <= data_mmap_size);
    assert(offsets[4]+data_size <= data_mmap_size);
    interpreter_t* vm = new_interpreter();
    init_interpreter(vm, graph_ptr, graph_size, data_ptr, data_size);
    interpreter_user_init(vm);
    run(vm);
    interpreter_user_shutdown(vm);
}

int main(int argc, char** argv) {
  char* file = getenv("STRUCT_INPUT_PATH");
  if(!file){
    printf("COULDN'T OPEN SPEC INPUT, SKIPING ENVIRONMENT SETUP\n");
    exit(0);
  } else {
    int fd=open(file, O_RDWR);
    assert(fd >= 0);
    struct stat attr;
    assert(fstat(fd, &attr) >= 0);

    data_mmap=mmap(0, attr.st_size, PROT_READ, MAP_SHARED, fd, 0);
    data_mmap_size = (size_t)attr.st_size;
    hprintf("mmaped buffer size: %lx\n",data_mmap_size);
  }
  run_vm();
}