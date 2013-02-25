CFLAGS          = -Wall -Werror -g -O
LDFLAGS         = -g
OBJS            = example.o rtlsim.o

all:            example

example:        $(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
		rm -f *.o example
