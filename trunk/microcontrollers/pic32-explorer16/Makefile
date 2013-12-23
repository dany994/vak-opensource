P              = /usr/local/mips-gcc-4.7.2/bin/mips-elf-
#P              = /usr/local/mipsel441/bin/mipsel-elf32-
#P               = /usr/local/pic32-tools/bin/pic32-
CC              = $(P)gcc -mips32r2 -g -nostdlib
GDB             = mipsel-elf32-gdb
OBJCOPY         = $(P)objcopy
OBJDUMP         = $(P)objdump
GDB             = /mips/arch/overflow/codesourcery/mips-sde-elf/lite/release/2012.03-64/Linux/bin/mips-sde-elf-gdb
#GDB             = /usr/local/mips/insight681/bin/mipsel-elf32-insight
#GDB             = /usr/local/mipsel441/bin/mipsel-elf32-gdb
CFLAGS          = -O -Wall -Werror -DPIC32MX7
LDFLAGS         = -T pic32.ld -e _start
#LIBS            = -lmchp_peripheral_32MX795F512L

PROG            = lcd
#PROG            = flash

all:            $(PROG).srec

$(PROG).srec:   $(PROG).c
		$(CC) $(CFLAGS) -c $<
		$(CC) $(LDFLAGS) $(PROG).o $(LIBS) -o $(PROG).elf
		$(OBJCOPY) -O srec $(PROG).elf $(PROG).srec
		$(OBJDUMP) -mmips:isa32r2 -d -S $(PROG).elf > $(PROG).dis

load:           $(PROG).srec
		pic32prog -p $(PROG).srec

clean:
		rm -f *.o *.lst *~ *.elf *.srec *.dis

debug:          $(PROG).srec
		$(GDB) $(PROG).elf
