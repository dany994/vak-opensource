CFLAGS          = -Wall -Werror 
OBJS            = example.o rtlsim.o

# For Mac OS X
#CFLAGS          += -D_XOPEN_SOURCE -Wno-deprecated-declarations

all:            example

example:        $(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
		rm -f *.o example
