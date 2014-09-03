/*
 * Copyright (c) 2002 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project by Marshall
 * Kirk McKusick and Network Associates Laboratories, the Security
 * Research Division of Network Associates, Inc. under DARPA/SPAWAR
 * contract N66001-01-C-8035 ("CBOSS"), as part of the DARPA CHATS
 * research program.
 *
 * Copyright (c) 1980, 1989, 1993
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
 * 4. Neither the name of the University nor the names of its contributors
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
 * $FreeBSD$
 */

/*
 * The following two constants set the default block and fragment sizes.
 * Both constants must be a power of 2 and meet the following constraints:
 *	MINBSIZE <= DESBLKSIZE <= MAXBSIZE
 *	sectorsize <= DESFRAGSIZE <= DESBLKSIZE
 *	DESBLKSIZE / DESFRAGSIZE <= 8
 */
#define	DFL_FRAGSIZE	4096
#define	DFL_BLKSIZE	32768

/*
 * variables set up by front end.
 */
extern int	mkfs_Eflag;		/* Erase previous disk contents */
extern int	mkfs_Lflag;		/* add a volume label */
extern int	mkfs_Nflag;		/* run mkfs without writing file system */
extern int	mkfs_Oflag;		/* build UFS1 format file system */
extern int	mkfs_Rflag;		/* regression test */
extern int	mkfs_Uflag;		/* enable soft updates for file system */
extern int	mkfs_Xflag;		/* exit in middle of newfs for testing */
extern int	mkfs_Jflag;		/* enable gjournal for file system */
extern int	mkfs_lflag;		/* enable multilabel MAC for file system */
extern int	mkfs_nflag;		/* do not create .snap directory */
extern int	mkfs_tflag;		/* enable TRIM */
extern int64_t	mkfs_fssize;		/* file system size */
extern int64_t	mkfs_mediasize;         /* device size */
extern int	mkfs_sectorsize;	/* bytes/sector */
extern int	mkfs_realsectorsize;	/* bytes/sector in hardware*/
extern int	mkfs_fsize;		/* fragment size */
extern int	mkfs_bsize;		/* block size */
extern int	mkfs_maxbsize;          /* maximum clustering */
extern int	mkfs_maxblkspercg;	/* maximum blocks per cylinder group */
extern int	mkfs_minfree;           /* free space threshold */
extern int	mkfs_metaspace;         /* space held for metadata blocks */
extern int	mkfs_opt;		/* optimization preference (space or time) */
extern int	mkfs_density;           /* number of bytes per inode */
extern int	mkfs_maxcontig;         /* max contiguous blocks to allocate */
extern int	mkfs_maxbpg;		/* maximum blocks per file in a cyl group */
extern int	mkfs_avgfilesize;	/* expected average file size */
extern int	mkfs_avgfilesperdir;	/* expected number of files per directory */
extern u_char	*mkfs_volumelabel;	/* volume label for filesystem */

/*
 * To override a limitation in libufs, export the offset (in sectors) of the
 * partition on the underlying media (file or disk). The value is used as
 * an offset for all accesses to the media through bread(), which is only
 * invoked directly in this program.
 * For bwrite() we need a different approach, namely override the library
 * version with one defined here. This is because bwrite() is called also
 * by the library function sbwrite() which we cannot intercept nor want to
 * rewrite. As a consequence, the internal version of bwrite() adds the
 * partition offset itself when calling the underlying function, pwrite().
 *
 * XXX This info really ought to go into the ufs_t, at which point
 * we can remove the above hack.
 */
extern ufs2_daddr_t mkfs_part_ofs;	/* partition offset in blocks */

void mkfs (ufs_t *disk, const char *fsys);
