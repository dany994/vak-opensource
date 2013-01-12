AS = as
AFLAGS =
CC = cc
CFLAGS = -O
LD = ld
LDFLAGS =

PREFIX=/usr

all: st cg xg

xg: su.o xg.o rt.o sys.o
	$(LD) $(LDFLAGS) -o xg su.o xg.o rt.o sys.o

xg.o: xg.s
	$(AS) $(AFLAGS) -o xg.o xg.s

xg.s: xg1 xg.int xg.int
	./xg1 <xg.int
	mv ASM xg.s

xg.int: st1 cg1 iclib.i blib.i xg.b
	./st1 < xg.b
	./cg1 < OCODE
	cat iclib.i blib.i INTCODE >xg.int

cg: su.o cg.o rt.o sys.o
	$(LD) $(LDFLAGS) -o cg su.o cg.o rt.o sys.o

cg.o: cg.s
	$(AS) $(AFLAGS) -o cg.o cg.s

cg.s: xg1 xg.int cg.int
	./xg1 <cg.int
	mv ASM cg.s

cg.int: st1 cg1 iclib.i blib.i cg.b
	./st1 < cg.b
	./cg1 < OCODE
	cat iclib.i blib.i INTCODE >cg.int

st: su.o st.o rt.o sys.o
	$(LD) $(LDFLAGS) -o st su.o st.o rt.o sys.o

st.o: st.s
	$(AS) $(AFLAGS) -o st.o st.s

st.s: xg1 xg.int st.int
	./xg1 <st.int
	mv ASM st.s

st.int: st1 cg1 iclib.i blib.i syn.b trn.b
	./st1 < syn.b
	./cg1 < OCODE
	cat iclib.i blib.i INTCODE >st.int
	./st1 < trn.b
	./cg1 < OCODE
	cat INTCODE >>st.int

xg1: su.o xg1.o rt.o sys.o
	$(LD) $(LDFLAGS) -o xg1 su.o xg1.o rt.o sys.o

xg1.o: xg1.s
	$(AS) $(AFLAGS) -o xg1.o xg1.s

xg1.s: xg0 xg1.int xg1.int
	./xg0 <xg1.int
	mv ASM xg1.s

xg1.int: st0 cg0 iclib.i blib.i xg.b
	./st0 < xg.b
	./cg0 < OCODE
	cat iclib.i blib.i INTCODE >xg1.int

cg1: su.o cg1.o rt.o sys.o
	$(LD) $(LDFLAGS) -o cg1 su.o cg1.o rt.o sys.o

cg1.o: cg1.s
	$(AS) $(AFLAGS) -o cg1.o cg1.s

cg1.s: xg0 xg1.int cg1.int
	./xg0 <cg1.int
	mv ASM cg1.s

cg1.int: st0 cg0 iclib.i blib.i cg.b
	./st0 < cg.b
	./cg0 < OCODE
	cat iclib.i blib.i INTCODE >cg1.int

st1: su.o st1.o rt.o sys.o
	$(LD) $(LDFLAGS) -o st1 su.o st1.o rt.o sys.o

st1.o: st1.s
	$(AS) $(AFLAGS) -o st1.o st1.s

st1.s: xg0 xg1.int st1.int
	./xg0 <st1.int
	mv ASM st1.s

st1.int: st0 cg0 iclib.i blib.i syn.b trn.b
	./st0 < syn.b
	./cg0 < OCODE
	cat iclib.i blib.i INTCODE >st1.int
	./st0 < trn.b
	./cg0 < OCODE
	cat INTCODE >>st1.int

xg0: su.o xg0.o rt.o sys.o
	$(LD) $(LDFLAGS) -o xg0 su.o xg0.o rt.o sys.o

xg0.o: xg0.s
	$(AS) $(AFLAGS) -o xg0.o xg0.s

xg0.s: icint xg0.int xg0.int
	./icint xg0.int <xg0.int
	mv ASM xg0.s

cg0: su.o cg0.o rt.o sys.o
	$(LD) $(LDFLAGS) -o cg0 su.o cg0.o rt.o sys.o

cg0.o: cg0.s
	$(AS) $(AFLAGS) -o cg0.o cg0.s

cg0.s: icint xg0.int cg0.int
	./icint xg0.int <cg0.int
	mv ASM cg0.s

st0: su.o st0.o rt.o sys.o
	$(LD) $(LDFLAGS) -o st0 su.o st0.o rt.o sys.o

st0.o: st0.s
	$(AS) $(AFLAGS) -o st0.o st0.s

st0.s: icint xg0.int st0.int
	./icint xg0.int <st0.int
	mv ASM st0.s

sys.o: sys.s
	$(AS) $(AFLAGS) -o sys.o sys.s

rt.o: rt.s
	$(AS) $(AFLAGS) -o rt.o rt.s

su.o: su.s
	$(AS) $(AFLAGS) -o su.o su.s

xg0.int: iclib.i blib.i xg.i
	cat iclib.i blib.i xg.i >xg0.int

xg.i: icint st0.int cg0.int LIBHDR xg.b
	./icint st0.int <xg.b
	./icint cg0.int <OCODE
	mv INTCODE xg.i

cg0.int: iclib.i blib.i cg.i
	cat iclib.i blib.i cg.i >cg0.int

st0.int: iclib.i blib.i syn.i trn.i
	cat iclib.i blib.i syn.i trn.i >st0.int

icint: icint.o blib.o
	$(CC) $(LDFLAGS) -o icint icint.o blib.o

blib.o: blib.c
	$(CC) $(CFLAGS) -c blib.c

icint.o: icint.c
	$(CC) $(CFLAGS) -c icint.c

cp: st cg xg LIBHDR iclib.i blib.i rules su.o rt.o sys.o bcpl icint
	-mkdir $(PREFIX)/bin
	-mkdir $(PREFIX)/lib/bcpl
	cp st        $(PREFIX)/lib/bcpl
	cp cg        $(PREFIX)/lib/bcpl
	cp xg        $(PREFIX)/lib/bcpl
	cp LIBHDR    $(PREFIX)/lib/bcpl
	cp iclib.i   $(PREFIX)/lib/bcpl
	cp blib.i    $(PREFIX)/lib/bcpl
	cp rules     $(PREFIX)/lib/bcpl
	cp su.o      $(PREFIX)/lib/bcpl
	cp rt.o      $(PREFIX)/lib/bcpl
	cp sys.o     $(PREFIX)/lib/bcpl
	cp bcpl      $(PREFIX)/bin
	cp icint     $(PREFIX)/bin
	make clean

clean:
	rm -f OCODE INTCODE ASM
	rm -f icint.o blib.o icint
	rm -f st0.int cg0.int xg.i xg0.int
	rm -f su.o rt.o sys.o
	rm -f st0.s st0.o st0
	rm -f cg0.s cg0.o cg0
	rm -f xg0.s xg0.o xg0
	rm -f st1.int cg1.int xg1.int
	rm -f st1.s st1.o st1
	rm -f cg1.s cg1.o cg1
	rm -f xg1.s xg1.o xg1
	rm -f st.int cg.int xg.int
	rm -f st.s st.o st
	rm -f cg.s cg.o cg
	rm -f xg.s xg.o xg
