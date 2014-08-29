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
#include <time.h>

#define _LIBUFS
#include "libufs.h"
#include "dir.h"

extern int verbose;

int
getino(ufs_t *disk, void **dino, ino_t inode, int *mode)
{
    ino_t min, max;
    caddr_t inoblock;
    struct ufs1_dinode *dp1;
    struct ufs2_dinode *dp2;
    struct fs *fs;

    ERROR(disk, NULL);

    fs = &disk->d_fs;
    inoblock = disk->d_inoblock;
    min = disk->d_inomin;
    max = disk->d_inomax;

    if (inoblock == NULL) {
        inoblock = malloc(fs->fs_bsize);
        if (inoblock == NULL) {
            ERROR(disk, "unable to allocate inode block");
            return (-1);
        }
        disk->d_inoblock = inoblock;
    }
    if (inode < min || inode >= max) {
        ufs_sector_read(disk, fsbtodb(fs, ino_to_fsba(fs, inode)), inoblock, fs->fs_bsize);
        disk->d_inomin = min = inode - (inode % INOPB(fs));
        disk->d_inomax = max = min + INOPB(fs);
    }
    switch (disk->d_ufs) {
    case 1:
        dp1 = &((struct ufs1_dinode *)inoblock)[inode - min];
        *mode = dp1->di_mode & IFMT;
        *dino = dp1;
        return (0);
    case 2:
        dp2 = &((struct ufs2_dinode *)inoblock)[inode - min];
        *mode = dp2->di_mode & IFMT;
        *dino = dp2;
        return (0);
    default:
        break;
    }
    ERROR(disk, "unknown UFS filesystem type");
    return (-1);
}

int
putino(ufs_t *disk)
{
    struct fs *fs;

    fs = &disk->d_fs;
    if (disk->d_inoblock == NULL) {
        ERROR(disk, "No inode block allocated");
        return (-1);
    }
    if (ufs_sector_write(disk, fsbtodb(fs, ino_to_fsba(&disk->d_fs, disk->d_inomin)),
        disk->d_inoblock, disk->d_fs.fs_bsize) <= 0)
        return (-1);
    return (0);
}

int
ufs_inode_get (ufs_t *disk, ufs_inode_t *inode, unsigned inum)
{
    unsigned bno, i;
    off_t offset;
    u_int32_t freelink, gen;
    int32_t atimensec, mtimensec, ctimensec;

    if (disk->d_ufs != 1) {
        fprintf(stderr, "%s: Only UFS1 format supported\n", __func__);
        exit(-1);
    }
    memset (inode, 0, sizeof (*inode));
    inode->disk = disk;
    inode->number = inum;

    /* Inodes are numbered starting from 1. */
    bno = fsbtodb(&disk->d_fs, ino_to_fsba(&disk->d_fs, inum));
    if (inum == 0 || bno >= disk->d_fs.fs_old_size)
        return -1;

    offset = (bno * (off_t) disk->d_bsize) +
        (ino_to_fsbo(&disk->d_fs, inum) * sizeof(struct ufs1_dinode));
//printf("--- %s(inum = %u) bno=%u, offset=%llu, d_bsize=%lu \n", __func__, inum, bno, (unsigned long long) offset, disk->d_bsize);
    if (ufs_seek (disk, offset) < 0)
        return -1;

    if (ufs_read16 (disk, &inode->mode) < 0)
        return -1;
    if (ufs_read16 (disk, (u_int16_t*) &inode->nlink) < 0)
        return -1;
    if (ufs_read32 (disk, &freelink) < 0)
        return -1;
    if (ufs_read64 (disk, &inode->size) < 0)
        return -1;
    if (ufs_read32 (disk, (u_int32_t*) &inode->atime) < 0)
        return -1;
    if (ufs_read32 (disk, (u_int32_t*) &atimensec) < 0)
        return -1;
    if (ufs_read32 (disk, (u_int32_t*) &inode->mtime) < 0)
        return -1;
    if (ufs_read32 (disk, (u_int32_t*) &mtimensec) < 0)
        return -1;
    if (ufs_read32 (disk, (u_int32_t*) &inode->ctime) < 0)
        return -1;
    if (ufs_read32 (disk, (u_int32_t*) &ctimensec) < 0)
        return -1;
    for (i=0; i<NDADDR; ++i) {
        if (ufs_read32 (disk, (u_int32_t*) &inode->daddr[i]) < 0)
            return -1;
    }
    for (i=0; i<NIADDR; ++i) {
        if (ufs_read32 (disk, (u_int32_t*) &inode->iaddr[i]) < 0)
            return -1;
    }
    if (ufs_read32 (disk, &inode->flags) < 0)
        return -1;
    if (ufs_read32 (disk, (u_int32_t*) &inode->blocks) < 0)
        return -1;
    if (ufs_read32 (disk, &gen) < 0)
        return -1;
    if (ufs_read32 (disk, &inode->uid) < 0)
        return -1;
    if (ufs_read32 (disk, &inode->gid) < 0)
        return -1;
/*if (inode->mode) { ufs_inode_print (inode, stdout); printf ("---\n"); }*/
    if (verbose > 3)
        printf ("get inode %u\n", inode->number);
    return 0;
}

void
ufs_inode_print (ufs_inode_t *inode, FILE *out)
{
    int i;

    fprintf (out, "     I-node: %u\n", inode->number);
    fprintf (out, "       Type: %s\n",
        (inode->mode & IFMT) == IFDIR ? "Directory" :
        (inode->mode & IFMT) == IFCHR ? "Character device" :
        (inode->mode & IFMT) == IFBLK ? "Block device" :
        (inode->mode & IFMT) == IFREG ? "File" :
        (inode->mode & IFMT) == IFLNK ? "Symbolic link" :
        (inode->mode & IFMT) == IFSOCK? "Socket" :
        (inode->mode & IFMT) == IFWHT ? "Whiteout" :
        "Unknown");
    fprintf (out, "       Size: %llu bytes\n", (unsigned long long)inode->size);
    fprintf (out, "       Mode: %#o\n", inode->mode);

    fprintf (out, "            ");
    if (inode->mode & ISUID)  fprintf (out, " SUID");
    if (inode->mode & ISGID)  fprintf (out, " SGID");
    if (inode->mode & ISVTX)  fprintf (out, " SVTX");
    if (inode->mode & IREAD)  fprintf (out, " READ");
    if (inode->mode & IWRITE) fprintf (out, " WRITE");
    if (inode->mode & IEXEC)  fprintf (out, " EXEC");
    fprintf (out, "\n");

    fprintf (out, "      Links: %u\n", inode->nlink);
    fprintf (out, "   Owner id: %u\n", inode->uid);

    fprintf (out, "     Blocks:");
    for (i=0; i<NDADDR; ++i) {
        fprintf (out, " %u", inode->daddr[i]);
    }
    for (i=0; i<NIADDR; ++i) {
        fprintf (out, " %u", inode->iaddr[i]);
    }
    fprintf (out, "\n");

    time_t t = inode->ctime;
    fprintf (out, "    Created: %s", ctime(&t));
    t = inode->mtime;
    fprintf (out, "   Modified: %s", ctime(&t));
    t = inode->atime;
    fprintf (out, "Last access: %s", ctime(&t));
}

void
ufs_inode_print_path (ufs_inode_t *inode,
    const char *dirname, const char *filename, FILE *out)
{
    fprintf (out, "%s/%s", dirname, filename);
    switch (inode->mode & IFMT) {
    case IFDIR:
        if (filename[0] != 0)
            fprintf (out, "/");
        break;
    case IFCHR:
        fprintf (out, " - char %d %d",
            inode->daddr[0] >> 8, inode->daddr[0] & 0xff);
        break;
    case IFBLK:
        fprintf (out, " - block %d %d",
            inode->daddr[0] >> 8, inode->daddr[0] & 0xff);
        break;
    default:
        fprintf (out, " - %llu bytes", (unsigned long long)inode->size);
        break;
    }
    fprintf (out, "\n");
}

static void
print_indirect_block (ufs_t *disk, unsigned int bno, FILE *out)
{
    unsigned bsize = disk->d_fs.fs_bsize;
    unsigned inodes_per_block = INOPB(&disk->d_fs);
    unsigned nb;
    int32_t data [MAXBSIZE/4];
    int i;

    fprintf (out, " [%d]", bno);
    if (ufs_sector_read (disk, bno, data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return;
    }
    for (i=0; i<inodes_per_block; i++) {
        nb = data [i];
        if (nb)
            fprintf (out, " %d", nb);
    }
}

static void
print_double_indirect_block (ufs_t *disk, unsigned int bno, FILE *out)
{
    unsigned bsize = disk->d_fs.fs_bsize;
    unsigned inodes_per_block = INOPB(&disk->d_fs);
    unsigned nb;
    int32_t data [MAXBSIZE/4];
    int i;

    fprintf (out, " [%d]", bno);
    if (ufs_sector_read (disk, bno, data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return;
    }
    for (i=0; i<inodes_per_block; i++) {
        nb = data [i];
        if (nb)
            print_indirect_block (disk, nb, out);
    }
}

static void
print_triple_indirect_block (ufs_t *disk, unsigned int bno, FILE *out)
{
    unsigned bsize = disk->d_fs.fs_bsize;
    unsigned inodes_per_block = INOPB(&disk->d_fs);
    unsigned nb;
    int32_t data [MAXBSIZE/4];
    int i;

    fprintf (out, " [%d]", bno);
    if (ufs_sector_read (disk, bno, data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return;
    }
    for (i=0; i<inodes_per_block; i++) {
        nb = data [i];
        if (nb)
            print_indirect_block (disk, nb, out);
    }
}

void
ufs_inode_print_blocks (ufs_inode_t *inode, FILE *out)
{
    int i;

    if ((inode->mode & IFMT) == IFCHR ||
        (inode->mode & IFMT) == IFBLK)
        return;

    fprintf (out, "    ");
    for (i=0; i<NDADDR; ++i) {
        if (inode->daddr[i] == 0)
            continue;
        fprintf (out, " %d", inode->daddr[i]);
    }
    if (inode->iaddr[0] != 0)
        print_indirect_block (inode->disk, inode->iaddr[0], out);
    if (inode->iaddr[1] != 0)
        print_double_indirect_block (inode->disk, inode->iaddr[1], out);
    if (inode->iaddr[2] != 0)
        print_triple_indirect_block (inode->disk, inode->iaddr[2], out);
    fprintf (out, "\n");
}

void ufs_directory_scan (ufs_inode_t *dir, const char *dirname,
    ufs_directory_scanner_t scanner, void *arg)
{
    ufs_inode_t file;
    unsigned long offset;
    unsigned char name [MAXBSIZE - 12];
    struct direct dirent;

    /* Variable record per file */
    for (offset = 0; offset < dir->size; offset += dirent.d_reclen) {
        if (ufs_inode_read (dir, offset, (unsigned char*) &dirent, 8) < 0) {
            fprintf (stderr, "%s: read error at offset %ld\n",
                dirname[0] ? dirname : "/", offset);
            return;
        }
        if (dirent.d_ino == 0)
            continue;
//printf ("--- scan offset %lu: ino=%u, reclen=%u, type=%u, namlen=%u\n", offset, dirent.d_ino, dirent.d_reclen, dirent.d_type, dirent.d_namlen);
        if (ufs_inode_read (dir, offset+8, name, (dirent.d_namlen + 4) / 4 * 4) < 0) {
            fprintf (stderr, "%s: name read error at offset %ld\n",
                dirname[0] ? dirname : "/", offset);
            return;
        }
//printf ("--- scan offset %lu: name='%s'\n", offset, name);

        if ((name[0]=='.' && name[1]==0) ||
            (name[0]=='.' && name[1]=='.' && name[2]==0))
            continue;

        if (ufs_inode_get (dir->disk, &file, dirent.d_ino) < 0) {
            fprintf (stderr, "cannot scan inode %d\n", dirent.d_ino);
            continue;
        }
        scanner (dir, &file, dirname, (char*) name, arg);
    }
}

/*
 * Return the physical block number on a device given the
 * inode and the logical block number in a file.
 */
static unsigned
map_block (ufs_inode_t *inode, unsigned lbn)
{
    unsigned bsize = inode->disk->d_fs.fs_bsize;
    unsigned block [MAXBSIZE / 4];
    unsigned int nb, i, j, sh;
    unsigned nshift = ffs(NINDIR(&inode->disk->d_fs)) - 1;
    unsigned nmask = NINDIR(&inode->disk->d_fs) - 1;

    /*
     * Blocks 0..NDADDR-1 are direct blocks.
     */
    if (lbn < NDADDR) {
        /* small file algorithm */
        return inode->daddr [lbn];
    }

    /*
     * Addresses NIADDR..NIADDR+2
     * have single, double, triple indirect blocks.
     * The first step is to determine
     * how many levels of indirection.
     */
    sh = 0;
    nb = 1;
    lbn -= NDADDR;
    for (j=NIADDR; ; j--) {
        if (j == 0)
            return 0;
        sh += nshift;
        nb <<= nshift;
        if (lbn < nb)
            break;
        lbn -= nb;
    }

    /*
     * Fetch the first indirect block.
     */
    nb = inode->iaddr [NIADDR-j];
    if (nb == 0)
        return 0;

    /*
     * Fetch through the indirect blocks.
     */
    for(; j <= 3; j++) {
        if (ufs_sector_read (inode->disk, nb, (unsigned char*) block, bsize) < 0)
            return 0;

        sh -= nshift;
        i = (lbn >> sh) & nmask;
        nb = block [i];
        if (nb == 0)
            return 0;
    }
    return nb;
}

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 */
static unsigned
map_block_write (ufs_inode_t *inode, unsigned lbn)
{
    unsigned bsize = inode->disk->d_fs.fs_bsize;
    unsigned block [MAXBSIZE / 4];
    unsigned int nb, newb, sh, i, j;
    unsigned nshift = ffs(NINDIR(&inode->disk->d_fs)) - 1;
    unsigned nmask = NINDIR(&inode->disk->d_fs) - 1;

    /*
     * Blocks 0..NDADDR-1 are direct blocks.
     */
    if (lbn < NDADDR) {
        /* small file algorithm */
        nb = inode->daddr [lbn];
        if (nb != 0) {
            if (verbose)
                printf ("map logical block %d to physical %d\n", lbn, nb);
            return nb;
        }

        /* allocate new block */
        if (ufs_block_alloc (inode->disk, &nb) < 0)
            return 0;
        inode->daddr[lbn] = nb;
        inode->dirty = 1;
        return nb;
    }

    /*
     * Addresses NIADDR..NIADDR+2
     * have single, double, triple indirect blocks.
     * The first step is to determine
     * how many levels of indirection.
     */
    sh = 0;
    nb = 1;
    lbn -= NDADDR;
    for (j=NIADDR; ; j--) {
        if (j == 0)
            return 0;
        sh += nshift;
        nb <<= nshift;
        if (lbn < nb)
            break;
        lbn -= nb;
    }

    /*
     * Fetch the first indirect block.
     */
    nb = inode->iaddr [NIADDR-j];
    if (nb == 0) {
        if (ufs_block_alloc (inode->disk, &nb) < 0)
            return 0;
        if (verbose)
            printf ("inode %d: allocate new block %d\n", inode->number, nb);
        memset (block, 0, bsize);
        if (ufs_sector_write (inode->disk, nb, (unsigned char*) block, bsize) < 0)
            return 0;
        inode->iaddr [NIADDR-j] = nb;
        inode->dirty = 1;
    }

    /*
     * Fetch through the indirect blocks
     */
    for(; j <= 3; j++) {
        if (ufs_sector_read (inode->disk, nb, (unsigned char*) block, bsize) < 0)
            return 0;

        sh -= nshift;
        i = (lbn >> sh) & nmask;
        if (block [i] != 0)
            nb = block [i];
        else {
            /* Allocate new block. */
            if (ufs_block_alloc (inode->disk, &newb) < 0)
                return 0;
            if (verbose)
                printf ("inode %d: allocate new block %d\n", inode->number, newb);
            block[i] = newb;
            if (ufs_sector_write (inode->disk, nb, (unsigned char*) block, bsize) < 0)
                return 0;
            memset (block, 0, bsize);
            if (ufs_sector_write (inode->disk, newb, (unsigned char*) block, bsize) < 0)
                return 0;
            nb = newb;
        }
    }
    return nb;
}

int
ufs_inode_read (ufs_inode_t *inode, unsigned long offset,
    unsigned char *data, unsigned long bytes)
{
    unsigned bsize = inode->disk->d_fs.fs_bsize;
    unsigned char block [MAXBSIZE];
    unsigned long n;
    unsigned int bn, inblock_offset;

    if (bytes + offset > inode->size)
        return -1;
    while (bytes != 0) {
        inblock_offset = offset % bsize;
        n = bsize - inblock_offset;
        if (n > bytes)
            n = bytes;

//printf ("--- %s: offset %lu -> block %lu, inblock_offset=%u, n=%lu \n", __func__, offset, offset / bsize, inblock_offset, n);
        bn = map_block (inode, offset / bsize);
//printf ("---     map_block returned bn=%u \n", bn);
        if (bn == 0)
            return -1;

        if (ufs_sector_read (inode->disk, bn, block, bsize) < 0)
            return -1;
        memcpy (data, block + inblock_offset, n);
        data += n;
        offset += n;
        bytes -= n;
    }
    return 0;
}

int ufs_inode_write (ufs_inode_t *inode, unsigned long offset,
    unsigned char *data, unsigned long bytes)
{
    unsigned bsize = inode->disk->d_fs.fs_bsize;
    unsigned char block [MAXBSIZE];
    unsigned long n;
    unsigned int bn, inblock_offset;

    inode->mtime = time(NULL);
    while (bytes != 0) {
        inblock_offset = offset % bsize;
        n = bsize - inblock_offset;
        if (n > bytes)
            n = bytes;

        bn = map_block_write (inode, offset / bsize);
        if (bn == 0)
            return -1;
        if (inode->size < offset + n) {
            /* Increase file size. */
            inode->size = offset + n;
            inode->dirty = 1;
        }
        if (verbose)
            printf ("inode %d offset %ld: write %ld bytes to block %d\n",
                inode->number, offset, n, bn);

        if (n == bsize) {
            if (ufs_sector_write (inode->disk, bn, data, bsize) < 0)
                return -1;
        } else {
            if (ufs_sector_read (inode->disk, bn, block, bsize) < 0)
                return -1;
            memcpy (block + inblock_offset, data, n);
            if (ufs_sector_write (inode->disk, bn, block, bsize) < 0)
                return -1;
        }
        data += n;
        offset += n;
        bytes -= n;
    }
    return 0;
}
