CFLAGS  = -O3 -DNRUNS=100000000 -Wall -Werror

OBJS    = dhry.o

all:    dhrystone

dhrystone: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o dhrystone
