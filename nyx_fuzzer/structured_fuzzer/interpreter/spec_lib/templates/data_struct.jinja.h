
#pragma pack(1)
typedef struct {
    {%- for (field_name, field_type) in struct.fields %}
        {{field_type.c_type_name}} {{field_name}};
    {%- endfor %}
} {{struct.c_type_name}};