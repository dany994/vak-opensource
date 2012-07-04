P              = /usr/local/mipsel441/bin/mipsel-elf32-
#P               = /usr/local/pic32-tools/bin/pic32-
CC              = $(P)gcc -mips32r2 -g -nostdlib
OBJCOPY         = $(P)objcopy
OBJDUMP         = $(P)objdump
CFLAGS          = -O -Wall -Werror
LDFLAGS         = -T pic32.ld -e _start
#LIBS            = -lmchp_peripheral_32MX795F512L

all:            flash.srec

flash.srec:     flash.c
		$(CC) $(CFLAGS) -c $<
		$(CC) $(LDFLAGS) flash.o $(LIBS) -o flash.elf
		$(OBJCOPY) -O srec flash.elf flash.srec
		$(OBJDUMP) -mmips:isa32r2 -d -S flash.elf > flash.dis

load:           flash.srec
		pic32prog -p flash.srec

clean:
		rm -f *.o *.lst *~ *.elf *.srec *.dis
