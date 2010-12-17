# Example makefile for use with the command-line C32 compiler and UBW32.
# Assumes C32 compiler is built and installed as per directions at:
#     http://www.paintyourdragon.com/uc/osxpic32/index.html
# Further assumes that 'ubw32' program is built and installed in
# /usr/local/bin or other directory in your search path.

# IMPORTANT: file 'procdefs.ld' absolutely must Must MUST be present in the
# working directory when compiling code for the UBW32!  Failure to do so will
# almost certainly result in your UBW32 getting 'bricked' and requiring
# re-flashing the bootloader (which, if you don't have a PICkit 2 or similar
# PIC programmer, you're screwed).  YOU HAVE BEEN WARNED.

# Type 'make' to build example program.  Put UBW32 device in Bootloader mode
# (PRG + RESET buttons), then type 'make write' to download program to the
# device.  Note: the latter will OVERWRITE any program you currently have
# installed there, including the Bit Whacker firmware, so be sure to keep
# around a copy of the original, which can be downloaded from the product
# web page here: http://www.schmalzhaus.com/UBW32/

HEX  = test.hex
UBW  = ubw32 -r
PROC = 32MX460F512L
CC   = /usr/local/pic32mx/bin/pic32mx-gcc -s -mprocessor=$(PROC)
#CC   = /usr/local/pic32mx/bin/pic32mx-gcc -O3 -mips16 -s -mprocessor=$(PROC)

all: $(HEX)

$(HEX): test.elf
	/usr/local/pic32mx/bin/pic32mx-bin2hex -a test.elf

test.elf: test.c
	$(CC) test.c -o test.elf

write:
	$(UBW) -w $(HEX)

erase:
	$(UBW) -e

clean:
	rm -f *.o *.elf *.hex
