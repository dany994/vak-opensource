SYSTEMC         = /usr/local/mips/systemc-2.3.0

CXX             = g++ -m32 -pthread
CXXFLAGS        = -DSC_INCLUDE_FX -I$(SYSTEMC)/include -Wall -O3
LIBS            = -static -L$(SYSTEMC)/lib-linux -lsystemc -lm

PROGS           = simple_fifo
OBJS            = simple_fifo.o

all:		$(PROGS)
		size $(PROGS)

clean:
		rm -f $(PROGS) *.o

simple_fifo:    $(OBJS)
		$(CXX) -o $@ $(OBJS) $(LIBS)

run:
		LD_LIBRARY_PATH=$(SYSTEMC)/lib-linux ./simple_fifo
