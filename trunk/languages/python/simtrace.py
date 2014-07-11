#!/usr/bin/python
# -*- encoding: utf-8 -*-
#
# Print the sequence of function calls from the Imperas trace file.
#
import sys, string, subprocess

if len(sys.argv) != 3:
    print "Usage: simtrace file.trace vmunix.elf"
    sys.exit (1)

# Extract the list of symbols from the binary executable.
nm_command = subprocess.Popen ("nm "+sys.argv[2], shell = True, stdout = subprocess.PIPE)
table = {}
max_addr = 0
for line in nm_command.stdout.readlines():
    word = line.split()
    addr = int(word[0], 16)
    func = word[2]
    table[addr] = func
    if addr > max_addr:
        max_addr = addr
    #print "%08x = %s" % (addr, func)
table_keys = sorted(table.keys())
#print table_keys

# Find a name of the function for the given address.
# Return the name and the offset.
def find_function (addr):
    if addr > max_addr:
        return ("", 0)
    if addr in table_keys:
        return (table[addr], 0)
    last = 0
    for a in table_keys:
        if a > addr:
            break
        last = a
    return (table[last], addr - last)

# Print a function name for the given address.
last_func = ""
def process_instruction(addr):
    #print "--- process_instruction(%#x)" % addr
    global last_func

    # Skip bootloader region.
    if addr > max_addr:
        return

    (func, offset) = find_function (addr)
    if func != last_func:
        if offset == 0:
            print "%08x : %s" % (addr, func)
        elif offset > 0x10000:
            print "%08x : ???" % (addr)
        else:
            print "%08x : %s + %u" % (addr, func, offset)
        last_func = func

# Check whether the string is a hex number
hex_digits = set(string.hexdigits)
def is_hex(s):
     return all(c in hex_digits for c in s)

# Read the trace file.
trace_file = open (sys.argv[1])
pc = 0
for line in trace_file.readlines():
    word = line.split()
    if len(word) < 7:
        continue

    va = word[2]
    pa = word[3]
    cca = word[4]
    if not (word[1] == ":" and
            len(va) == 8 and len(pa) == 8 and
            is_hex(va) and is_hex(pa)):
        continue
    pc = int(va, 16)

    if cca != "2:" and cca != "3:":
        print "Warning: unexpected CCA value!"

    #print pc, ":", string.join(word[6:])
    process_instruction(pc)

# Print the last executed address.
if pc != 0:
    last_func = ""
    print "=== Stopped at %#x: ===" % pc
    process_instruction(pc)
