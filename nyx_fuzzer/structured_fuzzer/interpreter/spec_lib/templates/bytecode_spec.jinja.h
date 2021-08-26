//-------------------------
// Move this file to bytecode_spec.h
// and implemented in user spec part

//includes
{% for inc in add_includes %}
#include {{inc}}
{% endfor %}

/**
{% for ttype in values %}
typedef {{ttype.c_type}} t_{{ttype.name}};
{% endfor %}
**/

void interpreter_user_init(interpreter_t *vm){
}

void interpreter_user_shutdown(interpreter_t *vm){
}

{% for ntype in nodes %}
void handler_{{ntype.name}}(
		{%- if interpreter_user_data_type -%}
			{{interpreter_user_data_type}} user
			{{-", " if ntype.data or ntype.args|length > 0 else "" -}}
		{%- endif -%}
		{%- if ntype.data -%}
			{{ntype.data.c_type_name}} *data_{{ntype.data.name}}
			{{-", " if ntype.args|length > 0 else "" -}}
		{% endif -%}
		{%- for (val,varname) in ntype.args -%}
		t_{{val.name}}* {{varname}}
		{{- ", " if not loop.last else ""-}}
		{%- endfor -%}
	){
	{{ ntype.handler_code -}}
}
{% endfor %}
