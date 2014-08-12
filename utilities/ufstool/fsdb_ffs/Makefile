#	$NetBSD: Makefile,v 1.1.1.1 1995/10/08 23:08:36 thorpej Exp $
#	@(#)Makefile	8.1 (Berkeley) 6/5/93
# $FreeBSD$

PROG=	fsdb
MAN=	fsdb.8
SRCS=	fsdb.c fsdbutil.c \
	ckdir.c ckutil.c ckinode.c pass1.c pass1b.c pass2.c pass3.c pass4.c \
	pass5.c cksetup.c ffs_subr.c ffs_tables.c
CFLAGS+= -I${.CURDIR}/../fsck_ffs
WARNS?= 2
LDADD=	-ledit -ltermcap
DPADD=	${LIBEDIT} ${LIBTERMCAP}
CFLAGS+= -I${.CURDIR}/.. -I${.CURDIR}/../libufs -DMAXBSIZE=8192 -Werror
LDFLAGS+= -L${.CURDIR}/../libufs

.PATH:	${.CURDIR}/../fsck_ffs ${.CURDIR}/../ufs/ffs

.include <bsd.prog.mk>
