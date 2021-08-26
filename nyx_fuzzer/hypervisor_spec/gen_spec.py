import sys
sys.path.insert(1, '../structured_fuzzer/interpreter/')

from spec_lib.graph_spec import *
from spec_lib.data_spec import *
from spec_lib.graph_builder import *
from spec_lib.generators import opts,flags,limits

import jinja2

from hexa_spec.legacy import make_legacy_pcnet, make_legacy_rtl8139, make_legacy_e1000, make_legacy_ee100pro, make_legacy_sdhci, make_legacy_intel_hda, make_legacy_ac97, make_legacy_ide_core, make_legacy_floppy, make_legacy_parallel, make_legacy_serial, make_legacy_cs4231a, make_legacy_xhci

from hexa_spec.ahci import make_ahci

from hexa_spec.xhci import make_xhci

s = Spec()

s.use_std_lib = False
s.custom_defines=""+ \
"#define ASSERT(x) assert(x)\n"+\
"#define VM_MALLOC(x) kvmalloc(x)\n"

s.includes.append("\"custom_includes.h\"")

s.interpreter_user_data_type = "hypertrash_context_t*"

if len(sys.argv) == 2:

    target = sys.argv[1]

    if target == "bhyve_ahci":
        make_ahci(s, 0x824000, 0, bhyve=True)
    elif target == "qemu_xhci":
        make_xhci(s, 0x1821000, 0x40, 30, 16, 0, 0x2000, 0x1000, 0x440, msix=True)
    elif target == "legacy_xhci":
        make_legacy_xhci(s)
    elif target == "legacy_pcnet":
        make_legacy_pcnet(s)
    elif target == "legacy_rtl8139":
        make_legacy_rtl8139(s)
    elif target == "legacy_e1000":
        make_legacy_e1000(s)
    elif target == "legacy_ee100pro":
        make_legacy_ee100pro(s)
    elif target == "legacy_sdhci":
        make_legacy_sdhci(s)
    elif target == "legacy_intel_hda":
        make_legacy_intel_hda(s)
    elif target == "legacy_ide_core":
        make_legacy_ide_core(s)
    elif target == "legacy_floppy":
        make_legacy_floppy(s)
    elif target == "legacy_parallel":
        make_legacy_parallel(s)
    elif target == "legacy_serial":
        make_legacy_serial(s)
    elif target == "legacy_cs4231a":
        make_legacy_cs4231a(s)
    elif target == "legacy_ac97":
        make_legacy_ac97(s)
    else:
        sys.exit(0)

    s.build_interpreter()

    import msgpack
    serialized_spec = s.build_msgpack()
    with open("spec.msgp","wb") as f:
        f.write(msgpack.packb(serialized_spec))


