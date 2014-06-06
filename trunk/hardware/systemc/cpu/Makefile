SYSTEMC_HOME    := /usr/local/systemc-2.3.0

OBJS            := Processor.o Memory.o example.o
EXEC            := example.exe
CXXFLAGS        := -g -O -Wall -Werror -I$(SYSTEMC_HOME)/include -DPRINT_WHILE_RUN
LDFLAGS         := -g
LIBS            := -Wl,-Bstatic -L$(SYSTEMC_HOME)/lib-linux -lsystemc -Wl,-Bdynamic -lpthread

all:            $(EXEC)

$(EXEC):        $(OBJS)
		g++ $(LDFLAGS) $^ $(LIBS) -o $@

clean:
		rm -f *.exe *.o

view:
		gtkwave wave.vcd &
