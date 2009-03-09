CFLAGS  = -Wall -g -O
INSTDIR	= /usr/local

all:    as16 dis16 #sim16 hello.hex

as16:   as.o
	$(CC) $(LDFLAGS) as.o -o as16

dis16:  dis.o
	$(CC) $(LDFLAGS) dis.o -o dis16

sim16:  sim.o
	$(CC) $(LDFLAGS) sim.o -o sim16

clean:
	rm -f *.[ob] a.out *.core as16 dis16 sim16 #*.hex *.lst

install: as16 #dis16 sim16
	install -c as16 $(INSTDIR)/bin
	install -c dis16 $(INSTDIR)/bin
#	install -c sim16 $(INSTDIR)/bin

.SUFFIXES: .hex

.s.hex:
	as16 $<
