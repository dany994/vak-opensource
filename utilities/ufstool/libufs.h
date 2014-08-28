/*
 * Copyright (c) 2002 Juli Mallett.  All rights reserved.
 *
 * This software was written by Juli Mallett <jmallett@FreeBSD.org> for the
 * FreeBSD project.  Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef __LIBUFS_H__
#define __LIBUFS_H__

#include <stdio.h>
#include "fs.h"

/*
 * libufs structures.
 */

/*
 * userland ufs disk.
 */
typedef struct {
    const char *d_name;     /* disk name */
    int d_ufs;              /* decimal UFS version */
    int d_fd;               /* raw device file descriptor */
    long d_bsize;           /* disk sector size in bytes */
    ufs2_daddr_t d_sblock;  /* superblock location */
    struct csum *d_sbcsum;  /* Superblock summary info */
    caddr_t d_inoblock;     /* inode block */
    ino_t d_inomin;         /* low inode */
    ino_t d_inomax;         /* high inode */
    union {
        struct fs d_fs;     /* filesystem information */
        char d_sb[MAXBSIZE];
                            /* superblock as buffer */
    } d_sbunion;
    union {
        struct cg d_cg;     /* cylinder group */
        char d_buf[MAXBSIZE];
                            /* cylinder group storage */
    } d_cgunion;
    int d_ccg;              /* current cylinder group */
    int d_lcg;              /* last cylinder group (in d_cg) */
    const char *d_error;    /* human readable disk error */
    int d_mine;             /* internal flags */
#define d_fs    d_sbunion.d_fs
#define d_sb    d_sbunion.d_sb
#define d_cg    d_cgunion.d_cg
    off_t seek;
} ufs_t;

/*
 * Userland inode.
 */
typedef struct {
    ufs_t           *disk;
    unsigned        number;
    int             dirty;              /* save needed */

    u_int16_t       mode;               /* file type and access mode */
    int16_t         nlink;              /* directory entries */
    u_int32_t       uid;                /* owner */
    u_int32_t       gid;                /* group */
    u_int64_t       size;               /* size */
    int32_t         blocks;             /* blocks actually held */
    ufs1_daddr_t    daddr [NDADDR];     /* direct device addresses constituting file */
    ufs1_daddr_t    iaddr [NIADDR];     /* indirect device addresses */
    u_int32_t       flags;              /* user defined flags */
    int32_t         atime;              /* time last accessed */
    int32_t         mtime;              /* time last modified */
    int32_t         ctime;              /* time created */
} ufs_inode_t;

/*
 * libufs macros (internal, non-exported).
 */
#ifdef  _LIBUFS
/*
 * Trace steps through libufs, to be used at entry and erroneous return.
 */
static inline void
ERROR(ufs_t *u, const char *str)
{
#ifdef  _LIBUFS_DEBUGGING
    if (str != NULL) {
        fprintf(stderr, "libufs: %s", str);
        if (errno != 0)
            fprintf(stderr, ": %s", strerror(errno));
        fprintf(stderr, "\n");
    }
#endif
    if (u != NULL)
        u->d_error = str;
}
#endif  /* _LIBUFS */

#undef btodb
#define btodb(bytes) ((unsigned)(bytes) >> 9)

#ifndef UF_NODUMP
# define UF_NODUMP      0x00000001
#endif
#ifndef UF_IMMUTABLE
# define UF_IMMUTABLE   0x00000002
#endif
#ifndef UF_APPEND
# define UF_APPEND      0x00000004
#endif
#ifndef UF_NOUNLINK
# define UF_NOUNLINK    0x00000010
#endif
#ifndef SF_ARCHIVED
# define SF_ARCHIVED    0x00010000
#endif
#ifndef SF_IMMUTABLE
# define SF_IMMUTABLE   0x00020000
#endif
#ifndef SF_APPEND
# define SF_APPEND      0x00040000
#endif
#ifndef SF_NOUNLINK
# define SF_NOUNLINK    0x00080000
#endif

__BEGIN_DECLS

/*
 * libufs prototypes.
 */

/*
 * block.c
 */
ssize_t ufs_sector_read(ufs_t *, ufs2_daddr_t, void *, size_t);
ssize_t ufs_sector_write(ufs_t *, ufs2_daddr_t, const void *, size_t);
int ufs_sector_erase(ufs_t *, ufs2_daddr_t, ufs2_daddr_t);
int ufs_seek (ufs_t *disk, off_t offset);
int ufs_read8 (ufs_t *disk, u_int8_t *val);
int ufs_read16 (ufs_t *disk, u_int16_t *val);
int ufs_read32 (ufs_t *disk, u_int32_t *val);
int ufs_read64 (ufs_t *disk, u_int64_t *val);
int ufs_write8 (ufs_t *disk, u_int8_t val);
int ufs_write16 (ufs_t *disk, u_int16_t val);
int ufs_write32 (ufs_t *disk, u_int32_t val);

/*
 * cgroup.c
 */
ufs2_daddr_t cgballoc(ufs_t *);
int cgbfree(ufs_t *, ufs2_daddr_t, long);
ino_t cgialloc(ufs_t *);
int cgread(ufs_t *);
int cgread1(ufs_t *, int);
int cgwrite(ufs_t *);
int cgwrite1(ufs_t *, int);
void ufs_print_cg(struct cg *cgr, FILE *out);


/*
 * inode.c
 */
int getino(ufs_t *, void **, ino_t, int *);
int putino(ufs_t *);
int ufs_inode_get (ufs_t *disk, ufs_inode_t *inode, unsigned inum);
void ufs_inode_print (ufs_inode_t *inode, FILE *out);
void ufs_inode_print_path (ufs_inode_t *inode, const char *dirname, const char *filename, FILE *out);
void ufs_inode_print_blocks (ufs_inode_t *inode, FILE *out);

/*
 * sblock.c
 */
int sbread(ufs_t *);
int sbwrite(ufs_t *, int);
void ufs_print(ufs_t *disk, FILE *out);

/*
 * type.c
 */
int ufs_disk_close(ufs_t *);
int ufs_disk_fillout(ufs_t *, const char *);
int ufs_disk_fillout_blank(ufs_t *, const char *);
int ufs_disk_reopen_writable(ufs_t *);

/*
 * ffs_subr.c
 */
void ffs_clrblock(struct fs *, u_char *, ufs1_daddr_t);
void ffs_clusteracct(struct fs *, struct cg *, ufs1_daddr_t, int);
void ffs_fragacct(struct fs *, int, int32_t [], int);
int ffs_isblock(struct fs *, u_char *, ufs1_daddr_t);
int ffs_isfreeblock(struct fs *, u_char *, ufs1_daddr_t);
void ffs_setblock(struct fs *, u_char *, ufs1_daddr_t);

/*
 * check.c
 */
void ufs_check(ufs_t *disk, const char *filesys, int verbose, int fix);

/*
 * mount.c
 */
int ufs_mount(ufs_t *disk, char *dirname);

__END_DECLS

#endif  /* __LIBUFS_H__ */
