from spec_lib.data_spec import *

#pip install -U Jinja2
import jinja2
import struct
import hashlib 
import time
import random
import os
import msgpack

class NodeType:
    def __init__(self, id, name, inputs, borrows, outputs, data):
        self.id = id
        self.name = name
        self.inputs = inputs
        self.borrows = borrows
        self.outputs = outputs
        self.size= 1+len(inputs)+len(borrows)+len(outputs)
        self.data=data
        self.args = []
        self.handler_code = ""
        for (i, arg) in enumerate(inputs):
            self.args.append((arg,"input_%d"%i))
        for (i, arg) in enumerate(borrows):
            self.args.append((arg,"borrow_%d"%i))
        for (i, arg) in enumerate(outputs):
            self.args.append((arg,"output_%d"%i))
    
    def msgpack(self):
        atomic_id = None
        if self.data:
            atomic_id = self.data.d_id
        return [self.name, atomic_id, [inp.id for inp in self.inputs], [bor.id for bor in self.borrows], [out.id for out in self.outputs] ]
    

class EdgeType:
    def __init__(self, id, name, c_type):
        self.id = id
        self.name = name
        self.c_type = c_type
    
    def msgpack(self):
        return [self.name]

class Spec:
    def __init__(self):
        self.nodes = []
        self.edges= []
        self.atomics = []
        self.name_to_node = {}
        timestamp = hex(int(time.time() * 1000))
        timestamp = timestamp.encode("ascii", "ignore")
        #self.checksum = 0x474e534b | (int(hashlib.md5(timestamp).hexdigest(), 16)&0xffffffff)<<32
        template_loader = jinja2.FileSystemLoader(searchpath=os.path.dirname(os.path.realpath(__file__)))
        self.tmpl = jinja2.Environment(loader=template_loader,undefined=jinja2.StrictUndefined)
        self.node_code_gen = None
        self.includes = []
        self.use_std_lib = True
        self.custom_defines = ""
        self.interpreter_user_data_type = None

    def get_checksum(self):
        data = [
            [n.msgpack() for n in self.nodes],
            [v.msgpack() for v in self.edges],
            [a.msgpack() for a in self.atomics],
        ]

        serialized_data = msgpack.packb(data)
        return 0x474e534b | (int(hashlib.md5(serialized_data).hexdigest(), 16)&0xffffffff)<<32

    
    def set_node_code_gen(self, fn):
        self.node_code_gen = fn

    def include(self, inc):
        self.includes.append(inc)

    def node_type(self, name, inputs=[], borrows=[], outputs=[], data=None, codegen_args=None):
        if data:
            data.emit_reader = True
        node = NodeType(len(self.nodes), name, inputs, borrows, outputs, data)
        self.nodes.append(node)
        self.name_to_node[name] = node
        if self.node_code_gen:
            node.handler_code = self.node_code_gen(node, codegen_args)
        return node

    def edge_type(self, name, c_type):
        value = EdgeType(len(self.edges), name, c_type)
        self.edges.append(value)
        return value

    def data_struct(self, name):
       dat = StructDataType(self, name) 
       return dat

    def finalize_struct(self, struct):
        struct.id = len(self.atomics)
        self.atomics.append(struct)
        return struct.id

    def data_int(self, name, size, generators=[]):
        dat = IntDataType(len(self.atomics), name, size, generators) 
        self.atomics.append(dat)
        return dat

    def data_vec(self, name, dtype, size_range):
        dat = VecDataType(len(self.atomics), name, dtype, size_range)
        self.atomics.append(dat)
        return dat

    def data_u8(self, name, generators=[]):
        return self.data_int(name, 1,generators);
    def data_u16(self, name, generators=[]):
        return self.data_int(name, 2,generators);
    def data_u32(self, name, generators=[]):
        return self.data_int(name, 4,generators);
    def data_u64(self, name, generators=[]):
        return self.data_int(name, 8,generators);

    def build_msgpack(self):
        return [ 
            self.get_checksum(),
            [n.msgpack() for n in self.nodes], 
            [v.msgpack() for v in self.edges],
            [a.msgpack() for a in self.atomics], 
        ]

    def build_interpreter(self, target_template_path="build/bytecode_spec.template.h"):

        template = self.tmpl.get_template("templates/interpreter.jinja.h")
        interpreter_h = template.render(interpreter_user_data_type=self.interpreter_user_data_type, custom_defines = self.custom_defines, use_std_lib=self.use_std_lib, values=self.edges, nodes=self.nodes, spec_checksum=self.get_checksum(), data_types=self.atomics, add_includes=self.includes)
        with open("build/interpreter.h","w") as f:
            f.write(interpreter_h)

        template = self.tmpl.get_template("templates/bytecode_spec.jinja.h")
        bytecode_spec_template_h  = template.render(interpreter_user_data_type=self.interpreter_user_data_type, values=self.edges, nodes=self.nodes, data_types=self.atomics, add_includes=self.includes)
        with open(target_template_path, "w") as f:
            f.write(bytecode_spec_template_h)
        
        template = self.tmpl.get_template("templates/data_include.jinja.h")
        bytecode_spec_template_h  = template.render( values=self.edges, nodes=self.nodes, data_types=self.atomics)
        with open("build/data_include.h", "w") as f:
            f.write(bytecode_spec_template_h)
