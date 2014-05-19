#
# Check for a required build environment.
#
ifndef IMPERAS_HOME
IMPERAS_ERROR   := $(error "Please source 'imperas.environ' to setup Imperas environment.")
endif

#
# Processor: PIC32 MX7
#
#EXECUTABLE	= pic32mx7
#PROCESSOR	= PIC32MX7
#OBJ		= obj/mx7.o

#
# Processor: PIC32 MZ
#
EXECUTABLE	= pic32mz
PROCESSOR	= PIC32MZ
OBJ		= obj/mz.o

#
# Board type
#
BOARD		= EXPLORER16
#BOARD		= MAXIMITE
#BOARD		= MAX32

OPTIMIZE        = -O2
OBJ		+= obj/loadhex.o \
                   obj/main.o \
                   obj/sdcard.o \
                   obj/spi.o \
                   obj/uart.o \
                   obj/vtty.o

CFLAGS          = -m32 -g -Wall -Werror $(OPTIMIZE) -D$(PROCESSOR) -D$(BOARD) \
                  -I$(IMPERAS_HOME)/ImpPublic/include/common \
                  -I$(IMPERAS_HOME)/ImpPublic/include/host \
                  -I$(IMPERAS_HOME)/ImpProprietary/include/host

LDFLAGS         = -L$(IMPERAS_HOME)/bin/$(IMPERAS_ARCH) -lRuntimeLoader \
                  -lpthread

all:		$(EXECUTABLE)

$(EXECUTABLE):  $(OBJ)
		$(CC) -m32 -o $@ $^ $(LDFLAGS)

obj/%.o:        %.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $< $(CFLAGS)

obj/%.o:        %.c??
		@mkdir -p $(@D)
		$(CC) -c -o $@ $< $(CFLAGS)

clean:
		rm -rf *.exe *.log *.so *.o obj pic32mx7 pic32mz *~
###
obj/loadhex.o: loadhex.c globals.h
obj/main.o: main.c globals.h
obj/mx7.o: mx7.c globals.h pic32mx.h
obj/mz.o: mz.c globals.h pic32mz.h
obj/sdcard.o: sdcard.c globals.h
obj/spi.o: spi.c globals.h pic32mx.h pic32mz.h
obj/uart.o: uart.c globals.h pic32mx.h pic32mz.h
obj/vtty.o: vtty.c globals.h
