from spec_lib.graph_spec import *
from spec_lib.data_spec import *
from spec_lib.graph_builder import *


s = Spec()

t_a = s.edge_type("a", c_type="int");

d_int = s.data_u32("a_data")

n_path = s.node_type("start",   outputs=[t_a], data=d_int)
n_path = s.node_type("inc",     borrows=[t_a])
n_open = s.node_type("stop",    inputs=[t_a])

s.build_interpreter()

import msgpack
serialized_spec = s.build_msgpack()
with open("test_spec.msgp","wb") as f:
    f.write(msgpack.packb(serialized_spec))
