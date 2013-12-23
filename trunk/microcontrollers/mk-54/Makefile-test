PROG            = mk-54
CFLAGS		= -m32 -O -Wall
LDFLAGS		= -m32
OBJS            = ir2.o ik13.o mk-54.o test.o

all:		$(PROG)

$(PROG):        $(OBJS)
		$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
		rm -f $(PROG) *.o *~ a.out
