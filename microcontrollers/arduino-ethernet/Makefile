TOPSRC          = /scratch/retrobsd/trunk
include $(TOPSRC)/target.mk

CFLAGS          += -O -Wall -Werror

OBJCLIENT       = w5100.o socket.o ethernet.o client.o example-telnet-client.o
OBJSERVER       = $(OBJCLIENT) server.o
LIBS            +=

all:            example-telnet-client example-chat-server

example-telnet-client: ${OBJCLIENT} example-telnet-client.o
		${CC} ${LDFLAGS} -o $@.elf ${OBJCLIENT} ${LIBS}
		${OBJDUMP} -S $@.elf > $@.dis
		${SIZE} $@.elf
		${ELF2AOUT} $@.elf $@

example-chat-server: ${OBJSERVER} example-chat-server.o
		${CC} ${LDFLAGS} -o $@.elf ${OBJSERVER} ${LIBS}
		${OBJDUMP} -S $@.elf > $@.dis
		${SIZE} $@.elf
		${ELF2AOUT} $@.elf $@

clean:
		rm -f *.o *.elf *.dis example-telnet-client example-chat-server

install:        all
		install example-telnet-client $(DESTDIR)/bin/
