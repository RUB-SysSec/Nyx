from spec_lib.graph_spec import *
from spec_lib.data_spec import *
from spec_lib.graph_builder import *
from spec_lib.generators import opts

s = Spec()

t_path = s.edge_type("path", c_type="char*");
t_fd = s.edge_type("fd", c_type="int");
t_mmap_buffer = s.edge_type("mmap_buffer", c_type="struct{ void* data; size_t size; }");

#d_string = s.data_type("string", size=(0,10) );

OPT_FOO = 1
OPT_BAR = 42
OPT_BAZ = 1337

#d_flags = s.data_u32("flags")
d_flags = s.data_u32("flags", [opts(OPT_FOO,OPT_BAR,OPT_BAZ)] )
d_char = s.data_u8("char")

d_string = s.data_vec("path_string", d_char, (5,100))

d_foo = s.data_struct("foo")
d_foo.u8("bla")
d_foo.field("flags",d_flags)
d_foo.u32("fuu")
d_foo.finalize()

n_path = s.node_type("path",    outputs=[t_path], data=d_string)
n_open = s.node_type("open",    borrows=[t_path], outputs=[t_fd])
n_stdo = s.node_type("fd_0",    outputs=[t_fd])
n_dup2 = s.node_type("dup2",    borrows=[t_fd, t_fd])
n_mmap = s.node_type("mmap",    borrows=[t_fd], outputs=[t_mmap_buffer], data=d_flags)
n_close = s.node_type("close",  inputs=[t_fd])
n_read = s.node_type("read",    borrows=[t_fd])

s.build_interpreter()

import msgpack
serialized_spec = s.build_msgpack()
with open("example_spec.msgp","wb") as f:
    f.write(msgpack.packb(serialized_spec))

b = Builder(s)

p = b.path()
fd1 = b.open(p)
fd2 = b.fd_0()
b.dup2(fd1,fd2)
b.read(fd2)
b.mmap(fd2)

b.write_to_file("build/input.bin")
print(repr(b.ops))

