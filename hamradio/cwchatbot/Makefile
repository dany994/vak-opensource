#CC		= i586-mingw32msvc-gcc

PROG            = dump-paddle
CFLAGS		= -O -Wall
UNAME           = $(shell uname)

OBJS            = paddle.o

# Linux
ifeq ($(UNAME),Linux)
    CFLAGS 	+= -I/usr/include/SDL -I/usr/local/include/SDL
    LDLIBS	+= -lusb -lSDL
    OBJS	+= audio-sdl.o
endif

# Mac OS X
ifeq ($(UNAME),Darwin)
    CFLAGS	+= -I/opt/local/include
    LDFLAGS	+= -framework CoreAudio
    LDLIBS	+= -L/opt/local/lib -lusb
    OBJS	+= audio-macosx.o
endif

# Windows
ifeq ($(UNAME),MinGW)
    LDLIBS	+= -lwinmm
    OBJS	+= audio-win32.o
endif

all:		$(PROG)

clean:
		rm -f $(PROG) *.o *~ a.out

dump-paddle:    dump-paddle.o $(OBJS)
		$(CC) $(LDFLAGS) dump-paddle.o $(OBJS) -o $@ $(LDLIBS)
