UNAME_S         := $(shell uname -s)

# -std=c++11 -Wall -march=native
ifeq ($(UNAME_S),Linux)
CXX             = g++
CFLAGS          = -O -I..
CXXFLAGS        = -O -I..
LDFLAGS         =
LDLIBS          = -lOpenCL
endif

ifeq ($(UNAME_S),Darwin)
CXX             = clang++
CFLAGS          = -O -I..
CXXFLAGS        = -O -I..
LDFLAGS         =
LDLIBS          = -framework OpenCL
endif

EXEC            = cldemo #device_query

all:            $(EXEC)

device_query:   device_query.o
		$(CXX) $(LDFLAGS) -o $@ device_query.o $(LDLIBS)

cldemo:         cldemo.o
		$(CC) $(LDFLAGS) -o $@ cldemo.o $(LDLIBS)

%.o:            %.cpp
		$(CXX) $(CXXFLAGS) -c $<

%.o:            %.c
		$(CC) $(CFLAGS) -O -c $<

clean:
		rm -f *.o $(EXEC)
