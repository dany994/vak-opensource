PROGS           = ffsinfo newfs fsck fsdb dumpfs

OBJS_NEWFS      = newfs.o ../libufs.a
OBJS_FFSINFO    = ffsinfo.o debug.o ../libufs.a
OBJS_DUMPFS     = dumpfs.o ../libufs.a
OBJS_FSCK       = fsck.o check_gjournal.o ../libufs.a
OBJS_FSDB       = fsdb.o fsdbutil.o ../libufs.a

CFLAGS          = -O1 -g -Wall -Werror -I..
#CFLAGS         += -D_LIBUFS_DEBUGGING
LDFLAGS         = -g

all:            $(PROGS)

ffsinfo:        $(OBJS_FFSINFO)
		$(CC) $(LDFLAGS) $(OBJS_FFSINFO) -o $@

newfs:          $(OBJS_NEWFS)
		$(CC) $(LDFLAGS) $(OBJS_NEWFS) -o $@

fsck:           $(OBJS_FSCK)
		$(CC) $(LDFLAGS) $(OBJS_FSCK) -o $@

fsdb:           $(OBJS_FSDB)
		$(CC) $(LDFLAGS) $(OBJS_FSDB) -ledit -ltermcap -o $@

dumpfs:         $(OBJS_DUMPFS)
		$(CC) $(LDFLAGS) $(OBJS_DUMPFS) -o $@

../libufs.a:
		$(MAKE) -C.. libufs.a

clean:
		rm -f *.o *.a $(PROGS)
###
debug.o: debug.c ../fs.h ../dinode.h debug.h
dumpfs.o: dumpfs.c ../libufs.h ../fs.h ../dinode.h
ffsinfo.o: ffsinfo.c ../libufs.h ../fs.h ../dinode.h debug.h
fsck.o: fsck.c ../fs.h ../dinode.h
fsdb.o: fsdb.c ../dir.h ../fs.h ../dinode.h fsdb.h
fsdbutil.o: fsdbutil.c ../fs.h ../dinode.h fsdb.h
check_gjournal.o: check_gjournal.c ../libufs.h ../fs.h ../dinode.h
newfs.o: newfs.c ../dir.h ../libufs.h ../fs.h ../dinode.h newfs.h
