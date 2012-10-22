#CC		= i586-mingw32msvc-gcc

PROG            = dump-paddle
CFLAGS		= -O -Wall
UNAME           = $(shell uname)

# Linux
ifeq ($(UNAME),Linux)
    LDLIBS	+= -lusb
endif

# Mac OS X
ifeq ($(UNAME),Darwin)
    CFLAGS	+= -I/opt/local/include
    LDLIBS	+= -L/opt/local/lib -lusb
endif

all:		$(PROG)

clean:
		rm -f $(PROG) *.o *~ a.out
