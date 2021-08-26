{% for ttype in values %}
typedef {{ttype.c_type}} t_{{ttype.name}};
{% endfor %}

{%- for dtype in data_types %}
{{dtype.c_type_def}}
{%- endfor %}