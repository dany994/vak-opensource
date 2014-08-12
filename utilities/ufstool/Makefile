# $FreeBSD$

PROGS           = ffsinfo newfs fsck fsdb dumpfs

OBJS_LIBUFS     = block.o cgroup.o inode.o sblock.o type.o ffs_subr.o ffs_tables.o
OBJS_NEWFS      = newfs.o mkfs.o libufs.a
OBJS_FFSINFO    = ffsinfo.o debug.o libufs.a
OBJS_DUMPFS     = dumpfs.o libufs.a
OBJS_FSCK       = ckdir.o ckutil.o ckinode.o fsck.o pass1.o pass1b.o pass2.o \
                  pass3.o pass4.o pass5.o cksetup.o suj.o gjournal.o \
                  libufs.a
OBJS_FSDB       = fsdb.o fsdbutil.o ckdir.o ckutil.o ckinode.o pass1.o \
                  pass1b.o pass2.o pass3.o pass4.o pass5.o cksetup.o \
                  ffs_subr.o ffs_tables.o libufs.a

CFLAGS          = -O0 -g -Werror -D_LIBUFS -DMAXBSIZE=8192 -DBBSIZE=512 -DFS_DEBUG
LDFLAGS         = -g
#CFLAGS         += -D_LIBUFS_DEBUGGING

all:            $(PROGS)

libufs.a:       $(OBJS_LIBUFS)
		@rm -f $@
		$(AR) cr $@ $(OBJS_LIBUFS)

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

clean:
		rm -f *.o *.a $(PROGS)
