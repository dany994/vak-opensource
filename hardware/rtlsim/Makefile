CFLAGS          = -m32 -Wall -Werror -g -O -U_FORTIFY_SOURCE
LDFLAGS         = -m32 -g
OBJS            = example.o rtlsim.o

all:            example sum random.c random.v random

example:        example.o rtlsim.o
		$(CC) $(LDFLAGS) -o $@ $@.o rtlsim.o

sum:            sum.o rtlsim.o
		$(CC) $(LDFLAGS) -o $@ $@.o rtlsim.o

random:         random.o rtlsim.o
		$(CC) $(LDFLAGS) -o $@ $@.o rtlsim.o

clean:
		rm -f *.o example sum random

random.c:       mkrandom-c.pl
		./mkrandom-c.pl > $@

random.v:       mkrandom-verilog.pl
		./mkrandom-verilog.pl > $@
