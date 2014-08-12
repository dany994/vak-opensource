#	@(#)Makefile	8.1 (Berkeley) 6/5/93
# $FreeBSD$

PROG=	dumpfs
WARNS?=	2
DPADD=	${LIBUFS}
LDADD=	-lufs
MAN=	dumpfs.8
CFLAGS+= -I${.CURDIR}/.. -I${.CURDIR}/../libufs -DMAXBSIZE=8192 -Werror
LDFLAGS+= -L${.CURDIR}/../libufs

.include <bsd.prog.mk>
