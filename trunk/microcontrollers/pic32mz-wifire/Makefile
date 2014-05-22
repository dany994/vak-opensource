P               = /usr/local/pic32-tools/bin/pic32-
CC              = $(P)gcc -mips32r2 -g -nostdlib
OBJCOPY         = $(P)objcopy
OBJDUMP         = $(P)objdump
#GDB             = /mips/arch/overflow/codesourcery/mips-sde-elf/lite/release/2012.03-64/Linux/bin/mips-sde-elf-gdb
#GDB             = mipsel-elf32-gdb
#GDB             = /usr/local/mips/insight681/bin/mipsel-elf32-insight
GDB             = /usr/local/mips/insight70/bin/mips-elf-insight
CFLAGS          = -O -Wall -Werror -DPIC32MX7
LDFLAGS         = -T using-bootloader.ld -e _start
PROGTOOL        = /opt/mpide-0023-linux-20120903/hardware/tools/avrdude \
                  -C /opt/mpide-0023-linux-20120903/hardware/tools/avrdude.conf \
                  -P /dev/ttyUSB0 -c stk500v2 -p pic32 -b 115200

#PROG            = led
PROG            = uart

all:            $(PROG).hex

$(PROG).hex:    $(PROG).c
		$(CC) $(CFLAGS) -c $<
		$(CC) $(LDFLAGS) $(PROG).o -o $(PROG).elf
		$(OBJCOPY) -O ihex $(PROG).elf $(PROG).hex
		$(OBJDUMP) -mmips:isa32r2 -d -S $(PROG).elf > $(PROG).dis

load:           $(PROG).hex
		$(PROGTOOL) -U flash:w:$(PROG).hex:i

clean:
		rm -f *.o *.lst *~ *.elf *.hex *.dis

debug:          $(PROG).srec
		$(GDB) $(PROG).elf
