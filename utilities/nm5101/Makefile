CC		= avr-gcc333 -g -mmcu=at90s2313 -Wall -Os
LD		= avr-gcc333 -g -mmcu=at90s2313 -nostdlib -T $(LDSCRIPT)
LDSCRIPT	= ldscript23xx.x
STARTUP		= startup2313.o
SIZE		= avr-size333
OBJCOPY		= avr-objcopy333
OBJDUMP		= avr-objdump333

all:		nm5101.sre cronyx.sre
		$(SIZE) *.out

nm5101.sre:	$(LDSCRIPT) $(STARTUP) nm5101.o pictures.o
		$(LD) $(STARTUP) nm5101.o pictures.o -o nm5101.out
		$(OBJCOPY) -O srec nm5101.out $@
		chmod -x $@
		$(OBJDUMP) -D -S nm5101.out > nm5101.dis

cronyx.sre:	$(LDSCRIPT) $(STARTUP) phrases.o cronyx.o
		$(LD) $(STARTUP) phrases.o cronyx.o -o cronyx.out
		$(OBJCOPY) -O srec cronyx.out $@
		chmod -x $@

gse.sre:	masterkit/gse.hex
		$(OBJCOPY) -I ihex -O srec $? $@
clean:
		rm -f *~ *.o *.out *.dis
