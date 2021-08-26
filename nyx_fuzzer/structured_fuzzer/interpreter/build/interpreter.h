//ALL OF THIS IS AUTO GENERATED, DO NOT CHANGE. CHANGE interpreter.jinja.h and regenerate!
#ifndef __INTERPRETER__GEN__
#define __INTERPRETER__GEN__

#include<stdint.h>
#include<stddef.h>
#include<assert.h>
#include<stdlib.h>

#define ASSERT(x) assert(x)
#define STATIC_ASSERT(cond, desc) _Static_assert(cond, desc)

#define INTERPRETER_CHECKSUM 17206576842326037323ULL


#define OP_DATA 0
#define OP_DATA_SIZE 1


#include "data_include.h"

typedef struct {
	uint16_t* ops;
	size_t ops_len;
	size_t ops_i;

	uint8_t* data;
	size_t data_len;
	size_t data_i;



} interpreter_t;


#include "bytecode_spec.h"

//=========================
//atomic data type functions
//=========================

d_vec_d_bytes read_d_vec_d_bytes(interpreter_t* vm){
	d_vec_d_bytes res = {0};
	ASSERT(vm->data_i+2 <= vm->data_len);
	uint16_t byte_size = *((uint16_t*)&vm->data[vm->data_i]);
	vm->data_i+=2;
	ASSERT(vm->data_i+byte_size <= vm->data_len);
	res.count = ((size_t)byte_size)/sizeof(d_u8);
	res.vals = (d_u8*)&(vm->data[vm->data_i+2]);
	vm->data_i += byte_size;
	
	return res;
}


//=========================
//edge type functions
//=========================


//=========================
//interpreter functions
//=========================

interpreter_t* new_interpreter(){
	interpreter_t* vm = malloc(sizeof(interpreter_t));
	vm->ops = 0;
	vm->ops_len = 0;
	vm->data = 0;
	vm->data_len = 0;



	return vm;
}

void init_interpreter(interpreter_t* vm, uint16_t* ops, size_t ops_len, uint8_t* data, size_t data_len){
	vm->ops=ops;
	vm->ops_len=ops_len;
	vm->ops_i = 0;
	vm->data = data;
	vm->data_i = 0;
	vm->data_len=data_len;
}

void run(interpreter_t* vm){
	ASSERT(vm->ops && vm->data && vm->ops_i == 0 && vm->data_i == 0); //init_interpreter was called previously
	while(vm->ops_i < vm->ops_len) {
		uint16_t op = vm->ops[vm->ops_i];
		switch(op){

			case OP_DATA: 
				ASSERT( vm->ops_len >= vm->ops_i + OP_DATA_SIZE );
				handler_data(read_d_vec_d_bytes(vm));
				vm->ops_i += OP_DATA_SIZE;

				break;
			default:
					ASSERT(0);
		}
	}
}

#endif