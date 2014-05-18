#
# Check for a required build environment.
#
ifndef IMPERAS_HOME
IMPERAS_ERROR   := $(error "Please source 'imperas.environ' to setup Imperas environment.")
endif

#
# PIC32 MX7
#
EXECUTABLE	= pic32mx7 
OPT             = -DPIC32MX7
OBJ		= obj/mx.o 

#
# PIC32 MZ
#
#EXECUTABLE	= pic32mz
#OPT		= -DPIC32MZ
#OBJ		= obj/mz.o 

OPT             += -O2 -g -DEXPLORER16
#OPT		+= -O0 -g

OBJ		+= obj/loadhex.o \
                   obj/main.o \
                   obj/sdcard.o \
                   obj/spi.o \
                   obj/uart.o \
                   obj/vtty.o

CFLAGS          = -m32 -g -Wall -Werror $(OPT) \
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
		rm -rf *.exe *.log *.so *.o obj $(EXECUTABLE) *~
###
obj/loadhex.o: loadhex.c globals.h
obj/main.o: main.c globals.h
obj/mx.o: mx.c globals.h pic32mx.h
obj/sdcard.o: sdcard.c globals.h
obj/vtty.o: vtty.c globals.h
