P               = /usr/local/pic32-tools/bin/pic32-
CC              = $(P)gcc -mips32r2 -g -nostdlib
OBJCOPY         = $(P)objcopy
OBJDUMP         = $(P)objdump
BIN2HEX         = $(P)bin2hex
#GDB             = /mips/arch/overflow/codesourcery/mips-sde-elf/lite/release/2012.03-64/Linux/bin/mips-sde-elf-gdb
#GDB             = mipsel-elf32-gdb
#GDB             = /usr/local/mips/insight681/bin/mipsel-elf32-insight
GDB             = /usr/local/mips/insight70/bin/mips-elf-insight
CFLAGS          = -O -Wall -Werror -DPIC32MZ
LDFLAGS         = -T using-bootloader.ld -e _start -mdebugger

# Linux
PROGTOOL        = /opt/mpide-0023-linux32-20140605-test/hardware/tools/avrdude \
                  -C /opt/mpide-0023-linux32-20140605-test/hardware/tools/avrdude.conf \
                  -P /dev/ttyUSB0 -c stk500v2 -p 32MZ2048ECG100 -b 115200

# Mac OS X
#PROGTOOL        = /Applications/Mpide.app/Contents/Resources/Java/hardware/tools/avr/bin/avrdude \
#                  -C /Applications/Mpide.app/Contents/Resources/Java/hardware/tools/avr/etc/avrdude.conf \
#                  -P /dev/tty.usbserial-* -c stk500v2 -p 32MZ2048ECG100 -b 115200 -D

PROG            = led
#PROG            = uart

all:            $(PROG).hex

$(PROG).hex:    $(PROG).c
		$(CC) $(CFLAGS) -c $<
		$(CC) $(LDFLAGS) $(PROG).o -o $(PROG).elf
#		$(OBJCOPY) -O ihex $(PROG).elf $(PROG).hex
		$(BIN2HEX) -a $(PROG).elf
		$(OBJDUMP) -mmips:isa32r2 -d -S $(PROG).elf > $(PROG).dis

load:           $(PROG).hex
		$(PROGTOOL) -U flash:w:$(PROG).hex:i

clean:
		rm -f *.o *.lst *~ *.elf *.hex *.dis

debug:          $(PROG).srec
		$(GDB) $(PROG).elf
