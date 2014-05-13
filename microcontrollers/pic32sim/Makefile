#
# Check for a required build environment.
#
ifndef IMPERAS_HOME
IMPERAS_ERROR   := $(error "Please source 'imperas.environ' to setup Imperas environment.")
endif

#
# This will take the platform.c file in this directory.
# Compile to executable using the HOST compiler.
#
SRC		= platform.c loadhex.c peripherals.c
EXECUTABLE	= pic32sim
OPT             = -O2 -g
#OPT		= -O0 -g

OBJ		= $(addprefix obj/, $(SRC:.c=.o))

CFLAGS          = -m32 -g -Wall -Werror $(OPT) \
                  -I$(IMPERAS_HOME)/ImpPublic/include/common \
                  -I$(IMPERAS_HOME)/ImpPublic/include/host \
                  -I$(IMPERAS_HOME)/ImpProprietary/include/host

LDFLAGS         = -L$(IMPERAS_HOME)/bin/$(IMPERAS_ARCH) -lRuntimeLoader

all:		$(EXECUTABLE)

$(EXECUTABLE):  $(OBJ)
		$(CC) -m32 -o $@ $^ $(LDFLAGS)

obj/%.o:        %.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS)

obj/%.o:        %.c??
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS)

clean:
		rm -rf *.exe *.log *.so *.o obj $(EXECUTABLE) *~
###
loadhex.o: loadhex.c
peripherals.o: peripherals.c globals.h pic32mx.h
platform.o: platform.c pic32mx.h globals.h
