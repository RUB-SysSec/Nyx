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

def make_msix(s, base, msix_table_offset, msix_pba_offset, cap_pos, num_intrs, unaligned=False):
    PCI_MSIX_ENTRY_SIZE = 16
    for i in range((num_intrs*PCI_MSIX_ENTRY_SIZE)//4):
        s.set_node_code_gen(autogen_read_u32)
        s.node_type("read_msix_table_%d"%(base+msix_table_offset+(i*4)), codegen_args=(base+msix_table_offset+(i*4)))
        s.set_node_code_gen(autogen_write_u32)
        s.node_type("write_msix_table_%d"%(base+msix_table_offset+(i*4)), data=d_u32, codegen_args=(base+msix_table_offset+(i*4)))
        if unaligned: 
            for j in range(1, 4):
                s.set_node_code_gen(autogen_read_u32)
                s.node_type("read_msix_table_%d"%(base+msix_table_offset+(i*4)+j), codegen_args=(base+msix_table_offset+(i*4)+j))
                s.set_node_code_gen(autogen_write_u32)
                s.node_type("write_msix_table_%d"%(base+msix_table_offset+(i*4)+j), data=d_u32, codegen_args=(base+msix_table_offset+(i*4)+j))

    for i in range(num_intrs):
        s.set_node_code_gen(autogen_read_u32)
        s.node_type("read_msix_pba_%d"%(base+msix_pba_offset+(i*4)), codegen_args=(base+msix_pba_offset+(i*4)))
        s.set_node_code_gen(autogen_write_u32)
        s.node_type("write_msix_pba_%d"%(base+msix_pba_offset+(i*4)), data=d_u32, codegen_args=(base+msix_pba_offset+(i*4)))


def make_cap(s, base, cap_len):
    s.set_node_code_gen(autogen_read_u32) 
    if cap_len == 0x40:
        for addr in [base, base+0x04, base+0x08, base+0x0c, base+0x10, base+0x14, base+0x18, base+0x20, base+0x24, base+0x28, base+0x2c, base+0x30, base+0x34, base+0x38, base+0x3c]:
            s.node_type("read_xhci_mem_cap_%x"%addr, codegen_args=addr)
    elif cap_len == 0x20:
        for addr in [base, base+0x04, base+0x08, base+0x0c, base+0x10, base+0x14, base+0x18]:
            s.node_type("read_xhci_mem_cap_%x"%addr, codegen_args=addr)
    else:
        assert(False)

    s.set_node_code_gen(autogen_pass)


def make_oper(s, base, cap_len, t_trb_ring, t_cmd_ring, t_dcbaa_ptr, t_dcbaa):
    s.set_node_code_gen(autogen_read_u32)
    for (name,offset) in [("USBCMD", 0x00), ("USBSTS", 0x04), ("PAGESIZE", 0x08), ("DNCTRL", 0x14), ("CONFIG", 0x38)]:
        s.set_node_code_gen(autogen_read_u32)
        s.node_type("read_mem_oper_%s"%name, codegen_args=base+cap_len+offset)
        s.set_node_code_gen(autogen_write_u32)
        s.node_type("write_mem_oper_%s"%name, data=d_u32, codegen_args=base+cap_len+offset)

    s.set_node_code_gen(autogen_pass)

    set_crcr=""+\
        "output_0->addr = *borrow_0;\n"+\
        "output_0->offset = 0;\n"+\
      "*(volatile uint32_t*)(0x18+0x%x)= *borrow_0 /*| (*data_u8 & 0x3F)*/;\n"%(base+cap_len)+\
      "*(volatile uint32_t*)(0x18+4+0x%x)= 0;\n"%(base+cap_len)
    s.node_type("set_crcr", borrows=[t_trb_ring], outputs=[t_cmd_ring], codegen_args=set_crcr);
    
    set_dcbaap=""+\
      "*(volatile uint32_t*)(0x30+0x%x)= *borrow_0 /*| (*data_u8 & 0x3F)*/;\n"%(base+cap_len)+\
      "*(volatile uint32_t*)(0x30+4+0x%x)= 0;\n"%(base+cap_len) +\
      "*output_0 = *borrow_0;\n"
    s.node_type("set_dcbaap", borrows=[t_dcbaa_ptr], outputs=[t_dcbaa], codegen_args=set_dcbaap);
    
    
    set_dcbaap_broken_1=""+\
      "*(volatile uint32_t*)(0x30+0x%x)= 0x%x /*| (*data_u8 & 0x3F)*/;\n"%(base+cap_len, base)+\
      "*(volatile uint32_t*)(0x30+4+0x%x)= 0;\n"%(base+cap_len)
    s.node_type("set_dcbaap_broken_1", codegen_args=set_dcbaap_broken_1);

    set_dcbaap_broken_2=""+\
      "*(volatile uint32_t*)(0x30+0x%x)= 0xFFFFF000 /*| (*data_u8 & 0x3F)*/;\n"%(base+cap_len)+\
      "*(volatile uint32_t*)(0x30+4+0x%x)= 0;\n"%(base+cap_len)
    s.node_type("set_dcbaap_broken_2", codegen_args=set_dcbaap_broken_2);


def make_port(s, base, cap_len, max_ports, port_offset):
    s.set_node_code_gen(autogen_read_u32)

    for i in range(0,max_ports):
        for (name,offset) in [("PORTSC", 0x00), ("PORTPMSC", 0x04), ("PORTLI", 0x08), ("PORTLPM", 0x1C)]:
            s.set_node_code_gen(autogen_read_u32)
            s.node_type("read_mem_port_%s_%d"%(name,i), codegen_args=base+port_offset+(i*0x10)+offset)
            s.set_node_code_gen(autogen_write_u32)
            s.node_type("write_mem_port_%s_%d"%(name,i), data=d_u32, codegen_args=base+port_offset+(i*0x10)+offset)

    s.set_node_code_gen(autogen_pass)

def make_mem_runtime(s, base, max_intrs, t_cmd_ring, t_dma_offset):
    for i in range(0, max_intrs+2):
        """
        
        """
        
        set_erdp=""+\
        "*(volatile uint32_t*)(0x18+0x%x)= (uint32_t)(user->dma_buffer_phys_addr+*borrow_0);\n"%(base+(0x20*i))+\
        "*(volatile uint32_t*)(0x18+4+0x%x)= 0;\n"%(base+(0x20*i))
        s.node_type("set_erdp%d"%(i), borrows=[t_dma_offset, t_cmd_ring], codegen_args=set_erdp);
        
        

        set_erstba=""+\
        "*(volatile uint32_t*)(0x10+0x%x)= (uint32_t)(user->dma_buffer_phys_addr+*borrow_0);\n"%(base+(0x20*i))+\
        "*(volatile uint32_t*)(0x10+4+0x%x)= 0;\n"%(base+(0x20*i))
        s.node_type("set_erstba%d"%(i), borrows=[t_dma_offset, t_cmd_ring], codegen_args=set_erstba);
        
        for (name,offset) in [("IMAN", 0x00), ("IMOD", 0x04), ("ERSTSZ", 0x08)]:
            s.set_node_code_gen(autogen_write_u32)
            s.node_type("write_mem_runtime_%s_%s"%(name, str(i)), data=d_u32, borrows=[t_cmd_ring], codegen_args=(base+(0x20*i)+offset))
        s.set_node_code_gen(autogen_pass)
        
        
    s.set_node_code_gen(autogen_pass)


def make_doorbell(s, base, max_slots, t_cmd_ring):
    d_doorbell = s.data_struct("doorbell")
    d_doorbell.u8("index",[limits(0,max_slots+1)])
    d_doorbell.u16("stream_id")
    d_doorbell.u8("endpoint_id",[limits(0,36)])
    d_doorbell.finalize()
    
    mem_runtime_write = ""+\
    "volatile uint32_t* ptr = (uint32_t*)(0x%x+4*data_doorbell->index);\n"%base+\
    "ptr[0] = data_doorbell->index != 0 ? ((uint32_t)data_doorbell->stream_id)<<16 | ((uint32_t)data_doorbell->endpoint_id) : 0;\n"

    s.node_type("doorbell", data=d_doorbell, borrows=[t_cmd_ring], codegen_args=mem_runtime_write)

    mem_runtime_write_0 = ""+\
    "volatile uint32_t* ptr = (uint32_t*)(0x%x);\n"%base+\
    "ptr[0] = 0;\n"

    s.node_type("doorbell_0", borrows=[t_cmd_ring], codegen_args=mem_runtime_write_0)

def make_xhci_ring(s, base, cap_len):
    t_trb_ring = s.edge_type("trb_ring", c_type= "uint32_t")
    new_xhci_ring = " *output_0 = (uint32_t) slab_alloc_page_aligend(0x1000);\n" 
    s.node_type("new_trb_ring", outputs=[t_trb_ring], codegen_args=new_xhci_ring )

    t_cmd_ring = s.edge_type("cmd_ring", c_type= "struct { uint32_t addr; uint8_t offset; }")

    t_dma_offset = s.edge_type("dma_offset", c_type="size_t")
    t_dma_buff = s.edge_type("dma_buff", c_type= "struct { size_t offset; size_t len; }")
    t_trb = s.edge_type("trb", c_type="size_t")

    s.set_node_code_gen(autogen_pass)
    new_dma_offset_c="*output_0 = *data_offset;\n"
    s.node_type("new_dma_offset", outputs=[t_dma_offset],  data=s.data_u16("offset", [limits(0,0x1000*16)]), codegen_args=new_dma_offset_c)
    new_dma_buff_c="output_0->offset = *borrow_0;\n"+\
    "    output_0->len = data_raw->count;\n"+\
    "    for(size_t i=0; i< data_raw->count; i++){\n"+\
    "      ((uint8_t*)user->dma_buffer_virt_addr)[*borrow_0+i] = data_raw->vals[i];\n"+\
    "    }\n"

    s.node_type("new_dma_buff", outputs=[t_dma_buff], borrows=[t_dma_offset], data= s.data_vec("raw", s.data_u8("byte"), (0,16) ), codegen_args=new_dma_buff_c )

    d_trb_generic = s.data_struct("d_trb_generic")
    d_trb_generic.u32("data_a")
    d_trb_generic.u32("data_b")
    d_trb_generic.u8("type", [opts(1, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 32, 33, 34 ,35, 36, 37, 38, 39, 49, 50)])
    d_trb_generic.finalize()

    t_trb = s.edge_type("t_trb", c_type="size_t")
    trb_generic_c=""+\
      "uint32_t* trb = (uint32_t*)(borrow_1->addr + (0x10*(uint32_t)borrow_1->offset));\n"+\
      "borrow_1->offset++;\n"+\
      "trb[0] = user->dma_buffer_phys_addr+borrow_0->offset;\n"+\
      "trb[1] = 0;\n"+\
      "trb[2] = (uint32_t)data_d_trb_generic->data_a;\n"+\
      "trb[3] = (uint32_t)data_d_trb_generic->data_b;\n"+\
      "uint8_t* bytes = (uint8_t*)trb;\n"+\
      "bytes[13] = (bytes[13] & ~(0xFC)) | (data_d_trb_generic->type<<2);\n"

    s.node_type("trb_generic", borrows=[t_dma_buff, t_cmd_ring], outputs=[t_cmd_ring], data=d_trb_generic, codegen_args=trb_generic_c)

    """
    d_trb_link = s.data_struct("d_trb_link")
    d_trb_link.u64("data")
    d_trb_link.finalize()

    trb_link_c=""+\
      "uint32_t* trb = (uint32_t*)(borrow_1->addr + (0x10*(uint32_t)borrow_1->offset));\n"+\
      "borrow_1->offset++;\n"+\
      "trb[0] = (uint32_t)borrow_0->addr;\n"+\
      "trb[1] = 0;\n"+\
      "trb[2] = (uint32_t)data_d_trb_link->data;\n"+\
      "trb[3] = (uint32_t)(data_d_trb_link->data>>32);\n"+\
      "uint8_t* bytes = (uint8_t*)trb;\n"+\
      "bytes[13] = (bytes[13] & ~(0x3C)) | (6<<2);\n"

    s.node_type("trb_link", borrows=[t_cmd_ring, t_cmd_ring], outputs=[t_cmd_ring], data=d_trb_link, codegen_args=trb_link_c)
    """

    d_trb_setup = s.data_struct("d_trb_setup")
    d_trb_setup.u64("setup_data")
    d_trb_setup.u64("data")
    d_trb_setup.finalize()

    trb_setup_c=""+\
      "uint32_t* trb = (uint32_t*)(borrow_0->addr + (0x10*(uint32_t)borrow_0->offset));\n"+\
      "borrow_0->offset++;\n"+\
      "trb[0] = (uint32_t)data_d_trb_setup->setup_data;\n"+\
      "trb[1] = (uint32_t)(data_d_trb_setup->setup_data>>32);\n"+\
      "trb[2] = (uint32_t)data_d_trb_setup->data;\n"+\
      "trb[3] = (uint32_t)(data_d_trb_setup->data>>32);\n"+\
      "uint8_t* bytes = (uint8_t*)trb;\n"+\
      "bytes[13] = (bytes[13] & ~(0x3C)) | (2<<2);\n"

    s.node_type("trb_setup", borrows=[t_cmd_ring], outputs=[t_cmd_ring], data=d_trb_setup, codegen_args=trb_setup_c)

    return t_trb_ring, t_trb, t_cmd_ring, t_dma_offset

def make_dbaa(s, base, t_trb_ring):
    t_dcbaa = s.edge_type("dcbaa", c_type= "uint32_t")
    t_dcbaa_ptr = s.edge_type("dcbaa_ptr", c_type= "uint32_t")

    new_dcbaa_ptr = " *output_0 = (uint32_t) slab_alloc_page_aligend(0x1000); /* memset((void*)*output_0, 0x0, 0x1000);*/\n" 
    s.node_type("new_dcbaa_ptr", outputs=[t_dcbaa_ptr], codegen_args=new_dcbaa_ptr )

    """
    new_dcbaa_1 = " *output_0 = (uint32_t) 0x%x;\n"%(base) 
    s.node_type("new_dcbaa_broken_1", outputs=[t_dcbaa_ptr], codegen_args=new_dcbaa_1 )

    new_dcbaa_2 = " *output_0 = (uint32_t) 0xFFFFFFFFFFFFFF00;\n"
    s.node_type("new_dcbaa_broken_2", outputs=[t_dcbaa_ptr], codegen_args=new_dcbaa_2 )
    """
    
    t_dcb_ctx = s.edge_type("dcb_ctx", c_type= "uint32_t")
    new_dcb_ctx=" *output_0 = (uint32_t) slab_alloc_page_aligend(0x1000);\n" +\
        "uint32_t* dcbaa = (uint32_t* )*borrow_0;\n"+\
        "dcbaa[*data_slot] = *output_0;\n" 

    s.node_type("new_dcb_ctx", outputs=[t_dcb_ctx], borrows=[t_dcbaa], data=s.data_u8("slot"), codegen_args=new_dcb_ctx)

    new_dcb_ctx_broken_1="*((uint32_t**)borrow_0)[*data_u8] = (uint32_t)0x%x;\n"%(base)
    s.node_type("new_dcb_ctx_broken_1", borrows=[t_dcbaa], data=d_u8, codegen_args=new_dcb_ctx_broken_1)

    new_dcb_ctx_broken_2=" *((uint32_t**)borrow_0)[*data_u8] = (uint32_t)0xFFFFFFFFFFFFFF00;\n"
    s.node_type("new_dcb_ctx_broken_2", borrows=[t_dcbaa], data=d_u8, codegen_args=new_dcb_ctx_broken_2)
    
    
    d_slot_context = s.data_struct("d_slot_context")
    d_slot_context.u32("data_a")
    d_slot_context.u32("data_b")
    d_slot_context.u32("data_c")
    d_slot_context.u32("data_d")
    d_slot_context.finalize()

    new_slot_context = "" +\
      "*((uint32_t*)(*borrow_0 + 0)) = (uint32_t)data_d_slot_context->data_a;\n"+\
      "*((uint32_t*)(*borrow_0 + 4)) = (uint32_t)data_d_slot_context->data_b;\n"+\
      "*((uint32_t*)(*borrow_0 + 8)) = (uint32_t)data_d_slot_context->data_c;\n"+\
      "*((uint32_t*)(*borrow_0 + 12)) = (uint32_t)data_d_slot_context->data_d;\n"
    s.node_type("new_slot_context", borrows=[t_dcb_ctx], data=d_slot_context, codegen_args=new_slot_context)

    d_slot_context = s.data_struct("d_ep_context")
    d_slot_context.u32("data_a")
    d_slot_context.u32("data_b")
    d_slot_context.u32("data_c")
    d_slot_context.u8("ep_identifier")
    d_slot_context.finalize()

    new_ep_context = "" +\
      "*((uint32_t*)(*borrow_0 + 0  + (0x20 * (1+(uint32_t)data_d_ep_context->ep_identifier%0x30)))) = (uint32_t)data_d_ep_context->data_a;\n"+\
      "*((uint32_t*)(*borrow_0 + 4  + (0x20 * (1+(uint32_t)data_d_ep_context->ep_identifier%0x30)))) = (uint32_t)data_d_ep_context->data_b;\n"+\
      "*((uint32_t*)(*borrow_0 + 16 + (0x20 * (1+(uint32_t)data_d_ep_context->ep_identifier%0x30)))) = (uint32_t)data_d_ep_context->data_c;\n"+\
      "*((uint32_t*)(*borrow_0 + 8  + (0x20 * (1+(uint32_t)data_d_ep_context->ep_identifier%0x30)))) = *borrow_1;\n" + \
      "*((uint32_t*)(*borrow_0 + 12 + (0x20 * (1+(uint32_t)data_d_ep_context->ep_identifier%0x30)))) = 0;\n"
    s.node_type("new_ep_context", borrows=[t_dcb_ctx, t_trb_ring], data=d_slot_context, codegen_args=new_ep_context)
    

    return t_dcbaa_ptr, t_dcbaa


def make_xhci(s, base, cap_len, max_ports, max_intrs, max_slots, doorbell_offset, runtime_offset, port_offset, msix=False):
    make_globals(s)

    if(msix):
        make_msix(s, base, 0x3000, 0x3800, 0x90, 16)

    s.set_node_code_gen(autogen_pass)

    t_trb_ring, t_trb, t_cmd_ring, t_dma_offset = make_xhci_ring(s, base, cap_len)

    t_dcbaa_ptr, t_dcbaa = make_dbaa(s, base, t_trb_ring)


    make_cap(s, base, cap_len)
    make_oper(s, base, cap_len,  t_trb_ring, t_cmd_ring, t_dcbaa_ptr, t_dcbaa)
    make_port(s, base, cap_len, max_ports, port_offset)
    make_mem_runtime(s, base+runtime_offset, max_intrs, t_cmd_ring, t_dma_offset)
    make_doorbell(s, base+doorbell_offset, max_slots, t_cmd_ring)
