CFLAGS		= -Wall -g $(shell sdl-config --cflags)
LDFLAGS		= -g
LIBS		= $(shell sdl-config --libs)

all:		example

clean:
		rm -f *~ *.o example

example:	example.o
		$(CC) $(LDFLAGS) example.o $(LIBS) -o $@
