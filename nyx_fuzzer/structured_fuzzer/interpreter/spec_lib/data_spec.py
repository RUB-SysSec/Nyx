import os

class StructDataType:
    def __init__(self, spec, name):
        self.spec = spec
        self.name = name
        self.emit_reader = False
        self.fixed_size = True
        self.size=0
        self.fields = []
        self.c_type_name = "d_"+name
        self.c_type_def = self.get_c_type_def()
    
    def finalize(self):
        self.d_id = self.spec.finalize_struct(self)
        self.spec = None

    def get_c_type_def(self):
        template = self.spec.tmpl.get_template("templates/data_struct.jinja.h")
        return template.render(struct=self)

    def field(self, name, dtype):
        self.size += dtype.size
        self.fields.append((name, dtype))
        self.c_type_def = self.get_c_type_def()
        return self

    def u64(self, name, generators=[]):
        field_type = self.spec.data_u64(self.name+"_"+name, generators)
        self.field(name, field_type)
        return field_type

    def u32(self, name, generators=[]):
        field_type = self.spec.data_u32(self.name+"_"+name, generators)
        self.field(name, field_type)
        return field_type

    def u16(self, name,generators=[]):
        field_type = self.spec.data_u16(self.name+"_"+name,generators)
        self.field(name, field_type)
        return field_type

    def u8(self, name,generators=[]):
        field_type = self.spec.data_u8(self.name+"_"+name,generators)
        self.field(name, field_type)
        return field_type
    
    def msgpack(self):
        return ["Struct", self.name, [[name,dtype.d_id] for (name,dtype) in self.fields]]


class IntDataType:
    def __init__(self, d_id, name, size, generators=[]):
        assert(size in [1,2,4,8])
        self.d_id = d_id
        self.name = name
        self.emit_reader = False
        self.fixed_size = True
        self.size = size
        self.generators = generators
        self.c_type_name = "d_"+self.name
        self.c_type_def = "typedef %s %s;"%("uint%d_t"%(size*8), self.c_type_name)
    
    def msgpack(self):
        print(repr((self.name,self.c_type_name,self.generators)))
        return ["Int", self.name, self.size,[g.msgpack() for g in self.generators]]


class VecDataType:
    def __init__(self, d_id, name, dtype, size_range):
        assert(dtype.fixed_size);
        self.d_id = d_id
        self.name = name
        self.emit_reader = False
        self.fixed_size = False
        self.size_range = size_range
        self.dtype = dtype
        self.c_type_name = "d_vec_"+self.name
        self.c_type_def = "typedef struct {size_t count; %s* vals; } %s;"%(self.dtype.c_type_name, self.c_type_name)
    
    def msgpack(self):
        return ["Vec", self.name, [self.size_range[0], self.size_range[1]], self.dtype.d_id]

class ArrayDataType:
    def __init__(self, d_id, name, dtype, count):
        assert(dtype.fixed_size);
        self.d_id = d_id
        self.name = name
        self.emit_reader = False
        self.fixed_size = True
        self.size = count*dtype.size
        self.dtype = dtype