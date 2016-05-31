#/usr/bin/env python3

"""
Extract comminst definitions from common-insts.rest into C macros.

USAGE: python3 script/extract_comminst_macros.py < common-inst.rest
"""

import re
import sys

pat = re.compile(r'\[(0x[0-9a-f]+)\]@([a-zA-Z0-9_.]+)', re.MULTILINE)

defs = []
longest = 0

text = sys.stdin.read()

for opcode, name in pat.findall(text):
    macro_name = "MU_CI_" + name.upper().replace(".", "_")
    opcode = opcode.upper()
    defs.append((macro_name, opcode))
    longest = max(longest, len(macro_name))

for macro_name, opcode in defs:
    print("#define {} {}".format(macro_name.ljust(longest), opcode))

