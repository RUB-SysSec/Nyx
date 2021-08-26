//-------------------------
// Move this file to bytecode_spec.h
// and implemented in user spec part

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "data_include.h"

/**

typedef char* t_path;

typedef int t_fd;

typedef struct{ void* data; size_t size} t_mmap_buffer;

**/


void handler_path(d_vec_path_string path_string, t_path* output_0){
	printf("path: %.*s\n",path_string.count, path_string.vals);
	*output_0 = "/etc/passwd";
}

void handler_open(t_path* borrow_0, t_fd* output_0){
	*output_0 = open(*borrow_0, O_RDONLY);
	printf("open(%s) = %d\n",*borrow_0, *output_0);
}

void handler_fd_0(t_fd* output_0){
	printf("fd:0\n");
	*output_0 = 0;
}

void handler_dup2(t_fd* borrow_0, t_fd* borrow_1){
	printf("dup2( %d, %d)\n", *borrow_0, *borrow_1);
	dup2(*borrow_0, *borrow_1);
}

void handler_mmap(d_flags flags, t_fd* borrow_0, t_mmap_buffer* output_0){
	printf("nope no mmpa\n");
}

void handler_close(t_fd* input_0){
	printf("close(%d)\n", *input_0);
	close(*input_0);
}

void handler_read(t_fd* borrow_0){
	printf("read from %d\n", *borrow_0);
}
