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
OPT             = -O2 -g -DEXPLORER16
#OPT		= -O0 -g
OPT_MX7         = -DPIC32MX7
OPT_MZ          = -DPIC32MZ

OBJ_MX7		= obj/loadhex.o \
                  obj/mx.o \
                  obj/main-mx7.o \
                  obj/sdcard.o \
                  obj/vtty.o

OBJ_MZ		= obj/loadhex.o \
                  obj/mz.o \
                  obj/main-mz.o \
                  obj/vtty.o

CFLAGS          = -m32 -g -Wall -Werror $(OPT) \
                  -I$(IMPERAS_HOME)/ImpPublic/include/common \
                  -I$(IMPERAS_HOME)/ImpPublic/include/host \
                  -I$(IMPERAS_HOME)/ImpProprietary/include/host

LDFLAGS         = -L$(IMPERAS_HOME)/bin/$(IMPERAS_ARCH) -lRuntimeLoader \
                  -lpthread

all:		$(EXECUTABLE)

pic32mx7:       $(OBJ_MX7)
		$(CC) -m32 -o $@ $^ $(LDFLAGS)

pic32mz:        $(OBJ_MZ)
		$(CC) -m32 -o $@ $^ $(LDFLAGS)

obj/main-mx7.o: main.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS) $(OPT_MX7)

obj/main-mz.o:  main.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS) $(OPT_MZ)

obj/%.o:        %.c
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS)

obj/%.o:        %.c??
		@mkdir -p $(@D)
		$(CC) -c -o $@ $^ $(CFLAGS)

clean:
		rm -rf *.exe *.log *.so *.o obj $(EXECUTABLE) *~
###
loadhex.o: loadhex.c globals.h
main.o: mx.c globals.h vtty.h
mx.o: mx.c globals.h vtty.h pic32mx.h
sdcard.o: sdcard.c globals.h
vtty.o: vtty.c globals.h vtty.h
