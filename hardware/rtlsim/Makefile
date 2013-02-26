CFLAGS          = -Wall -Werror -g -O
LDFLAGS         = -g
OBJS            = example.o rtlsim.o

all:            example sum

example:        example.o rtlsim.o
		$(CC) $(LDFLAGS) -o $@ $@.o rtlsim.o

sum:            sum.o rtlsim.o
		$(CC) $(LDFLAGS) -o $@ $@.o rtlsim.o

clean:
		rm -f *.o example sum
