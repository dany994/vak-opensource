CC		= gcc

SVNVERS         = $(shell svnversion)
CFLAGS		= -Wall -g -O -I/opt/local/include -Ihidapi -DSVNVERSION='"$(SVNVERS)"'
LDFLAGS		=

# Linux
ifneq (,$(wildcard /lib/i386-linux-gnu))
    LIBS        += -lusb-1.0
    HIDSRC      = hidapi/hid-libusb.c
endif

# Mac OS X
ifneq (,$(wildcard /System/Library/Frameworks/CoreFoundation.framework))
    LIBS        += -framework IOKit -framework CoreFoundation
    HIDSRC      = hidapi/hid-mac.c
endif

OBJS		= gdbproxy.o rpmisc.o target-pic32mx.o hid.o \
                  proxy-mips.o adapter-pickit2.o

# Olimex ARM-USB-Tiny JTAG adapter.
CFLAGS          += -DUSE_MPSSE
OBJS            += adapter-mpsse.o
LIBS            += -L/opt/local/lib -lusb

all:		pic32proxy

pic32proxy:	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

hid.o:          $(HIDSRC)
		$(CC) $(CFLAGS) -c -o $@ $<

clean:
		rm -f *~ *.o core pic32proxy

install:	pic32proxy
		install -c -s pic32proxy /usr/local/bin/pic32proxy
		if [ `uname` = Linux ]; then \
		    chown root /usr/local/bin/pic32proxy; \
		    chmod 4755 /usr/local/bin/pic32proxy; \
		fi
		install -d /usr/local/share/locale/ru/LC_MESSAGES
###
adapter-mpsse.o: adapter-mpsse.c adapter.h mips.h pic32.h
adapter-pickit2.o: adapter-pickit2.c adapter.h hidapi/hidapi.h pickit2.h mips.h pic32.h
gdbproxy.o: gdbproxy.c gdbproxy.h
proxy-mips.o: proxy-mips.c gdbproxy.h target.h
rpmisc.o: rpmisc.c gdbproxy.h
target-pic32mx.o: target-pic32mx.c target.h adapter.h mips.h pic32.h
