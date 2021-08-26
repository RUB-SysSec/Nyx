//ALL OF THIS IS AUTO GENERATED, DO NOT CHANGE. CHANGE interpreter.jinja.h and regenerate!
#ifndef __INTERPRETER__GEN__
#define __INTERPRETER__GEN__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" 

#include<stdint.h>
#include<stddef.h>

//includes
{% for inc in add_includes %}
#include {{inc}}
{% endfor %}

{% if use_std_lib %}
#include<stdlib.h>
#include<assert.h>
#define ASSERT(x) assert(x)
#define VM_MALLOC(x) malloc(x)
{% endif %}


#define STATIC_ASSERT(cond, desc) _Static_assert(cond, desc)

#define INTERPRETER_CHECKSUM {{spec_checksum}}ULL

{{ custom_defines }}

{%for ntype in nodes %}
#define OP_{{ntype.name|upper}} {{ntype.id}}
#define OP_{{ntype.name|upper}}_SIZE {{ntype.size}}
{%- if ntype.data and ntype.data.fixed_size %}
#define DATA_{{ntype.name|upper}}_SIZE {{ntype.data.size}}
{%-endif %}
{% endfor %}

#include "data_include.h"

typedef struct {
	uint16_t* ops;
	size_t ops_len;
	size_t ops_i;

	uint8_t* data;
	size_t data_len;
	size_t data_i;

	{% if interpreter_user_data_type -%}
		{{interpreter_user_data_type}} user_data;
	{%- endif -%}

{% for ttype in values %}
	t_{{ttype.name}}* t_{{ttype.name}}_vals;
{%- endfor %}

} interpreter_t;


#include "bytecode_spec.h"

//=========================
//atomic data type functions
//=========================

{% for dtype in data_types %}
{%- if dtype.emit_reader %}
	{%- if dtype.fixed_size -%}
{{dtype.c_type_name}}* read_{{dtype.c_type_name}}(interpreter_t* vm){
	ASSERT(vm->data_i + sizeof({{dtype.c_type_name}}) <= vm->data_len);
	STATIC_ASSERT(sizeof({{dtype.c_type_name}}) == {{dtype.size}}, "size missmatch in d_{{dtype.name}}");
	{{dtype.c_type_name}}* res = ({{dtype.c_type_name}}*)&vm->data[vm->data_i];
	vm->data_i += sizeof({{dtype.c_type_name}});
	{%-else -%}
{{dtype.c_type_name}} read_{{dtype.c_type_name}}(interpreter_t* vm){
	{{dtype.c_type_name}} res = {0};
	ASSERT(vm->data_i+2 <= vm->data_len);
	uint16_t byte_size = *((uint16_t*)&vm->data[vm->data_i]);
	vm->data_i+=2;
	ASSERT(vm->data_i+byte_size <= vm->data_len);
	res.count = ((size_t)byte_size)/sizeof({{dtype.dtype.c_type_name}});
	res.vals = ({{dtype.dtype.c_type_name}}*)&(vm->data[vm->data_i+2]);
	vm->data_i += byte_size;
	{% endif %}
	return res;
}

{%endif-%}
{% endfor %}
//=========================
//edge type functions
//=========================
{% for ttype in values %}
t_{{ttype.name}}* get_t_{{ttype.name}}(interpreter_t* vm, uint16_t op_id){
	return &vm->t_{{ttype.name}}_vals[op_id];
}
{% endfor %}

//=========================
//interpreter functions
//=========================

interpreter_t* new_interpreter(){
	interpreter_t* vm = VM_MALLOC(sizeof(interpreter_t));
	vm->ops = 0;
	vm->ops_len = 0;
	vm->data = 0;
	vm->data_len = 0;

{% for ttype in values %}
	vm->t_{{ttype.name}}_vals = VM_MALLOC(sizeof(t_{{ttype.name}})*0xffff);
{%- endfor %}

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

void interpreter_run(interpreter_t* vm){
	ASSERT(vm->ops && vm->data && vm->ops_i == 0 && vm->data_i == 0); //init_interpreter was called previously
	while(vm->ops_i < vm->ops_len) {
		uint16_t op = vm->ops[vm->ops_i];
		switch(op){
{% for ntype in nodes %}
			case OP_{{ntype.name|upper}}: {
				ASSERT( vm->ops_len >= vm->ops_i + OP_{{ntype.name|upper}}_SIZE );
				{%- if ntype.data -%}
					{{ntype.data.c_type_name}} {{- "* " if ntype.data.fixed_size else " " -}} data = read_{{ntype.data.c_type_name}}(vm);
				{% endif %}
				handler_{{ntype.name}}(
				{%- if interpreter_user_data_type -%}
					vm->user_data
					{{-", " if ntype.data or ntype.args|length > 0 else "" -}}
				{%- endif -%}
				{%- if ntype.data -%}
					{{- "&" if not ntype.data.fixed_size else "" -}}data
					{{-", " if ntype.args|length > 0 else "" -}}
				{% endif -%}
				{%- for (val,varname) in ntype.args -%}
				get_t_{{val.name}}(vm, vm->ops[vm->ops_i+{{loop.index}}])
				{{- ", " if not loop.last else ""-}}
				{%- endfor -%}
						);
				vm->ops_i += OP_{{ntype.name|upper}}_SIZE;

				break;}
{%- endfor %}
			default:
					ASSERT(0);
		}
	}
}
#pragma GCC diagnostic pop 
#endif
