PROG            = dyio
CFLAGS		= -O -Wall -Werror
LDFLAGS		=
#CC		= i586-mingw32msvc-gcc
OBJS            = $(PROG).o serial.o

all:		$(PROG)

$(PROG):        $(OBJS)
		$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
		rm -f $(PROG) *.o *~ a.out
