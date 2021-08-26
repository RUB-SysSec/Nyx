from spec_lib.graph_spec import *
from spec_lib.data_spec import *

import string

class BuilderValue:
    def __init__(self, value_type, id):
        self.value_type = value_type
        self.id = id
        self.unused=True

class Builder:
    def __init__(self, spec):
        self.spec = spec
        self.ops = []
        self.data = b""
        self.value_to_id = {}
        for n in spec.nodes:
            def build(node):
                return lambda *args: self.build_node(node.name, *args)
            setattr(self, n.name, build(n))

    def build_node(self, name, *args):
        node = self.spec.name_to_node[name]
        assert(len(args) == len(node.inputs)+len(node.borrows))
        self.ops.append(node.id)
        for (i,val) in enumerate(node.inputs):
            assert(args[i].value_type == val)
            assert(args[i].unused)
            self.ops.append(args[i].id)
            args[i].unused=False
        for(i, val) in enumerate(node.borrows):
            j = len(node.inputs)+i
            assert(args[j].value_type == val)
            assert(args[j].unused)
            self.ops.append(args[j].id)

        self.build_data(node.data)

        res = []
        for val in node.outputs:
            b_val = self.build_value(val)
            res.append(b_val)
            self.ops.append(b_val.id)
        if len(res) == 1:
            return res[0]
        return res

    def build_data(self,dat):
        if not dat:
            return
        if not dat.fixed_size:
            assert(len(dat.size_range)==2)
            n = random.randrange(dat.size_range[0], dat.size_range[1])
            self.data+=struct.pack("H",n)
        else:
            n = dat.size
        print("generate data: %d %s"%(n,repr(dat)))
        data = bytes(''.join(random.choice(string.ascii_uppercase + string.digits) for i in range(n)), "ascii")
        self.data += data


    def build_value(self, val):
        if val not in self.value_to_id:
            self.value_to_id[val] = 0
        vid = self.value_to_id[val]
        self.value_to_id[val]+=1
        return BuilderValue(val, vid)

    def write_to_file(self, path):
        graph_size = len(self.ops)
        data_size = len(self.data)
        base = 5*8
        graph_offset = base
        data_offset = graph_offset+2*graph_size
        header = struct.pack("<QQQQQ", self.spec.get_checksum(), graph_size, data_size, graph_offset, data_offset)
        graph_data = struct.pack("H"*len(self.ops),*self.ops)
        with open(path,"wb") as f:
            f.write(header)
            f.write(graph_data)
            f.write(self.data)

