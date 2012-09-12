
FC              = gfortran -fno-automatic -fsecond-underscore
VERSION         = $(shell $(FC) --version | head -1)

#FC              = fort77
#VERSION         = $(shell f2c --version)

CFLAGS          = -O -Wall

PROG            = nec2dxs
OBJS            = $(PROG).o get_time.o

all:            $(PROG)

$(PROG):        $(OBJS)
		$(FC) $(OBJS) -o $@

$(PROG).o:      $(PROG).f
		echo "      DATA G77PORT/'$(VERSION)'/" > g77port.inc
		$(FC) $(CFLAGS) -c $< -o $@

clean:
		rm -f $(PROG) *.o

gwave:          $(PROG) gwave.nec
		./$(PROG)
