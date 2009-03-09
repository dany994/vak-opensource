DC		= gdc -Wall -g
DFLAGS		= -O
LDFLAGS		=
INSTDIR		= /usr/local

all:		decho sample mk_black

mk_black:	mk_black.o
		$(DC) $(LDFLAGS) mk_black.o -o $@

decho:		decho.o
		$(DC) $(LDFLAGS) decho.o -o $@

sample:		sample.o getopt.o
		$(DC) $(LDFLAGS) sample.o getopt.o -o $@

clean:
		rm -f decho sample *.o *~ a.out

.SUFFIXES:	.d

.d.o:
		$(DC) $(DFLAGS) -c $<
