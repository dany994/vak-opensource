TOPSRC          = /scratch/retrobsd/trunk
include $(TOPSRC)/target.mk

CFLAGS          += -O -Wall -Werror

OBJS            = w5100.o socket.o ethernet.o client.o example-telnet-client.o
LIBS            +=

all:            example-telnet-client

example-telnet-client: ${OBJS}
		${CC} ${LDFLAGS} -o $@.elf ${OBJS} ${LIBS}
		${OBJDUMP} -S $@.elf > $@.dis
		${SIZE} $@.elf
		${ELF2AOUT} $@.elf $@

clean:
		-rm -f example-telnet-client ${OBJS} *.elf *.dis

install:        all
		install example-telnet-client $(DESTDIR)/bin/
