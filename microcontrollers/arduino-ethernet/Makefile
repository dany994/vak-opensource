TOPSRC          = /scratch/retrobsd/trunk
include $(TOPSRC)/target.mk

OBJS            = example-telnet-client.o
LIBS            +=

all:            adc-demo

adc-demo:       ${OBJS}
		${CC} ${LDFLAGS} -o adc-demo.elf ${OBJS} ${LIBS}
		${OBJDUMP} -S adc-demo.elf > adc-demo.dis
		${SIZE} adc-demo.elf
		${ELF2AOUT} adc-demo.elf $@

clean:
		-rm -f adc-demo ${OBJS} adc-demo.elf adc-demo.dis

install:        all
		install adc-demo $(DESTDIR)/bin/
