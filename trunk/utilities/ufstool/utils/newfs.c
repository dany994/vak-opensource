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
 * Copyright (c) 1983, 1989, 1993, 1994
 *  The Regents of the University of California.  All rights reserved.
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
 */

/*
 * newfs: friendly front end to mkfs
 */
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <err.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef linux
#   include <linux/fs.h>
#else
#   include <sys/disk.h>
#endif

#include "dir.h"
#include "libufs.h"
#include "newfs.h"

static ufs_t disk;      /* libufs disk structure */

static void
usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\tnewfs [ options ] special-device [device-type]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t-E Erase previous disk content\n");
    fprintf(stderr, "\t-J Enable journaling via gjournal\n");
    fprintf(stderr, "\t-L volume label to add to superblock\n");
    fprintf(stderr, "\t-N do not create file system, just print out parameters\n");
    fprintf(stderr, "\t-O file system format: 1 => UFS1 (default), 2 => UFS2\n");
    fprintf(stderr, "\t-R regression test, suppress random factors\n");
    fprintf(stderr, "\t-S sector size\n");
    fprintf(stderr, "\t-U enable soft updates\n");
    fprintf(stderr, "\t-a maximum contiguous blocks\n");
    fprintf(stderr, "\t-b block size\n");
    fprintf(stderr, "\t-c blocks per cylinders group\n");
    fprintf(stderr, "\t-d maximum extent size\n");
    fprintf(stderr, "\t-e maximum blocks per file in a cylinder group\n");
    fprintf(stderr, "\t-f frag size\n");
    fprintf(stderr, "\t-g average file size\n");
    fprintf(stderr, "\t-h average files per directory\n");
    fprintf(stderr, "\t-i number of bytes per inode\n");
    fprintf(stderr, "\t-k space to hold for metadata blocks\n");
    fprintf(stderr, "\t-l enable multilabel MAC\n");
    fprintf(stderr, "\t-n do not create .snap directory\n");
    fprintf(stderr, "\t-m minimum free space %%\n");
    fprintf(stderr, "\t-o optimization preference (`space' or `time')\n");
    fprintf(stderr, "\t-r reserved sectors at the end of device\n");
    fprintf(stderr, "\t-s file system size (sectors)\n");
    fprintf(stderr, "\t-t enable TRIM\n");
    exit(1);
}

/*
 * Convert an expression of the following forms to a uint64_t.
 *  1) A positive decimal number.
 *  2) A positive decimal number followed by a 'b' or 'B' (mult by 1).
 *  3) A positive decimal number followed by a 'k' or 'K' (mult by 1 << 10).
 *  4) A positive decimal number followed by a 'm' or 'M' (mult by 1 << 20).
 *  5) A positive decimal number followed by a 'g' or 'G' (mult by 1 << 30).
 *  6) A positive decimal number followed by a 't' or 'T' (mult by 1 << 40).
 *  7) A positive decimal number followed by a 'p' or 'P' (mult by 1 << 50).
 *  8) A positive decimal number followed by a 'e' or 'E' (mult by 1 << 60).
 */
static int
expand_number(const char *buf, uint64_t *num)
{
    uint64_t number;
    unsigned shift;
    char *endptr;

    number = strtoumax(buf, &endptr, 0);

    if (endptr == buf) {
        /* No valid digits. */
        errno = EINVAL;
        return (-1);
    }

    switch (tolower((unsigned char)*endptr)) {
    case 'e':
        shift = 60;
        break;
    case 'p':
        shift = 50;
        break;
    case 't':
        shift = 40;
        break;
    case 'g':
        shift = 30;
        break;
    case 'm':
        shift = 20;
        break;
    case 'k':
        shift = 10;
        break;
    case 'b':
    case '\0': /* No unit. */
        *num = number;
        return (0);
    default:
        /* Unrecognized unit. */
        errno = EINVAL;
        return (-1);
    }

    if ((number << shift) >> shift != number) {
        /* Overflow */
        errno = ERANGE;
        return (-1);
    }

    *num = number << shift;
    return (0);
}

static int
expand_number_int(const char *buf, int *num)
{
    uint64_t num64;
    int rval;

    rval = expand_number(buf, &num64);
    if (rval < 0)
        return (rval);
    if (num64 > INT_MAX || (int64_t)num64 < INT_MIN) {
        errno = ERANGE;
        return (-1);
    }
    *num = (int)num64;
    return (0);
}

static void
getfssize(int64_t *fsz, const char *s, intmax_t disksize, intmax_t reserved)
{
    intmax_t available;

    available = disksize - reserved;
    if (available <= 0)
        errx(1, "%s: reserved not less than device size %jd",
            s, disksize);
    if (*fsz == 0)
        *fsz = available;
    else if (*fsz > available)
        errx(1, "%s: maximum file system size is %jd",
            s, available);
}

int
main(int argc, char *argv[])
{
    struct stat st;
    char *cp, *special;
    intmax_t reserved;
    int ch, i, rval;

    reserved = 0;
    while ((ch = getopt(argc, argv,
        "EJL:NO:RS:UXa:b:c:d:e:f:g:h:i:k:lm:no:r:s:t")) != -1)
        switch (ch) {
        case 'E':
            mkfs_Eflag = 1;
            break;
        case 'J':
            mkfs_Jflag = 1;
            break;
        case 'L':
            mkfs_volumelabel = (u_char*) optarg;
            i = -1;
            while (isalnum(mkfs_volumelabel[++i]));
            if (mkfs_volumelabel[i] != '\0') {
                errx(1, "bad volume label. Valid characters are alphanumerics.");
            }
            if (strlen((char*)mkfs_volumelabel) >= MAXVOLLEN) {
                errx(1, "bad volume label. Length is longer than %d.",
                    MAXVOLLEN);
            }
            mkfs_Lflag = 1;
            break;
        case 'N':
            mkfs_Nflag = 1;
            break;
        case 'O':
            mkfs_Oflag = atoi(optarg);
            if (mkfs_Oflag < 1 || mkfs_Oflag > 2)
                errx(1, "%s: bad file system format value",
                    optarg);
            break;
        case 'R':
            mkfs_Rflag = 1;
            break;
        case 'S':
            rval = expand_number_int(optarg, &mkfs_sectorsize);
            if (rval < 0 || mkfs_sectorsize <= 0)
                errx(1, "%s: bad sector size", optarg);
            break;
        case 'U':
            mkfs_Uflag = 1;
            break;
        case 'X':
            mkfs_Xflag++;
            break;
        case 'a':
            rval = expand_number_int(optarg, &mkfs_maxcontig);
            if (rval < 0 || mkfs_maxcontig <= 0)
                errx(1, "%s: bad maximum contiguous blocks",
                    optarg);
            break;
        case 'b':
            rval = expand_number_int(optarg, &mkfs_bsize);
            if (rval < 0)
                 errx(1, "%s: bad block size",
                    optarg);
            if (mkfs_bsize < MINBSIZE)
                errx(1, "%s: block size too small, min is %d",
                    optarg, MINBSIZE);
            if (mkfs_bsize > MAXBSIZE)
                errx(1, "%s: block size too large, max is %d",
                    optarg, MAXBSIZE);
            break;
        case 'c':
            rval = expand_number_int(optarg, &mkfs_maxblkspercg);
            if (rval < 0 || mkfs_maxblkspercg <= 0)
                errx(1, "%s: bad blocks per cylinder group",
                    optarg);
            break;
        case 'd':
            rval = expand_number_int(optarg, &mkfs_maxbsize);
            if (rval < 0 || mkfs_maxbsize < MINBSIZE)
                errx(1, "%s: bad extent block size", optarg);
            break;
        case 'e':
            rval = expand_number_int(optarg, &mkfs_maxbpg);
            if (rval < 0 || mkfs_maxbpg <= 0)
              errx(1, "%s: bad blocks per file in a cylinder group",
                    optarg);
            break;
        case 'f':
            rval = expand_number_int(optarg, &mkfs_fsize);
            if (rval < 0 || mkfs_fsize <= 0)
                errx(1, "%s: bad fragment size", optarg);
            break;
        case 'g':
            rval = expand_number_int(optarg, &mkfs_avgfilesize);
            if (rval < 0 || mkfs_avgfilesize <= 0)
                errx(1, "%s: bad average file size", optarg);
            break;
        case 'h':
            rval = expand_number_int(optarg, &mkfs_avgfilesperdir);
            if (rval < 0 || mkfs_avgfilesperdir <= 0)
                   errx(1, "%s: bad average files per dir", optarg);
            break;
        case 'i':
            rval = expand_number_int(optarg, &mkfs_density);
            if (rval < 0 || mkfs_density <= 0)
                errx(1, "%s: bad bytes per inode", optarg);
            break;
        case 'l':
            mkfs_lflag = 1;
            break;
        case 'k':
            mkfs_metaspace = atoi(optarg);
            if (mkfs_metaspace < 0)
                errx(1, "%s: bad metadata space %%", optarg);
            if (mkfs_metaspace == 0)
                /* force to stay zero in mkfs */
                mkfs_metaspace = -1;
            break;
        case 'm':
            mkfs_minfree = atoi(optarg);
            if (mkfs_minfree < 0 || mkfs_minfree > 99)
                errx(1, "%s: bad free space %%", optarg);
            break;
        case 'n':
            mkfs_nflag = 1;
            break;
        case 'o':
            if (strcmp(optarg, "space") == 0)
                mkfs_opt = FS_OPTSPACE;
            else if (strcmp(optarg, "time") == 0)
                mkfs_opt = FS_OPTTIME;
            else
                errx(1,
        "%s: unknown optimization preference: use `space' or `time'",
                    optarg);
            break;
        case 'r':
            errno = 0;
            reserved = strtoimax(optarg, &cp, 0);
            if (errno != 0 || cp == optarg ||
                *cp != '\0' || reserved < 0)
                errx(1, "%s: bad reserved size", optarg);
            break;
        case 's':
            errno = 0;
            mkfs_fssize = strtoimax(optarg, &cp, 0);
            if (errno != 0 || cp == optarg ||
                *cp != '\0' || mkfs_fssize < 0)
                errx(1, "%s: bad file system size", optarg);
            break;
        case 't':
            mkfs_tflag = 1;
            break;
        case '?':
        default:
            usage();
        }
    argc -= optind;
    argv += optind;

    if (argc != 1)
        usage();
    special = argv[0];

    if (mkfs_sectorsize == 0)
        mkfs_sectorsize = mkfs_bsize ? mkfs_bsize : DEV_BSIZE;

    if (stat(special, &st) < 0) {
        /* File does not exist: need to create. */
        if (mkfs_fssize == 0)
            errx(1, "%s: file does not exist, use -s to specify size", special);
        if (mkfs_Nflag)
            errx(1, "%s: file does not exist, cannot create with -N flag", special);
        close(open(special, O_RDONLY | O_CREAT, 0664));
        mkfs_mediasize = mkfs_fssize * mkfs_sectorsize;
    }

    if (ufs_disk_open_blank(&disk, special) == -1 ||
        (!mkfs_Nflag && ufs_disk_reopen_writable(&disk) == -1)) {
        if (disk.d_error != NULL)
            errx(1, "%s: %s", special, disk.d_error);
        else
            err(1, "%s", special);
    }
    if (fstat(disk.d_fd, &st) < 0)
        err(1, "%s", special);

    if (mkfs_mediasize == 0) {
        /* Query media size. */
        if ((st.st_mode & S_IFMT) == S_IFREG) {
            /* Regular file. */
            mkfs_mediasize = st.st_size;
        } else {
            /* Assume device. */
#if defined(DKIOCGETBLOCKCOUNT)
            /* For Apple Darwin */
            unsigned long long numsectors;
            if (ioctl(disk.d_fd, DKIOCGETBLOCKCOUNT, &numsectors) < 0)
                errx(1, "%s: cannot get media size", special);
            mkfs_mediasize = numsectors << 9;
#elif defined(BLKGETSIZE64)
            /* For Linux. */
            unsigned long long numbytes;
            if (ioctl(disk.d_fd, BLKGETSIZE64, &numbytes) < 0)
                errx(1, "%s: cannot get media size", special);
            mkfs_mediasize = numbytes;
#else
            /* For BSD. */
            if (ioctl(disk.d_fd, DIOCGMEDIASIZE, &mkfs_mediasize) < 0)
                errx(1, "%s: cannot get media size", special);
#endif
        }
    }

    /* TODO: set fssize and part_ofs from the partition */
    if (mkfs_fssize == 0)
        getfssize(&mkfs_fssize, special, mkfs_mediasize / mkfs_sectorsize, reserved);

    if (mkfs_fsize <= 0)
        mkfs_fsize = MAX(DFL_FRAGSIZE, mkfs_sectorsize);
    if (mkfs_bsize <= 0)
        mkfs_bsize = MIN(DFL_BLKSIZE, 8 * mkfs_fsize);
    if (mkfs_minfree < MINFREE && mkfs_opt != FS_OPTSPACE) {
        fprintf(stderr, "Warning: changing optimization to space ");
        fprintf(stderr, "because minfree is less than %d%%\n", MINFREE);
        mkfs_opt = FS_OPTSPACE;
    }
    mkfs_realsectorsize = mkfs_sectorsize;
    if (mkfs_sectorsize != DEV_BSIZE) {     /* XXX */
        int secperblk = mkfs_sectorsize / DEV_BSIZE;

        mkfs_sectorsize = DEV_BSIZE;
        mkfs_fssize *= secperblk;
    }
    mkfs(&disk, special);
    ufs_disk_close(&disk);
    return 0;
}
