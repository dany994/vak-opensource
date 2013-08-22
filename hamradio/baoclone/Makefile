CC		= gcc -m32
#CC              = i586-mingw32msvc-gcc -m32

CFLAGS		= -g -O -Wall -Werror
LDFLAGS		=

OBJS		= main.o radio.o
LIBS            =

all:		baoclone

baoclone:	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
		rm -f *~ *.o core baoclone

install:	baoclone
		install -c -s baoclone /usr/local/bin/baoclone
###
