DEST		= /usr/local/bin
MINGW		= /usr/bin/i586-mingw32msvc-
CC		= gcc
CFLAGS		= -Wall -Werror -O
LDFLAGS		= -s

# Версия для прошивки через AVR STK500.
OBJS		= avr-stk.o main.o identify.o

all:		stkprog

# Unix
$(PROG):	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS)

stkprog:	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
		rm -f *.o stkprog
###
avr-stk.o: avr-stk.c avr.h stk500.h
identify.o: identify.c avr.h prog.h avr-int.h
main.o: main.c avr.h unix.c mingw32.c
