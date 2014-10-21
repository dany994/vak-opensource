#SYSTEMC         = /usr/local/mips/systemc-2.2
SYSTEMC_AMS     = /usr/local/systemc-ams-1.0.1

CXX             = g++ -pthread
CXXFLAGS        = -DSC_INCLUDE_FX -I$(SYSTEMC_AMS)/include -I$(SYSTEMC)/include -Wall -O3
LIBS            = -static -L$(SYSTEMC_AMS)/lib-linux64 -lsystemc-ams \
                  -L$(SYSTEMC)/lib-linux64 -lsystemc -lm

PROGS           = pendulum
OBJS            = pendulum.o

all:		$(PROGS)
		size $(PROGS)

clean:
		rm -f $(PROGS) *.o *.vcd *.dat

pendulum:       $(OBJS)
		$(CXX) -o $@ $(OBJS) $(LIBS)

run:
		LD_LIBRARY_PATH=$(SYSTEMC_AMS)/lib-linux64 ./pendulum

view:
		gtkwave -A trace.vcd &

pendulum.o: pendulum.cpp point-mass.h spring.h
