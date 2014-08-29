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
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _LIBUFS
#include "libufs.h"

int verbose;

ssize_t
ufs_sector_read(ufs_t *disk, ufs2_daddr_t sectno, void *data, size_t size)
{
    ssize_t cnt;

    ERROR(disk, NULL);

    cnt = pread(disk->d_fd, data, size, (off_t)(sectno * disk->d_bsize));
    if (cnt == -1) {
        ERROR(disk, "read error from block device");
        goto fail;
    }
    if (cnt == 0) {
        ERROR(disk, "end of file from block device");
        goto fail;
    }
    if ((size_t)cnt != size) {
        ERROR(disk, "short read or read error from block device");
        goto fail;
    }
    return (cnt);
fail:
    memset(data, 0, size);
    return (-1);
}

ssize_t
ufs_sector_write(ufs_t *disk, ufs2_daddr_t sectno, const void *data, size_t size)
{
    ssize_t cnt;
    int rv;

    ERROR(disk, NULL);

    rv = ufs_disk_reopen_writable(disk);
    if (rv == -1) {
        ERROR(disk, "failed to open disk for writing");
        return (-1);
    }

    cnt = pwrite(disk->d_fd, data, size, (off_t)(sectno * disk->d_bsize));
    if (cnt == -1) {
        ERROR(disk, "write error to block device");
        return (-1);
    }
    if ((size_t)cnt != size) {
        ERROR(disk, "short write to block device");
        return (-1);
    }
    return (cnt);
}

int
ufs_sector_erase(ufs_t *disk, ufs2_daddr_t sectno, ufs2_daddr_t size)
{
    char *zero_chunk;
    off_t offset, zero_chunk_size, pwrite_size;
    int rv;

    ERROR(disk, NULL);
    rv = ufs_disk_reopen_writable(disk);
    if (rv == -1) {
        ERROR(disk, "failed to open disk for writing");
        return(rv);
    }

    offset = sectno * disk->d_bsize;
    zero_chunk_size = 65536 * disk->d_bsize;
    zero_chunk = calloc(1, zero_chunk_size);
    if (zero_chunk == NULL) {
        ERROR(disk, "failed to allocate memory");
        return (-1);
    }
    while (size > 0) {
        pwrite_size = size;
        if (pwrite_size > zero_chunk_size)
            pwrite_size = zero_chunk_size;
        rv = pwrite(disk->d_fd, zero_chunk, pwrite_size, offset);
        if (rv == -1) {
            ERROR(disk, "failed writing to disk");
            break;
        }
        size -= rv;
        offset += rv;
        rv = 0;
    }
    free(zero_chunk);
    return (rv);
}

int
ufs_seek (ufs_t *disk, off_t offset)
{
    unsigned bsize = disk->d_fs.fs_bsize;

/*  printf ("seek %ld, block %ld\n", offset, offset / bsize);*/
    if (lseek (disk->d_fd, offset, 0) < 0) {
        if (verbose)
            printf ("error seeking %lld, block %d\n",
                (long long)offset, (int) (offset / bsize));
        return -1;
    }
    disk->seek = offset;
    return 0;
}

int
ufs_read8 (ufs_t *disk, unsigned char *val)
{
    unsigned bsize = disk->d_fs.fs_bsize;

    if (read (disk->d_fd, val, 1) != 1) {
        if (verbose)
            printf ("error read8, seek %lld block %d\n",
                (long long)disk->seek, (int) (disk->seek / bsize));
        return -1;
    }
    return 0;
}

int
ufs_read16 (ufs_t *disk, unsigned short *val)
{
    unsigned bsize = disk->d_fs.fs_bsize;
    unsigned char data [2];

    if (read (disk->d_fd, data, 2) != 2) {
        if (verbose)
            printf ("error read16, seek %lld block %d\n",
                (long long)disk->seek, (int) (disk->seek / bsize));
        return -1;
    }
    *val = data[1] << 8 | data[0];
    return 0;
}

int
ufs_read32 (ufs_t *disk, unsigned *val)
{
    unsigned bsize = disk->d_fs.fs_bsize;
    unsigned char data [4];

    if (read (disk->d_fd, data, 4) != 4) {
        if (verbose)
            printf ("error read32, seek %lld block %d\n",
                (long long)disk->seek, (int) (disk->seek / bsize));
        return -1;
    }
    *val = (unsigned long) data[0] | (unsigned long) data[1] << 8 |
        data[2] << 16 | data[3] << 24;
    return 0;
}

int
ufs_read64 (ufs_t *disk, unsigned long long *val)
{
    unsigned bsize = disk->d_fs.fs_bsize;
    unsigned char data [8];

    if (read (disk->d_fd, data, 8) != 8) {
        if (verbose)
            printf ("error read32, seek %lld block %d\n",
                (long long)disk->seek, (int) (disk->seek / bsize));
        return -1;
    }
    *val = (unsigned long) data[0] | (unsigned long) data[1] << 8 |
        data[2] << 16 | data[3] << 24 | (unsigned long long)data[4] << 32 |
        (unsigned long long)data[5] << 40 | (unsigned long long)data[6] << 48 |
        (unsigned long long)data[7] << 56;
    return 0;
}

int
ufs_write8 (ufs_t *disk, unsigned char val)
{
    if (write (disk->d_fd, &val, 1) != 1)
        return -1;
    return 0;
}

int
ufs_write16 (ufs_t *disk, unsigned short val)
{
    unsigned char data [2];

    data[0] = val;
    data[1] = val >> 8;
    if (write (disk->d_fd, data, 2) != 2)
        return -1;
    return 0;
}

int
ufs_write32 (ufs_t *disk, unsigned val)
{
    unsigned char data [4];

    data[0] = val;
    data[1] = val >> 8;
    data[2] = val >> 16;
    data[3] = val >> 24;
    if (write (disk->d_fd, data, 4) != 4)
        return -1;
    return 0;
}

/*
 * Get a block from free list.
 */
int
ufs_block_alloc (ufs_t *fs, unsigned int *bno)
{
#if 0
    int i;
    unsigned buf [MAXBSIZE / 4];
again:
    if (fs->nfree == 0)
        return -1;
    fs->nfree--;
    --fs->tfree;            /* Count total free blocks. */
    *bno = fs->free [fs->nfree];
    if (verbose)
        printf ("allocate new block %d from slot %d\n", *bno, fs->nfree);
    fs->free [fs->nfree] = 0;
    fs->dirty = 1;
    if (fs->nfree <= 0) {
        if (! fs_read_block (fs, *bno, (unsigned char*) buf))
            return -1;
        fs->nfree = buf[0];
        for (i=0; i<NICFREE; i++)
            fs->free[i] = buf[i+1];
    }
    if (*bno == 0)
        goto again;
    return 0;
#endif
    return -1;
}
