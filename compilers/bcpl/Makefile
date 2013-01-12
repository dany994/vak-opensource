AS      = as
AFLAGS  = -g
CC      = cc
CFLAGS  = -g -O0 -Wall -Werror
LD      = ld
LDFLAGS = -g

PREFIX  = /usr/local

all: hello st #cg xg

#
# X86 code generator, native (pass 2)
#
xg: su.o xg.o rt.o sys.o
	$(LD) $(LDFLAGS) -o xg su.o xg.o rt.o sys.o

xg.o: xg.s
	$(AS) $(AFLAGS) -o xg.o xg.s

xg.s: xg1 xg.int
	./xg1 < xg.int
	mv ASM xg.s

xg.int: st1 cg1 iclib.i blib.i xg.b
	./st1 < xg.b
	./cg1 < OCODE
	cat iclib.i blib.i INTCODE >xg.int

#
# Intcode generator, native (pass 2)
#
cg: su.o cg.o rt.o sys.o
	$(LD) $(LDFLAGS) -o cg su.o cg.o rt.o sys.o

cg.o: cg.s
	$(AS) $(AFLAGS) -o cg.o cg.s

cg.s: xg1 cg.int
	./xg1 < cg.int
	mv ASM cg.s

cg.int: st1 cg1 iclib.i blib.i cg.b
	./st1 < cg.b
	./cg1 < OCODE
	cat iclib.i blib.i INTCODE >cg.int

#
# Syntax translator, native (pass 2)
#
st: su.o st.o rt.o sys.o
	$(LD) $(LDFLAGS) -o st su.o st.o rt.o sys.o

st.o: st.s
	$(AS) $(AFLAGS) -o st.o st.s

st.s: xg1 st.int
	./xg1 < st.int
	mv ASM st.s

st.int: st1 cg1 iclib.i blib.i syn.b trn.b
	./st1 < syn.b
	./cg1 < OCODE
	cat iclib.i blib.i INTCODE >st.int
	./st1 < trn.b
	./cg1 < OCODE
	cat INTCODE >>st.int

#
# X86 code generator, native (pass 1)
#
xg1: su.o xg1.o rt.o sys.o
	$(LD) $(LDFLAGS) -o xg1 su.o xg1.o rt.o sys.o

xg1.o: xg1.s
	$(AS) $(AFLAGS) -o xg1.o xg1.s

xg1.s: xg0 xg1.int
	./xg0 < xg1.int
	mv ASM xg1.s

xg1.int: st0 cg0 iclib.i blib.i xg.b
	./st0 < xg.b
	./cg0 < OCODE
	cat iclib.i blib.i INTCODE >xg1.int

#
# Intcode generator, native (pass 1)
#
cg1: su.o cg1.o rt.o sys.o
	$(LD) $(LDFLAGS) -o cg1 su.o cg1.o rt.o sys.o

cg1.o: cg1.s
	$(AS) $(AFLAGS) -o cg1.o cg1.s

cg1.s: xg0 cg1.int
	./xg0 < cg1.int
	mv ASM cg1.s

cg1.int: st0 cg0 iclib.i blib.i cg.b
	./st0 < cg.b
	./cg0 < OCODE
	cat iclib.i blib.i INTCODE >cg1.int

#
# Syntax translator, native (pass 1)
#
st1: su.o st1.o rt.o sys.o
	$(LD) $(LDFLAGS) -o st1 su.o st1.o rt.o sys.o

st1.o: st1.s
	$(AS) $(AFLAGS) -o st1.o st1.s

st1.s: xg0 st1.int
	./xg0 < st1.int
	mv ASM st1.s

st1.int: st0 cg0 iclib.i blib.i syn.b trn.b
	./st0 < syn.b
	./cg0 < OCODE
	cat iclib.i blib.i INTCODE >st1.int
	./st0 < trn.b
	./cg0 < OCODE
	cat INTCODE >>st1.int

#
# X86 code generator, native (pass 0)
#
xg0: su.o xg0.o rt.o sys.o
	$(LD) $(LDFLAGS) -o xg0 su.o xg0.o rt.o sys.o

xg0.o: xg0.s
	$(AS) $(AFLAGS) -o xg0.o xg0.s

xg0.s: icint xg0.int
	./icint xg0.int < xg0.int
	mv ASM xg0.s

#
# Intcode generator, native (pass 0)
#
cg0: su.o cg0.o rt.o sys.o
	$(LD) $(LDFLAGS) -o cg0 su.o cg0.o rt.o sys.o

cg0.o: cg0.s
	$(AS) $(AFLAGS) -o cg0.o cg0.s

cg0.s: icint xg0.int cg0.int
	./icint xg0.int < cg0.int
	mv ASM cg0.s

#
# Syntax translator, native (pass 0)
#
st0: su.o st0.o rt.o sys.o
	$(LD) $(LDFLAGS) -o st0 su.o st0.o rt.o sys.o

st0.o: st0.s
	$(AS) $(AFLAGS) -o st0.o st0.s

st0.s: icint xg0.int st0.int
	./icint xg0.int < st0.int
	mv ASM st0.s

#
# System library, runtime library and startup
#
sys.o: sys.s
	$(AS) $(AFLAGS) -o sys.o sys.s

rt.o: rt.s
	$(AS) $(AFLAGS) -o rt.o rt.s

su.o: su.s
	$(AS) $(AFLAGS) -o su.o su.s

#
# X86 code generator, intcode
#
xg0.int: iclib.i blib.i xg.i
	cat iclib.i blib.i xg.i >xg0.int

xg.i: icint st0.int cg0.int LIBHDR xg.b
	./icint st0.int < xg.b
	./icint cg0.int < OCODE
	mv INTCODE xg.i

#
# Syntax translator and code generator, intcode
#
cg0.int: iclib.i blib.i cg.i
	cat iclib.i blib.i cg.i >cg0.int

st0.int: iclib.i blib.i syn.i trn.i
	cat iclib.i blib.i syn.i trn.i >st0.int

#
# Intcode interpreter
#
icint: icint.o blib.o
	$(CC) $(LDFLAGS) -o icint icint.o blib.o

blib.o: blib.c
	$(CC) $(CFLAGS) -c blib.c

icint.o: icint.c
	$(CC) $(CFLAGS) -c icint.c

#
# Hello, intcode
#
hello: icint st0.int cg0.int LIBHDR hello.b iclib.i blib.i xg0.int clib.o
	./icint st0.int < hello.b
	./icint cg0.int < OCODE
	cat iclib.i blib.i INTCODE > hello.int
	./icint hello.int
	./icint xg0.int < hello.int
	mv ASM hello.s
	$(AS) $(AFLAGS) -o hello.o hello.s
	$(CC) $(LDFLAGS) -o hello clib.o hello.o

#
# Installation
#
cp: st cg xg LIBHDR iclib.i blib.i rules su.o rt.o sys.o bcpl icint
	-mkdir $(PREFIX)/bin
	-mkdir $(PREFIX)/lib/bcpl
	cp st        $(PREFIX)/lib/bcpl/
	cp cg        $(PREFIX)/lib/bcpl/
	cp xg        $(PREFIX)/lib/bcpl/
	cp LIBHDR    $(PREFIX)/lib/bcpl/
	cp iclib.i   $(PREFIX)/lib/bcpl/
	cp blib.i    $(PREFIX)/lib/bcpl/
	cp rules     $(PREFIX)/lib/bcpl/
	cp su.o      $(PREFIX)/lib/bcpl/
	cp rt.o      $(PREFIX)/lib/bcpl/
	cp sys.o     $(PREFIX)/lib/bcpl/
	cp bcpl      $(PREFIX)/bin/
	cp icint     $(PREFIX)/bin/
	make clean

clean:
	rm -f OCODE INTCODE ASM *.o *.int xg.i
	rm -f st*.s cg*.s xg*.s hello.s
	rm -f icint st0 cg0 xg0 st1 cg1 xg1 st cg xg hello
