P               = /usr/local/pic32-tools/bin/pic32-
CC              = $(P)gcc -mips32r2 -g -nostdlib
OBJCOPY         = $(P)objcopy
OBJDUMP         = $(P)objdump
#GDB             = /mips/arch/overflow/codesourcery/mips-sde-elf/lite/release/2012.03-64/Linux/bin/mips-sde-elf-gdb
#GDB             = mipsel-elf32-gdb
#GDB             = /usr/local/mips/insight681/bin/mipsel-elf32-insight
GDB             = /usr/local/mips/insight70/bin/mips-elf-insight
CFLAGS          = -O -Wall -Werror -DPIC32MX7
LDFLAGS         = -T mikroe-bootloader.ld -e _start

PROG            = led

all:            $(PROG).hex

$(PROG).hex:    $(PROG).c
		$(CC) $(CFLAGS) -c $<
		$(CC) $(LDFLAGS) $(PROG).o -o $(PROG).elf
		$(OBJCOPY) -O ihex $(PROG).elf $(PROG).hex
		$(OBJDUMP) -mmips:isa32r2 -d -S $(PROG).elf > $(PROG).dis

load:           $(PROG).hex
		pic32prog -p $(PROG).hex

clean:
		rm -f *.o *.lst *~ *.elf *.hex *.dis

debug:          $(PROG).hex
		$(GDB) $(PROG).elf
