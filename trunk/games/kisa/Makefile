PC		= gpc

all:		kisa

clean:
		rm -f *~

kisa:		kisa.o
		$(PC) kisa.o -o $@

.pas.o:
		$(PC) -c $<

.SUFFIXES:	.pas
