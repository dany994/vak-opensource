/*
 * Copyright (c) 2003 Juli Mallett.  All rights reserved.
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
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define _LIBUFS
#include "libufs.h"

ufs2_daddr_t
cgballoc(ufs_t *disk)
{
    u_int8_t *blksfree;
    struct cg *cgp;
    struct fs *fs;
    long bno;

    fs = &disk->d_fs;
    cgp = &disk->d_cg;
    blksfree = cg_blksfree(cgp);
    for (bno = 0; bno < fs->fs_fpg / fs->fs_frag; bno++) {
        if (ffs_isblock(fs, blksfree, bno))
            goto gotit;
    }
    return (0);
gotit:
    fs->fs_cs(fs, cgp->cg_cgx).cs_nbfree--;
    ffs_clrblock(fs, blksfree, (long)bno);
    ffs_clusteracct(fs, cgp, bno, -1);
    cgp->cg_cs.cs_nbfree--;
    fs->fs_cstotal.cs_nbfree--;
    fs->fs_fmod = 1;
    return (cgbase(fs, cgp->cg_cgx) + blkstofrags(fs, bno));
}

int
cgbfree(ufs_t *disk, ufs2_daddr_t bno, long size)
{
    u_int8_t *blksfree;
    struct fs *fs;
    struct cg *cgp;
    ufs1_daddr_t fragno, cgbno;
    int i, cg, blk, frags, bbase;

    fs = &disk->d_fs;
    cg = dtog(fs, bno);
    if (cgread1(disk, cg) != 1)
        return (-1);
    cgp = &disk->d_cg;
    cgbno = dtogd(fs, bno);
    blksfree = cg_blksfree(cgp);
    if (size == fs->fs_bsize) {
        fragno = fragstoblks(fs, cgbno);
        ffs_setblock(fs, blksfree, fragno);
        ffs_clusteracct(fs, cgp, fragno, 1);
        cgp->cg_cs.cs_nbfree++;
        fs->fs_cstotal.cs_nbfree++;
        fs->fs_cs(fs, cg).cs_nbfree++;
    } else {
        bbase = cgbno - fragnum(fs, cgbno);
        /*
         * decrement the counts associated with the old frags
         */
        blk = blkmap(fs, blksfree, bbase);
        ffs_fragacct(fs, blk, (int32_t*) cgp->cg_frsum, -1);
        /*
         * deallocate the fragment
         */
        frags = numfrags(fs, size);
        for (i = 0; i < frags; i++)
            setbit(blksfree, cgbno + i);
        cgp->cg_cs.cs_nffree += i;
        fs->fs_cstotal.cs_nffree += i;
        fs->fs_cs(fs, cg).cs_nffree += i;
        /*
         * add back in counts associated with the new frags
         */
        blk = blkmap(fs, blksfree, bbase);
        ffs_fragacct(fs, blk, (int32_t*) cgp->cg_frsum, 1);
        /*
         * if a complete block has been reassembled, account for it
         */
        fragno = fragstoblks(fs, bbase);
        if (ffs_isblock(fs, blksfree, fragno)) {
            cgp->cg_cs.cs_nffree -= fs->fs_frag;
            fs->fs_cstotal.cs_nffree -= fs->fs_frag;
            fs->fs_cs(fs, cg).cs_nffree -= fs->fs_frag;
            ffs_clusteracct(fs, cgp, fragno, 1);
            cgp->cg_cs.cs_nbfree++;
            fs->fs_cstotal.cs_nbfree++;
            fs->fs_cs(fs, cg).cs_nbfree++;
        }
    }
    return cgwrite(disk);
}

ino_t
cgialloc(ufs_t *disk)
{
    struct ufs2_dinode *dp2;
    u_int8_t *inosused;
    struct cg *cgp;
    struct fs *fs;
    ino_t ino;
    int i;

    fs = &disk->d_fs;
    cgp = &disk->d_cg;
    inosused = cg_inosused(cgp);
    for (ino = 0; ino < fs->fs_ipg; ino++) {
        if (isclr(inosused, ino))
            goto gotit;
    }
    return (0);
gotit:
    if (fs->fs_magic == FS_UFS2_MAGIC &&
        ino + INOPB(fs) > cgp->cg_initediblk &&
        cgp->cg_initediblk < cgp->cg_niblk) {
        char block[MAXBSIZE];
        bzero(block, (int)fs->fs_bsize);
        dp2 = (struct ufs2_dinode *)&block;
        for (i = 0; i < INOPB(fs); i++) {
            dp2->di_gen = random() / 2 + 1;
            dp2++;
        }
        if (ufs_sector_write(disk, ino_to_fsba(fs,
            cgp->cg_cgx * fs->fs_ipg + cgp->cg_initediblk),
            block, fs->fs_bsize)) {
            return (0);
        }
        cgp->cg_initediblk += INOPB(fs);
    }

    setbit(inosused, ino);
    cgp->cg_irotor = ino;
    cgp->cg_cs.cs_nifree--;
    fs->fs_cstotal.cs_nifree--;
    fs->fs_cs(fs, cgp->cg_cgx).cs_nifree--;
    fs->fs_fmod = 1;

    return (ino + (cgp->cg_cgx * fs->fs_ipg));
}

int
cgread(ufs_t *disk)
{
    return (cgread1(disk, disk->d_ccg++));
}

int
cgread1(ufs_t *disk, int c)
{
    struct fs *fs;

    fs = &disk->d_fs;

    if ((unsigned)c >= fs->fs_ncg) {
        return (0);
    }
    if (ufs_sector_read(disk, fsbtodb(fs, cgtod(fs, c)), disk->d_cgunion.d_buf,
        fs->fs_bsize) == -1) {
        ERROR(disk, "unable to read cylinder group");
        return (-1);
    }
    disk->d_lcg = c;
    return (1);
}

int
cgwrite(ufs_t *disk)
{
    return (cgwrite1(disk, disk->d_lcg));
}

int
cgwrite1(ufs_t *disk, int c)
{
    struct fs *fs;

    fs = &disk->d_fs;
    if (ufs_sector_write(disk, fsbtodb(fs, cgtod(fs, c)),
        disk->d_cgunion.d_buf, fs->fs_bsize) == -1) {
        ERROR(disk, "unable to write cylinder group");
        return (-1);
    }
    return (0);
}

/*
 * Dump a cylinder group.
 */
void
ufs_print_cg(struct cg *cgr, FILE *out)
{
    int j;

    fprintf(out, "                     Magic number: %#x\n", cgr->cg_magic);
    fprintf(out, "                Last time written: %s", ctime((const time_t*)&cgr->cg_old_time));
    fprintf(out, "             Cylinder group index: %d\n", cgr->cg_cgx);
    fprintf(out, "              Number of cylinders: %d\n", cgr->cg_old_ncyl);
    fprintf(out, "          Number of inode sectors: %d\n", cgr->cg_old_niblk);
    fprintf(out, "           Number of data sectors: %d\n", cgr->cg_ndblk);
    fprintf(out, "                 Number of blocks: %d\n", cgr->cg_nclusterblks);

    /* Cylinder summary information */
    fprintf(out, "            Number of directories: %d\n", cgr->cg_cs.cs_ndir);
    fprintf(out, "            Number of free blocks: %d\n", cgr->cg_cs.cs_nbfree);
    fprintf(out, "            Number of free inodes: %d\n", cgr->cg_cs.cs_nifree);
    fprintf(out, "             Number of free frags: %d\n", cgr->cg_cs.cs_nffree);

    fprintf(out, "Rotational pos of last used block: %d\n", cgr->cg_rotor);
    fprintf(out, " Rotational pos of last used frag: %d\n", cgr->cg_frotor);
    fprintf(out, "Rotational pos of last used inode: %d\n", cgr->cg_irotor);

    /* Counts of available frags */
    fprintf(out, "        Counts of available frags:");
    for (j = 0; j < MAXFRAG; j++) {
        if (j)
            fprintf(out, ",");
        fprintf(out, " %d", cgr->cg_frsum[j]);
    }
    fprintf(out, "\n");
    fprintf(out, "   Offset of block totals per cyl: %d bytes\n", cgr->cg_old_btotoff);
    fprintf(out, "   Offset of free block positions: %d bytes\n", cgr->cg_old_boff);
    fprintf(out, "         Offset of used inode map: %d bytes\n", cgr->cg_iusedoff);
    fprintf(out, "         Offset of free block map: %d bytes\n", cgr->cg_freeoff);
    fprintf(out, "   Offset of next available space: %d bytes\n", cgr->cg_nextfreeoff);
    fprintf(out, " Offset of counts of avail blocks: %d bytes\n", cgr->cg_clustersumoff);
    fprintf(out, "         Offset of free block map: %d bytes\n", cgr->cg_clusteroff);
}
