#!/usr/bin/env python2.7

from collections import *
from subprocess import *
import re

def disassembly_at(original_program, addr, nbytes):
    out = check_output(['objdump',
        '--start-address', hex(addr), '--stop-address', hex(addr+nbytes),
        '-d', original_program])
    #print '>>> objdump full output:'; print out; print '<<<'; print

    dis = OrderedDict()
    for l in out.splitlines():
        if not l: continue
        m = re.match(r'\s*([0-9a-fA-F]+):\s+([0-9a-fA-F].*)', l)
        if m:
            addr = int(m.group(1),16)
            code = m.group(2)
            dis[addr] = code
            print '{}: {}'.format(addr,code) 
            continue
        print '  info:', l
    return dis


def postbreak_instruction(original_program, wanted_addr, brk_opcode_len):
    base = disassembly_at(original_program, wanted_addr, 40)

    print
    
    assert base.keys()[0] == wanted_addr
    original_instruction = base[wanted_addr]
    print "\nOriginal instruction:\n{}: {}".format(hex(wanted_addr), original_instruction)

    postbreak_addr = None
    for addr in base.keys()[1:]:
        if addr >= (wanted_addr + brk_opcode_len):
            postbreak_addr = addr
            postbreak_instruction = base[addr]
            break
        print "Also covered by the main breakpoint opcode: {}: {}".format(hex(addr), base[addr])
    if postbreak_addr is None:
        print "ERROR: did not decompile enough instructions!"
        return None

    print "\nThe second (service) breakpoint can be set at:\n{}: {}".format(hex(postbreak_addr), postbreak_instruction)

    after_postbreak = [ a for a in base.keys() if a > postbreak_addr ]
    for addr in after_postbreak:
        if addr >= (postbreak_addr + brk_opcode_len):
            break
        print "Also covered by the service breakpoint opcode: {}: {}".format(hex(addr), base[addr])


    print
    print
    print "Assuming the output above is OK (no risk of jumping in the middle of a breakpoint), you can call:"
    print "     set_breakpoint({}, {}, foo);".format(hex(wanted_addr), hex(postbreak_addr))
    return postbreak_addr



if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description='Print the correct set_breakpoint line (== finds postbreak_instruction).')
    parser.add_argument("original_program", help="Original ELF file")
    parser.add_argument("wanted_addr", help="Where you want to break, as 0xAAAA (e.g., 0x08048520)")
    parser.add_argument("brk_opcode_len", type=int, help="sizeof(BRK_OPCODE) for this architecture")
    args = parser.parse_args()
    assert args.wanted_addr[0:2] in ["0x","0X"]
    args.wanted_addr = int(args.wanted_addr,16)
    postbreak_instruction(**vars(args))
