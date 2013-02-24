CFLAGS          = -Wall -Werror -D_XOPEN_SOURCE -Wno-deprecated-declarations
OBJS            = example.o rtlsim.o

all:            example

example:        $(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
		rm -f *.o example
