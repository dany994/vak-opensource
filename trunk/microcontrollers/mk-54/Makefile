P               = /usr/local/pic32-tools/bin/pic32-
CC              = $(P)gcc -mips32r2 -g -nostdlib
OBJCOPY         = $(P)objcopy
OBJDUMP         = $(P)objdump
#GDB             = /mips/arch/overflow/codesourcery/mips-sde-elf/lite/release/2012.03-64/Linux/bin/mips-sde-elf-gdb
#GDB             = mipsel-elf32-gdb
#GDB             = /usr/local/mips/insight681/bin/mipsel-elf32-insight
GDB             = /usr/local/mips/insight70/bin/mips-elf-insight
CFLAGS          = -O3 -Wall -Werror
LDFLAGS         = -T using-bootloader.ld -e _start

OBJS            = ir2.o ik13.o mk-54.o

# Olimex T795 board with pic32mx7 processor.
#PROG            = olimex-mx7
#CFLAGS          += -DPIC32MX7
#OBJS            += olimex-mx7.o

# Homemade board with pic32mx2 processor.
PROG            = bare-mx2
CFLAGS          += -DPIC32MX2
OBJS            += bare-mx2.o

all:            $(PROG).srec

$(PROG).srec:   $(OBJS)
		$(CC) $(LDFLAGS) $(OBJS) -o $(PROG).elf
		$(OBJCOPY) -O srec $(PROG).elf $@
		$(OBJDUMP) -mmips:isa32r2 -d -S $(PROG).elf > $(PROG).dis

load:           $(PROG).srec
		pic32prog -p $(PROG).srec

clean:
		rm -f *.o *.lst *~ *.elf *.srec *.dis test $(PROG)

debug:          $(PROG).srec
		$(GDB) $(PROG).elf

###
ik13.o: ik13.c calc.h
ir2.o: ir2.c calc.h
mk-54.o: mk-54.c calc.h ik1302.c ik1303.c
bare-mx2.o: bare-mx2.c calc.h pic32mx.h
olimex-mx7.o: olimex-mx7.c calc.h pic32mx.h
