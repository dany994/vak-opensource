/*	$NetBSD: endian.h,v 1.28 2009/08/08 21:23:15 christos Exp $	*/

/*
 * Copyright (c) 1987, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)endian.h	8.1 (Berkeley) 6/11/93
 */

#ifndef _SYS_ENDIAN_H_
#define _SYS_ENDIAN_H_

/*
 * Definitions for byte order, according to byte significance from low
 * address to high.
 */
#define	_LITTLE_ENDIAN	1234	/* LSB first: i386, vax */
#define	_BIG_ENDIAN	4321	/* MSB first: 68000, ibm, net */
#define	_PDP_ENDIAN	3412	/* LSB first in word, MSW first in long */


//#include <machine/endian_machdep.h>

/*
 * Define the order of 32-bit words in 64-bit words.
 */
#if _BYTE_ORDER == _LITTLE_ENDIAN
#define _QUAD_HIGHWORD 1
#define _QUAD_LOWWORD 0
#endif

#if _BYTE_ORDER == _BIG_ENDIAN
#define _QUAD_HIGHWORD 0
#define _QUAD_LOWWORD 1
#endif


/*
 *  Traditional names for byteorder.  These are defined as the numeric
 *  sequences so that third party code can "#define XXX_ENDIAN" and not
 *  cause errors.
 */
//#define	LITTLE_ENDIAN	1234		/* LSB first: i386, vax */
//#define	BIG_ENDIAN	4321		/* MSB first: 68000, ibm, net */
//#define	PDP_ENDIAN	3412		/* LSB first in word, MSW first in long */
//#define BYTE_ORDER	_BYTE_ORDER

/*
 * Macros to convert to a specific endianness.
 */

#if BYTE_ORDER == BIG_ENDIAN

#define htobe16(x)	(x)
#define htobe32(x)	(x)
#define htobe64(x)	(x)
#define htole16(x)	bswap16((u_int16_t) (x))
#define htole32(x)	bswap32((u_int32_t) (x))
#define htole64(x)	bswap64((u_int64_t) (x))

#define HTOBE16(x)	(void) (x)
#define HTOBE32(x)	(void) (x)
#define HTOBE64(x)	(void) (x)
#define HTOLE16(x)	(x) = bswap16(__CAST(u_int16_t, (x)))
#define HTOLE32(x)	(x) = bswap32(__CAST(u_int32_t, (x)))
#define HTOLE64(x)	(x) = bswap64(__CAST(u_int64_t, (x)))

#else	/* LITTLE_ENDIAN */

#define htobe16(x)	bswap16(__CAST(u_int16_t, (x)))
#define htobe32(x)	bswap32(__CAST(u_int32_t, (x)))
#define htobe64(x)	bswap64(__CAST(u_int64_t, (x)))
#define htole16(x)	(x)
#define htole32(x)	(x)
#define htole64(x)	(x)

#define HTOBE16(x)	(x) = bswap16(__CAST(u_int16_t, (x)))
#define HTOBE32(x)	(x) = bswap32(__CAST(u_int32_t, (x)))
#define HTOBE64(x)	(x) = bswap64(__CAST(u_int64_t, (x)))
#define HTOLE16(x)	__CAST(void, (x))
#define HTOLE32(x)	__CAST(void, (x))
#define HTOLE64(x)	__CAST(void, (x))

#endif	/* LITTLE_ENDIAN */

#define be16toh(x)	htobe16(x)
#define be32toh(x)	htobe32(x)
#define be64toh(x)	htobe64(x)
#define le16toh(x)	htole16(x)
#define le32toh(x)	htole32(x)
#define le64toh(x)	htole64(x)

#define BE16TOH(x)	HTOBE16(x)
#define BE32TOH(x)	HTOBE32(x)
#define BE64TOH(x)	HTOBE64(x)
#define LE16TOH(x)	HTOLE16(x)
#define LE32TOH(x)	HTOLE32(x)
#define LE64TOH(x)	HTOLE64(x)

/*
 * Routines to encode/decode big- and little-endian multi-octet values
 * to/from an octet stream.
 */

static __inline void __unused
be16enc(void *buf, u_int16_t u)
{
	u_int8_t *p = (u_int8_t *) buf;

	p[0] = u >> 8;
	p[1] = u;
}

static __inline void __unused
le16enc(void *buf, u_int16_t u)
{
	u_int8_t *p = (u_int8_t *) buf;

	p[0] = u;
	p[1] = u >> 8;
}

static __inline u_int16_t __unused
be16dec(const void *buf)
{
	const u_int8_t *p = (const u_int8_t *) buf;

	return (p[0] << 8) | p[1];
}

static __inline u_int16_t __unused
le16dec(const void *buf)
{
	const u_int8_t *p = (const u_int8_t *) buf;

	return (p[1] << 8) | p[0];
}

static __inline void __unused
be32enc(void *buf, u_int32_t u)
{
	u_int8_t *p = (u_int8_t *) buf;

	p[0] = u >> 24;
	p[1] = u >> 16;
	p[2] = u >> 8;
	p[3] = u;
}

static __inline void __unused
le32enc(void *buf, u_int32_t u)
{
	u_int8_t *p = (u_int8_t *) buf;

	p[0] = u;
	p[1] = u >> 8;
	p[2] = u >> 16;
	p[3] = u >> 24;
}

static __inline u_int32_t __unused
be32dec(const void *buf)
{
	const u_int8_t *p = (const u_int8_t *) buf;

	return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

static __inline u_int32_t __unused
le32dec(const void *buf)
{
	const u_int8_t *p = (const u_int8_t *) buf;

	return ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

static __inline void __unused
be64enc(void *buf, u_int64_t u)
{
	u_int8_t *p = (u_int8_t *) buf;

	be32enc(p, (u_int32_t) (u >> 32));
	be32enc(p + 4, (u_int32_t) u);
}

static __inline void __unused
le64enc(void *buf, u_int64_t u)
{
	u_int8_t *p = (u_int8_t *) buf;

	le32enc(p, (u_int32_t) u);
	le32enc(p + 4, (u_int32_t) (u >> 32));
}

static __inline u_int64_t __unused
be64dec(const void *buf)
{
	const u_int8_t *p = (const u_int8_t *)buf;

	return (((u_int64_t) be32dec(p)) << 32) | be32dec(p + 4);
}

static __inline u_int64_t __unused
le64dec(const void *buf)
{
	const u_int8_t *p = (const u_int8_t *)buf;

	return le32dec(p) | (((u_int64_t) le32dec(p + 4)) << 32);
}

#endif /* !_SYS_ENDIAN_H_ */
