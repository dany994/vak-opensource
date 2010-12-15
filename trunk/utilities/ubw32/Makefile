CC    = gcc
EXECS = ubw32
OBJS  = main.o hex.o

ifeq ($(shell uname -s),Darwin)
# Rules for Mac OS X
  OBJS    += usb-osx.o
  CFLAGS   = -fast
  LDFLAGS  = -Wl,-framework,IOKit,-framework,CoreFoundation
else
# Rules for Linux, etc.
  OBJS    += usb-linux.o
  CFLAGS   = -O3
  LDFLAGS  = -lhid -lusb
endif

all: $(EXECS)

*.o: ubw32.h

.c.o:
	$(CC) $(CFLAGS) -c $*.c
 
ubw32: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o ubw32
	strip ubw32

# Must install as root; e.g. 'sudo make install'
install: ubw32
	cp ubw32 /usr/local/bin

clean:
	rm -f $(EXECS) *.o core

