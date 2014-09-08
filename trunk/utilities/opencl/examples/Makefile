UNAME_S := $(shell uname -s)
# -std=c++11 -Wall -march=native
ifeq ($(UNAME_S),Linux)
CXX=clang++
CPPFLAGS=-O3
LDFLAGS=-O3
LDLIBS=-lOpenCL
endif

ifeq ($(UNAME_S),Darwin)
CXX=clang++
CPPFLAGS=-O3 -I..
LDFLAGS=-O3
LDLIBS=-framework OpenCL
endif

RM=rm -f

SRCS=device_query.cpp
OBJS=device_query.o
EXEC=device_query

all: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXEC) $(OBJS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c $<

clean:
	$(RM) $(OBJS) $(EXEC)

dist-clean:
	$(RM) $(EXEC)
