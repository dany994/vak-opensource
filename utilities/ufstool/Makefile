# $FreeBSD$

PROGS           = ufstool ffsinfo newfs fsck fsdb dumpfs
#PROGS           = ffsinfo newfs fsck fsdb dumpfs

OBJS_UFSTOOL    = ufstool.o manifest.o mount.o libufs.a
OBJS_NEWFS      = newfs.o mkfs.o libufs.a
OBJS_FFSINFO    = ffsinfo.o debug.o libufs.a
OBJS_DUMPFS     = dumpfs.o libufs.a
OBJS_FSCK       = ckdir.o ckutil.o ckinode.o fsck.o pass1.o pass1b.o pass2.o \
                  pass3.o pass4.o pass5.o cksetup.o suj.o gjournal.o \
                  libufs.a
OBJS_FSDB       = fsdb.o fsdbutil.o ckdir.o ckutil.o ckinode.o pass1.o \
                  pass1b.o pass2.o pass3.o pass4.o pass5.o cksetup.o \
                  ffs_subr.o ffs_tables.o libufs.a
OBJS_LIBUFS     = block.o cgroup.o inode.o sblock.o type.o ffs_subr.o ffs_tables.o

CFLAGS          = -O1 -g -Werror
#CFLAGS         += -D_LIBUFS_DEBUGGING
LDFLAGS         = -g

# Fuse
MOUNT_CFLAGS    = $(shell pkg-config fuse --cflags)
LIBS            += $(shell pkg-config fuse --libs)

all:            $(PROGS)

ufstool:        $(OBJS_UFSTOOL)
		$(CC) $(LDFLAGS) $(OBJS_UFSTOOL) $(LIBS) -o $@

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

libufs.a:       $(OBJS_LIBUFS)
		@rm -f $@
		$(AR) cr $@ $(OBJS_LIBUFS)

mount.o:        mount.c libufs.h fs.h
		$(CC) $(CFLAGS) $(MOUNT_CFLAGS) -c -o $@ $<

clean:
		rm -f *.o *.a $(PROGS)
