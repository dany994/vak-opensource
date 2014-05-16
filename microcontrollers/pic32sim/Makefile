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
                  obj/mx.o \
                  obj/main-mx7.o \
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
		$(CC) -c -o $@ $^ $(CFLAGS) -DPIC32MX7

obj/main-mz.o:  main.c
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
