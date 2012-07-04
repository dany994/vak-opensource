CC		= gcc

SVNVERS         = $(shell svnversion)
CFLAGS		= -Wall -g -O -I/opt/local/include -Ihidapi -DSVNVERSION='"$(SVNVERS)"'
LDFLAGS		= -s

# Linux
LIBS            += -lusb-1.0
HIDSRC          = hidapi/hid-libusb.c

# Mac OS X
#LIBS            += -framework IOKit -framework CoreFoundation
#HIDSRC          = hidapi/hid-mac.c

REMOTE_OBJS	= gdbproxy.o rpmisc.o target.o adapter-pickit2.o adapter-mpsse.o hid.o proxy-ejtag.o

# Olimex ARM-USB-Tiny JTAG adapter.
CFLAGS         += -DUSE_MPSSE
PROG_OBJS      += adapter-mpsse.o
LIBS           += -L/opt/local/lib -lusb

all:		pic32proxy

pic32proxy:	$(REMOTE_OBJS)
		$(CC) $(LDFLAGS) -o $@ $(REMOTE_OBJS) $(LIBS)

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
adapter-bitbang.o: adapter-bitbang.c adapter.h
adapter-lpt.o: adapter-lpt.c adapter.h
adapter-mpsse.o: adapter-mpsse.c adapter.h
adapter-usb.o: adapter-usb.c adapter.h
conf.o: conf.c conf.h
gdbproxy.o: gdbproxy.c gdbproxy.h
proxy-elvees.o: proxy-elvees.c gdbproxy.h target.h
proxy-skeleton.o: proxy-skeleton.c gdbproxy.h
rpmisc.o: rpmisc.c gdbproxy.h
target.o: target.c target.h adapter.h mips.h
