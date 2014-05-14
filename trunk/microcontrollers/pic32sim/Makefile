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
EXECUTABLE	= pic32mx7 #pic32mz
OPT             = -O2 -g
#OPT		= -O0 -g

OBJ_MX7		= obj/loadhex.o \
                  obj/peripherals-mx7.o \
                  obj/main-mx7.o

OBJ_MZ		= obj/loadhex.o \
                  obj/peripherals-mz.o \
                  obj/main-mz.o

CFLAGS          = -m32 -g -Wall -Werror $(OPT) \
                  -I$(IMPERAS_HOME)/ImpPublic/include/common \
                  -I$(IMPERAS_HOME)/ImpPublic/include/host \
                  -I$(IMPERAS_HOME)/ImpProprietary/include/host

LDFLAGS         = -L$(IMPERAS_HOME)/bin/$(IMPERAS_ARCH) -lRuntimeLoader

all:		$(EXECUTABLE)

pic32mx7:       $(OBJ_MX7)
		$(CC) -m32 -o $@ $^ $(LDFLAGS)

pic32mz:        $(OBJ_MZ)
		$(CC) -m32 -o $@ $^ $(LDFLAGS)

obj/main-mx7.o: main.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS) -DPIC32MX7

obj/main-mz.o:  main.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS) -DPIC32MZ

obj/peripherals-mx7.o: peripherals.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS) -DPIC32MX7

obj/peripherals-mz.o:  peripherals.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS) -DPIC32MZ

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
