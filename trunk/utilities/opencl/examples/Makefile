UNAME_S         := $(shell uname -s)

# -std=c++11 -Wall -march=native
ifeq ($(UNAME_S),Linux)
CXX             = clang++
CFLAGS          = -O -I.. -DCL_USE_DEPRECATED_OPENCL_1_1_APIS
CXXFLAGS        = -O -I.. -DCL_USE_DEPRECATED_OPENCL_1_1_APIS
LDFLAGS         =
LDLIBS          = -lOpenCL
endif

ifeq ($(UNAME_S),Darwin)
CXX             = clang++
CXXFLAGS        = -O -I..
LDFLAGS         =
LDLIBS          = -framework OpenCL
endif

EXEC            = device_query cldemo

all:            $(EXEC)

device_query:   device_query.o
		$(CXX) $(LDFLAGS) -o $@ device_query.o $(LDLIBS)

cldemo:         cldemo.o
		$(CC) $(LDFLAGS) -o $@ cldemo.o $(LDLIBS)

%.o:            %.cpp
		$(CXX) $(CXXFLAGS) -c $<

%.o:            %.c
		$(CC) $(CFLAGS) -O -I.. -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -c $<

clean:
		rm -f *.o $(EXEC)
