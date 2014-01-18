CFLAGS		+= -Werror -Wall -g
LDFLAGS         += -g

PROG            = asmk

all:            $(PROG)

$(PROG):        $(PROG).o
		${CC} ${LDFLAGS} -o $@ $(PROG).o ${LIBS}

clean:
		rm -f *.o $(PROG) *~
