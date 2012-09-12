
#FC              = gfortran
FC             = fort77
FFLAGS          = -O -fno-automatic -Wall
CFLAGS          = -O -Wall

PROG            = nec2dxs
OBJS            = $(PROG).o second.o

all:            $(PROG)

$(PROG):        $(OBJS)
		$(FC) $(FFLAGS) $(OBJS) -o $@

clean:
		rm -f $(PROG) *.o
