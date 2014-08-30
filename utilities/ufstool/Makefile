# $FreeBSD$

PROGS           = ufstool

OBJS_UFSTOOL    = ufstool.o manifest.o mount.o libufs.a
OBJS_LIBUFS     = block.o cgroup.o disk.o inode.o sblock.o \
                  ffs_subr.o mkfs.o check.o check_suj.o

CFLAGS          = -O1 -g -Wall -Werror
#CFLAGS         += -D_LIBUFS_DEBUGGING
LDFLAGS         = -g

# Fuse
MOUNT_CFLAGS    = $(shell pkg-config --cflags fuse)
LIBS            += $(shell pkg-config --libs fuse)

all:            $(PROGS)

ufstool:        $(OBJS_UFSTOOL)
		$(CC) $(LDFLAGS) $(OBJS_UFSTOOL) $(LIBS) -o $@

libufs.a:       $(OBJS_LIBUFS)
		@rm -f $@
		$(AR) cr $@ $(OBJS_LIBUFS)

mount.o:        mount.c libufs.h fs.h
		$(CC) $(CFLAGS) $(MOUNT_CFLAGS) -c -o $@ $<

clean:
		rm -f *.o *.a $(PROGS)
		$(MAKE) -Cutils clean
###
block.o: block.c libufs.h fs.h dinode.h
cgroup.o: cgroup.c libufs.h fs.h dinode.h
check.o: check.c dir.h fs.h dinode.h
check_suj.o: check_suj.c dir.h libufs.h fs.h dinode.h
disk.o: disk.c libufs.h fs.h dinode.h
ffs_subr.o: ffs_subr.c fs.h dinode.h
inode.o: inode.c libufs.h fs.h dinode.h
mkfs.o: mkfs.c dir.h libufs.h fs.h dinode.h
mount.o: mount.c libufs.h fs.h dinode.h dir.h
sblock.o: sblock.c libufs.h fs.h dinode.h
ufstool.o: ufstool.c libufs.h fs.h dinode.h manifest.h
