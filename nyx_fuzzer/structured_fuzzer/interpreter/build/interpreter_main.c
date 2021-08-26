#include <stdio.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "interpreter.h"

uint8_t* mmap_file(const char* path, size_t* size){
   uint8_t *memblock;

   int fd;
   struct stat sb;

   fd = open(path, O_RDWR);
   fstat(fd, &sb);
   //printf("Size: %lu\n", (uint64_t)sb.st_size);

   memblock = mmap(NULL, sb.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
	 if(memblock==(void*)-1){
        perror("mmap failed");
				exit(0);
	 }
	
	 *size = sb.st_size;
	 return memblock;
}

void main(int argc, char** argv){
	assert(argc >= 2);
	size_t size = 0;
	uint8_t* mem = mmap_file(argv[1], &size);
	uint64_t* offsets = (uint64_t*)mem;
	assert(size > sizeof(uint64_t)*5);
	assert(offsets[0] == INTERPRETER_CHECKSUM);
	assert(offsets[1] < 0xffffff);
	assert(offsets[2] < 0xffffff);
	assert(offsets[3] < 0xffffff);
	assert(offsets[4] < 0xffffff);
	uint64_t graph_size = offsets[1];
	uint64_t data_size = offsets[2];
	uint16_t* graph_ptr = (uint16_t*)(mem+offsets[3]);
	uint8_t* data_ptr = (uint8_t*)(mem+offsets[4]);
	assert(offsets[3]+graph_size*sizeof(uint16_t) <= size);
	assert(offsets[4]+data_size <= size);
	interpreter_t* vm = new_interpreter();
	init_interpreter(vm, graph_ptr, graph_size, data_ptr, data_size);
  interpreter_user_init(vm);
	run(vm);
  interpreter_user_shutdown(vm);
}
