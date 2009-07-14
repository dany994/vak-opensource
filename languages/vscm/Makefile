CFLAGS          = #-O -fstrength-reduce
CC              = gcc -Wall -g -DDEBUG

OBJS            = vscm.o func.o
TESTS           = cxr int list quote t member

vscm:           $(OBJS)
		$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o vscm

tests:          vscm $(TESTS)
		for i in $(TESTS); do vscm -v $$i > $$i.out;\
		diff $$i.std $$i.out; done

newtests:       vscm $(TESTS)
		for i in $(TESTS); do vscm -v $$i > $$i.std; done

clean:;         rm -f *.o *.b .,* core.* core a.out vscm *.out

###
$(OBJS): vscm.h
