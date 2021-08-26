import sys
sys.path.insert(1, '../../structured_fuzzer/interpreter/')

from spec_lib.graph_spec import *
from spec_lib.data_spec import *
from spec_lib.graph_builder import *
from spec_lib.generators import opts,flags,limits

import jinja2

d_u32 = None
d_u16 = None
d_u8 = None

def make_globals(s):
  global d_u8, d_u16, d_u32
  d_u32 = s.data_u32("u32")
  d_u16 = s.data_u16("u16")
  d_u8 = s.data_u8("u8")

def autogen_write_32(node, arg):
    var = "data_write_mmio_%x_32"%arg
    return "*(volatile uint32_t*)(%s->addr)=%s->data;\n"%(var,var)

def autogen_read_u32(node, arg):
    return "volatile uint32_t tmp = *(volatile uint32_t*)(0x%x);\n(void)(tmp);\n"%arg

def autogen_write_u32(node, arg):
    return "*(volatile uint32_t*)(0x%x)=*data_u32;\n"%arg

def autogen_read_u16(node, arg):
    return "volatile uint16_t tmp = *(volatile uint16_t*)(0x%x);\n(void)(tmp);\n"%arg

def autogen_write_u16(node, arg):
    return "*(volatile uint16_t*)(0x%x)=*data_u16;\n"%arg

def autogen_read_u8(node, arg):
    return "volatile uint8_t tmp = *(volatile uint8_t*)(0x%x);\n(void)(tmp);\n"%arg

def autogen_write_u8(node, arg):
    return "*(volatile uint8_t*)(0x%x)=*data_u8;\n"%arg


def autogen_pass(node, arg):
    return arg



def make_ahci_host_regs(s, base):
    s.set_node_code_gen(autogen_read_u32)

    for (name,offset) in [("AHCI_HOST_REG_CAP", 0x00), \
                            ("AHCI_HOST_REG_CTL", 0x01), \
                            ("AHCI_HOST_REG_IRQ_STAT", 0x02), \
                            ("AHCI_HOST_REG_PORTS_IMPL", 0x03), \
                            ("AHCI_HOST_REG_VERSION", 0x04), \
                            ("AHCI_HOST_REG_CCC_CTL", 0x05), \
                            ("AHCI_HOST_REG_CCC_PORTS", 0x06), \
                            ("AHCI_HOST_REG_EM_LOC", 0x07), \
                            ("AHCI_HOST_REG_EM_CTL", 0x08), \
                            ("AHCI_HOST_REG_CAP2", 0x09), \
                            ("AHCI_HOST_REG_BOHC", 0x0a)]:
        s.set_node_code_gen(autogen_read_u32)
        s.node_type("read_mem_%s"%(name), codegen_args=base+(offset*4))
        s.set_node_code_gen(autogen_write_u32)
        s.node_type("write_mem_%s"%(name), data=d_u32, codegen_args=base+(offset*4))

    s.set_node_code_gen(autogen_pass)



def make_ahci_port_regs(s, base, ports_offset, max_ports, port_size, bhyve=False):

    s.set_node_code_gen(autogen_pass)

    #t_fis = s.edge_type("fis", c_type= "uint32_t")
    t_fis_ptr = s.edge_type("fis_ptr", c_type= "uint32_t")
    new_fis_ptr = "*output_0 = (uint32_t) slab_alloc_page_aligend(0x1000);\n" 
    s.node_type("new_fis_ptr", outputs=[t_fis_ptr], codegen_args=new_fis_ptr )

    #t_lst_ptr = s.edge_type("lst_ptr", c_type= "uint32_t")
    #new_lst_ptr = "*output_0 = (uint32_t) kvmalloc(0x1000);\n" 
    #s.node_type("new_lst_ptr", outputs=[t_lst_ptr], codegen_args=new_lst_ptr )


    t_cmd_lst_ptr = s.edge_type("cmd_lst_ptr", c_type= "uint32_t")
    new_cmd_list_ptr = " *output_0 = (uint32_t) slab_alloc_page_aligend(0x1000);\n" 
    s.node_type("new_cmd_list_ptr", outputs=[t_cmd_lst_ptr], codegen_args=new_cmd_list_ptr)

    t_cmd_lst = s.edge_type("cmd_lst", c_type= "uint32_t")


    
    if not bhyve:

        t_cmd_table = s.edge_type("cmd_table", c_type= "uint32_t")
        new_cmd_table = "*output_0 = (uint32_t) slab_alloc_page_aligend(0x1000);\n" + \
        "*((uint32_t**)output_0)[0] = data_d_cmd_table->cfis_0;\n" + \
        "*((uint32_t**)output_0)[1] = data_d_cmd_table->cfis_1;\n" + \
        "*((uint32_t**)output_0)[2] = data_d_cmd_table->cfis_2;\n" + \
        "*((uint32_t**)output_0)[3] = data_d_cmd_table->cfis_3;\n" + \
        "*((uint32_t**)output_0)[4] = data_d_cmd_table->cfis_4;\n" + \
        "*((uint32_t**)output_0)[5] = data_d_cmd_table->cfis_5;\n" + \
        "*((uint32_t**)output_0)[6] = data_d_cmd_table->cfis_6;\n" + \
        "*((uint32_t**)output_0)[7] = data_d_cmd_table->cfis_7;\n" + \
        "*((uint32_t**)output_0)[8] = data_d_cmd_table->cfis_8;\n" + \
        "*((uint32_t**)output_0)[9] = data_d_cmd_table->cfis_9;\n" + \
        "*((uint32_t**)output_0)[10] = data_d_cmd_table->cfis_10;\n" + \
        "*((uint32_t**)output_0)[11] = data_d_cmd_table->cfis_11;\n" + \
        "*((uint32_t**)output_0)[12] = data_d_cmd_table->cfis_12;\n" + \
        "*((uint32_t**)output_0)[13] = data_d_cmd_table->cfis_13;\n" + \
        "*((uint32_t**)output_0)[14] = data_d_cmd_table->cfis_14;\n" + \
        "*((uint32_t**)output_0)[15] = data_d_cmd_table->cfis_15;\n" + \
        "*((uint32_t**)output_0)[16] = data_d_cmd_table->acmd_0;\n" + \
        "*((uint32_t**)output_0)[17] = data_d_cmd_table->acmd_1;\n" + \
        "*((uint32_t**)output_0)[18] = data_d_cmd_table->acmd_2;\n" + \
        "*((uint32_t**)output_0)[19] = data_d_cmd_table->acmd_3;\n" 

        d_cmd_table = s.data_struct("d_cmd_table")
        d_cmd_table.u32("cfis_0")
        d_cmd_table.u32("cfis_1")
        d_cmd_table.u32("cfis_2")
        d_cmd_table.u32("cfis_3")
        d_cmd_table.u32("cfis_4")
        d_cmd_table.u32("cfis_5")
        d_cmd_table.u32("cfis_6")
        d_cmd_table.u32("cfis_7")
        d_cmd_table.u32("cfis_8")
        d_cmd_table.u32("cfis_9")
        d_cmd_table.u32("cfis_10")
        d_cmd_table.u32("cfis_11")
        d_cmd_table.u32("cfis_12")
        d_cmd_table.u32("cfis_13")
        d_cmd_table.u32("cfis_14")
        d_cmd_table.u32("cfis_15")
        d_cmd_table.u32("acmd_0")
        d_cmd_table.u32("acmd_1")
        d_cmd_table.u32("acmd_2")
        d_cmd_table.u32("acmd_3")

    else:

        t_cmd_table = s.edge_type("cmd_table", c_type= "uint32_t")
        new_cmd_table = "*output_0 = (uint32_t) slab_alloc_page_aligend(0x1000);\n" + \
        "*((uint32_t**)output_0)[0] = data_d_cmd_table->cfis_0;\n" + \
        "*((uint32_t**)output_0)[1] = data_d_cmd_table->cfis_1;\n" + \
        "*((uint32_t**)output_0)[2] = data_d_cmd_table->cfis_2;\n" + \
        "*((uint32_t**)output_0)[3] = data_d_cmd_table->cfis_3;\n" + \
        "*((uint32_t**)output_0)[4] = data_d_cmd_table->cfis_4;\n" + \
        "*((uint8_t**)output_0)[16] = data_d_cmd_table->acmd_0;\n" + \
        "*((uint8_t**)output_0)[17] = data_d_cmd_table->acmd_1;\n" + \
        "*((uint8_t**)output_0)[18] = data_d_cmd_table->acmd_2;\n" 

        d_cmd_table = s.data_struct("d_cmd_table")
        d_cmd_table.u32("cfis_0")
        d_cmd_table.u32("cfis_1")
        d_cmd_table.u32("cfis_2")
        d_cmd_table.u32("cfis_3")
        d_cmd_table.u32("cfis_4")
        d_cmd_table.u32("acmd_0")
        d_cmd_table.u32("acmd_1")
        d_cmd_table.u32("acmd_2")


    d_cmd_table.finalize()

    s.node_type("new_cmd_table", outputs=[t_cmd_table], data=d_cmd_table, codegen_args=new_cmd_table)



    d_cmd_list_entry = s.data_struct("d_cmd_list_entry")
    d_cmd_list_entry.u32("data_a")
    d_cmd_list_entry.u32("data_b")
    d_cmd_list_entry.finalize()
    t_cmd_lst_entry = s.edge_type("cmd_lst_entry", c_type= "struct { uint32_t dw0; uint32_t dw1; }")
    new_cmd_list_entry = " output_0->dw0 = (uint32_t)data_d_cmd_list_entry->data_a;\n" +\
    " output_0->dw1 = (uint32_t)data_d_cmd_list_entry->data_b;\n"
    s.node_type("new_cmd_list_entry", outputs=[t_cmd_lst_entry], data=d_cmd_list_entry, codegen_args=new_cmd_list_entry)



    t_cmd_table_obj = s.edge_type("cmd_table_obj", c_type= "struct{ uint32_t cmd_table; uint16_t* prdtl;}")


    add_cmd_list_entry=""+\
    "*((uint32_t**)borrow_0)[((*data_u8)%32)*0x20 + 0] = (borrow_1->dw0 & 0xFFFF) << 16;\n" +\
    "*((uint32_t**)borrow_0)[((*data_u8)%32)*0x20 + 1] = borrow_1->dw1;\n" +\
    "*((uint32_t**)borrow_0)[((*data_u8)%32)*0x20 + 2] = *borrow_2;\n" +\
    "*((uint32_t**)borrow_0)[((*data_u8)%32)*0x20 + 3] = 0x0;\n" +\
    "*((uint32_t**)borrow_0)[((*data_u8)%32)*0x20 + 4] = 0x0;\n" +\
    "*((uint32_t**)borrow_0)[((*data_u8)%32)*0x20 + 5] = 0x0;\n" +\
    "*((uint32_t**)borrow_0)[((*data_u8)%32)*0x20 + 6] = 0x0;\n" +\
    "*((uint32_t**)borrow_0)[((*data_u8)%32)*0x20 + 7] = 0x0;\n" +\
    "output_0->cmd_table = *borrow_2;\n" +\
    "output_0->prdtl = (uint16_t*)(*((uint32_t*)borrow_0) + ((*data_u8)%32)*0x20)+2;\n"
    s.node_type("add_cmd_list_entry", borrows=[t_cmd_lst, t_cmd_lst_entry, t_cmd_table], outputs=[t_cmd_table_obj], data=d_u8, codegen_args=add_cmd_list_entry);
    
    
    

    add_prdt_item="" +\
    "((uint32_t*)borrow_0->cmd_table)[0x80 + (*(borrow_0->prdtl)%248)] = (uint32_t) slab_alloc_page_aligend(0x1000);\n" +\
    "((uint32_t*)borrow_0->cmd_table)[0x80 + 1 + (*(borrow_0->prdtl)%248)] = 0;\n" +\
    "((uint32_t*)borrow_0->cmd_table)[0x80 + 2 + (*(borrow_0->prdtl)%248)] = 0;\n" +\
    "((uint32_t*)borrow_0->cmd_table)[0x80 + 3 + (*(borrow_0->prdtl)%248)] = *data_u32;\n" +\
    "*(borrow_0->prdtl) += 1;\n"
    s.node_type("add_prdt_item", borrows=[t_cmd_table_obj], data=d_u32, codegen_args=add_prdt_item);
    

    for i in range(max_ports):
        
        for (name,offset) in [ \
                                ("AHCI_PORT_REG_LST_ADDR", 0x00),
                                ("AHCI_PORT_REG_LST_ADDR_HI", 0x01),
                                ("AHCI_PORT_REG_FIS_ADDR", 0x02), \
                                ("AHCI_PORT_REG_FIS_ADDR_HI", 0x03), \
                                ("AHCI_PORT_REG_IRQ_STAT", 0x04), \
                                ("AHCI_PORT_REG_IRQ_MASK", 0x05), \
                                ("AHCI_PORT_REG_CMD", 0x06), \
                                ("AHCI_PORT_REG_TFDATA", 0x08), \
                                ("AHCI_PORT_REG_SIG", 0x09), \
                                ("AHCI_PORT_REG_SCR_STAT", 10), \
                                ("AHCI_PORT_REG_SCR_CTL", 11), \
                                ("AHCI_PORT_REG_SCR_ERR", 12), \
                                ("AHCI_PORT_REG_SCR_ACT", 13), \
                                ("AHCI_PORT_REG_CMD_ISSUE", 14), \
                                ("AHCI_PORT_REG_SCR_NOTIF", 15), \
                                ("AHCI_PORT_REG_FIS_CTL", 16), \
                                ("AHCI_PORT_REG_DEV_SLEEP", 17), \
                                ("AHCI_PORT_REG_VENDOR_1", 28), \
                                ("AHCI_PORT_REG_VENDOR_2", 29), \
                                ("AHCI_PORT_REG_VENDOR_3", 30), \
                                ("AHCI_PORT_REG_VENDOR_4", 31)]:
            s.set_node_code_gen(autogen_read_u32)
            s.node_type("read_mem_%s_%d"%(name,i), codegen_args=base+ports_offset+(i*port_size)+(offset*4))

            if(offset > 4):
                s.set_node_code_gen(autogen_write_u32)
                s.node_type("write_mem_%s_%d"%(name,i), data=d_u32, codegen_args=base+ports_offset+(i*port_size)+(offset*4))

        s.set_node_code_gen(autogen_pass)
        
        set_fis=""+\
        "*(volatile uint32_t*)(0x%x)= *borrow_0;\n"%(base+ports_offset+(i*port_size)+8)+\
        "*(volatile uint32_t*)(0x%x+4)= 0;\n"%(base+ports_offset+(i*port_size)+8)
        s.node_type("set_fis_%d"%(i), borrows=[t_fis_ptr], codegen_args=set_fis);
        
        
        set_fis2=""+\
        "*(volatile uint32_t*)(0x%x)= 0x%x;\n"%(base+ports_offset+(i*port_size)+8, base)+\
        "*(volatile uint32_t*)(0x%x+4)= 0;\n"%(base+ports_offset+(i*port_size)+8)
        s.node_type("set_fis2_%d"%(i), codegen_args=set_fis2);

        set_fis3=""+\
        "*(volatile uint32_t*)(0x%x)= 0xFFFFFF00;\n"%(base+ports_offset+(i*port_size)+8)+\
        "*(volatile uint32_t*)(0x%x+4)= 0;\n"%(base+ports_offset+(i*port_size)+8)
        s.node_type("set_fis3_%d"%(i), codegen_args=set_fis3);
        

        set_lst=""+\
        "*(volatile uint32_t*)(0x%x)= *borrow_0;\n"%(base+ports_offset+(i*port_size)+0)+\
        "*(volatile uint32_t*)(0x%x+4)= 0;\n"%(base+ports_offset+(i*port_size)+0) +\
        "*output_0 = *borrow_0;\n"
        s.node_type("set_lst_%d"%(i), borrows=[t_cmd_lst_ptr], outputs=[t_cmd_lst], codegen_args=set_lst);
        
        
        set_lst2=""+\
        "*(volatile uint32_t*)(0x%x)= 0x%x;\n"%(base+ports_offset+(i*port_size)+0, base)+\
        "*(volatile uint32_t*)(0x%x+4)= 0;\n"%(base+ports_offset+(i*port_size)+0)
        s.node_type("set_lst2_%d"%(i), codegen_args=set_lst2);

        set_lst3=""+\
        "*(volatile uint32_t*)(0x%x)= 0xFFFFFF00;\n"%(base+ports_offset+(i*port_size)+0)+\
        "*(volatile uint32_t*)(0x%x)= 0;\n"%(base+ports_offset+(i*port_size)+0)
        s.node_type("set_lst3_%d"%(i), codegen_args=set_lst3);
        


    s.set_node_code_gen(autogen_pass)

def make_ahci(s, mmio_base, pio_base, ahci_port_start=0x100, ahci_port_offset_len=0x80, max_ports=32, bhyve=False):
    make_globals(s)
    make_ahci_host_regs(s, mmio_base)
    make_ahci_port_regs(s, mmio_base, ahci_port_start, 2, 0x80, bhyve=bhyve)
