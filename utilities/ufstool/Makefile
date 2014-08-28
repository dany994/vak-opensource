# $FreeBSD$

PROGS           = ufstool ffsinfo newfs fsck fsdb dumpfs
#PROGS           = ffsinfo newfs fsck fsdb dumpfs

OBJS_UFSTOOL    = ufstool.o manifest.o mount.o libufs.a
OBJS_NEWFS      = newfs.o libufs.a
OBJS_FFSINFO    = ffsinfo.o debug.o libufs.a
OBJS_DUMPFS     = dumpfs.o libufs.a
OBJS_FSCK       = fsck.o libufs.a
OBJS_FSDB       = fsdb.o fsdbutil.o ffs_subr.o ffs_tables.o libufs.a
OBJS_LIBUFS     = block.o cgroup.o inode.o sblock.o type.o \
                  ffs_subr.o ffs_tables.o mkfs.o \
                    check.o check_suj.o check_gjournal.o

CFLAGS          = -O1 -g -Wall -Werror
#CFLAGS         += -D_LIBUFS_DEBUGGING
LDFLAGS         = -g

# Fuse
MOUNT_CFLAGS    = $(shell pkg-config --cflags fuse)
LIBS            += $(shell pkg-config --libs fuse)

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
###
block.o: block.c libufs.h fs.h dinode.h
cgroup.o: cgroup.c libufs.h fs.h dinode.h
check.o: check.c dir.h fs.h dinode.h fsck.h
debug.o: debug.c fs.h dinode.h debug.h
dumpfs.o: dumpfs.c libufs.h fs.h dinode.h
ffsinfo.o: ffsinfo.c libufs.h fs.h dinode.h debug.h
ffs_subr.o: ffs_subr.c fs.h dinode.h
ffs_tables.o: ffs_tables.c fs.h dinode.h
fsck.o: fsck.c fs.h dinode.h fsck.h
fsdb.o: fsdb.c dir.h fs.h dinode.h fsdb.h fsck.h
fsdbutil.o: fsdbutil.c fs.h dinode.h fsdb.h fsck.h
gjournal.o: gjournal.c libufs.h fs.h dinode.h fsck.h
inode.o: inode.c libufs.h fs.h dinode.h
mkfs.o: mkfs.c dir.h libufs.h fs.h dinode.h newfs.h
mount.o: mount.c libufs.h fs.h dinode.h dir.h
newfs.o: newfs.c dir.h libufs.h fs.h dinode.h newfs.h
sblock.o: sblock.c libufs.h fs.h dinode.h
check_suj.o: check_suj.c dir.h libufs.h fs.h dinode.h fsck.h
type.o: type.c libufs.h fs.h dinode.h
ufstool.o: ufstool.c libufs.h fs.h dinode.h newfs.h manifest.h
