from spec_lib.graph_spec import *
from spec_lib.data_spec import *
from spec_lib.graph_builder import *


s = Spec()

d_bytes = s.data_vec("d_bytes", s.data_u8("u8"), size_range=(0,1<<4))

n_path = s.node_type("data", data=d_bytes)

s.build_interpreter()

import msgpack
serialized_spec = s.build_msgpack()
with open("spec_vec_u8.msgp","wb") as f:
    f.write(msgpack.packb(serialized_spec))
