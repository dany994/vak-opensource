SYSTEMC_HOME    := /usr/local/systemc-2.3.0

OBJS            := cpu.o
EXEC            := cpu.exe
CXXFLAGS        := -g -O -Wall -Werror -I$(SYSTEMC_HOME)/include
LDFLAGS         := -g
LIBS            := -Wl,-Bstatic -L$(SYSTEMC_HOME)/lib-linux -lsystemc -Wl,-Bdynamic -lpthread

all:            $(EXEC)

$(EXEC):        $(OBJS)
		g++ $(LDFLAGS) $^ $(LIBS) -o $@

clean:
		rm -f $(EXEC) *.o
