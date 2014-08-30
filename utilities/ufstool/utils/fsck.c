/*
 * Copyright (c) 1980, 1986, 1993
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

#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <err.h>
#include <time.h>

#include "libufs.h"
#include "internal.h"

static void usage(void);
static int argtoi(int flag, const char *req, const char *str, int base);
static int checkfilesys(char *filesys);

int
main(int argc, char *argv[])
{
    int ch;
    int ret = 0;

    sync();
    check_skipclean = 1;
    check_inoopt = 0;
    while ((ch = getopt(argc, argv, "b:c:CdEfm:nprSyZ")) != -1) {
        switch (ch) {
        case 'b':
            check_skipclean = 0;
            check_bflag = argtoi('b', "number", optarg, 10);
            printf("Alternate super block location: %d\n", check_bflag);
            break;

        case 'c':
            check_skipclean = 0;
            check_cvtlevel = argtoi('c', "conversion level", optarg, 10);
            if (check_cvtlevel < 3)
                errx(EEXIT, "cannot do level %d conversion",
                    check_cvtlevel);
            break;

        case 'd':
            check_debug++;
            break;

        case 'E':
            check_Eflag++;
            break;

        case 'f':
            check_skipclean = 0;
            break;

        case 'm':
            check_lfmode = argtoi('m', "mode", optarg, 8);
            if (check_lfmode &~ 07777)
                errx(EEXIT, "bad mode to -m: %o", check_lfmode);
            printf("** lost+found creation mode %o\n", check_lfmode);
            break;

        case 'n':
            check_nflag++;
            check_yflag = 0;
            break;

        case 'p':
            check_preen++;
            /*FALLTHROUGH*/

        case 'C':
            check_clean++;
            break;

        case 'r':
            check_inoopt++;
            break;

        case 'S':
            check_surrender = 1;
            break;

        case 'y':
            check_yflag++;
            check_nflag = 0;
            break;

        case 'Z':
            check_Zflag++;
            break;

        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;

    if (!argc)
        usage();

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        (void)signal(SIGINT, check_catch);
    if (check_clean)
        (void)signal(SIGQUIT, check_catchquit);

    while (argc-- > 0)
        (void)checkfilesys(*argv++);

    if (check_returntosingle)
        ret = 2;
    exit(ret);
}

static int
argtoi(int flag, const char *req, const char *str, int base)
{
    char *cp;
    int ret;

    ret = (int)strtol(str, &cp, base);
    if (cp == str || *cp)
        errx(EEXIT, "-%c flag requires a %s", flag, req);
    return (ret);
}

/*
 * Check the specified file system.
 */
/* ARGSUSED */
static int
checkfilesys(char *filesys)
{
    ufs2_daddr_t n_ffree, n_bfree;
    struct dups *dp;
    int cylno;
    intmax_t blks, files;

    check_filename = filesys;
    if (check_debug && check_clean)
        check_warn("starting\n");

    check_sblock_init();
    if (check_clean && check_skipclean) {
        /*
         * If file system is gjournaled, check it here.
         */
        if ((check_fsreadfd = open(filesys, O_RDONLY)) < 0 || check_readsb(0) == 0)
            exit(3);    /* Cannot read superblock */
        close(check_fsreadfd);
        if ((check_sblk.b_un.b_fs->fs_flags & FS_GJOURNAL) != 0) {
            //printf("GJournaled file system detected on %s.\n",
            //    filesys);
            if (check_sblk.b_un.b_fs->fs_clean == 1) {
                check_warn("FILE SYSTEM CLEAN; SKIPPING CHECKS\n");
                exit(0);
            }
            if ((check_sblk.b_un.b_fs->fs_flags & (FS_UNCLEAN | FS_NEEDSFSCK)) == 0) {
                check_gjournal(filesys);
                exit(0);
            } else {
                check_fatal(
                "UNEXPECTED INCONSISTENCY, CANNOT RUN FAST FSCK\n");
            }
        }
    }

    switch (check_setup(filesys, 0)) {
    case 0:
        if (check_preen)
            check_fatal("CAN'T CHECK FILE SYSTEM.");
        return (0);
    case -1:
        check_warn("clean, %ld free ", (long)(check_sblk.b_un.b_fs->fs_cstotal.cs_nffree +
            check_sblk.b_un.b_fs->fs_frag * check_sblk.b_un.b_fs->fs_cstotal.cs_nbfree));
        printf("(%jd frags, %jd blocks, %.1f%% fragmentation)\n",
            (intmax_t)check_sblk.b_un.b_fs->fs_cstotal.cs_nffree,
            (intmax_t)check_sblk.b_un.b_fs->fs_cstotal.cs_nbfree,
            check_sblk.b_un.b_fs->fs_cstotal.cs_nffree * 100.0 / check_sblk.b_un.b_fs->fs_dsize);
        return (0);
    }
    /*
     * Determine if we can and should do journal recovery.
     */
    if ((check_sblk.b_un.b_fs->fs_flags & FS_SUJ) == FS_SUJ) {
        if ((check_sblk.b_un.b_fs->fs_flags & FS_NEEDSFSCK) != FS_NEEDSFSCK && check_skipclean) {
            if (check_preen || check_reply("USE JOURNAL")) {
                if (check_suj(filesys) == 0) {
                    printf("\n***** FILE SYSTEM MARKED CLEAN *****\n");
                    exit(0);
                }
            }
            printf("** Skipping journal, falling through to full fsck\n\n");
        }
        /*
         * Write the superblock so we don't try to recover the
         * journal on another pass.
         */
        check_sblk.b_un.b_fs->fs_mtime = time(NULL);
        dirty(&check_sblk);
    }

    /*
     * Cleared if any questions answered no. Used to decide if
     * the superblock should be marked clean.
     */
    check_resolved = 1;
    /*
     * 1: scan inodes tallying blocks used
     */
    if (check_preen == 0) {
        printf("** Last Mounted on %s\n", check_sblk.b_un.b_fs->fs_fsmnt);
        printf("** Phase 1 - Check Blocks and Sizes\n");
    }
    check_pass1();

    /*
     * 1b: locate first references to duplicates, if any
     */
    if (check_duplist) {
        if (check_preen || check_usedsoftdep)
            check_fatal("INTERNAL ERROR: dups with %s%s%s",
                check_preen ? "-p" : "",
                (check_preen && check_usedsoftdep) ? " and " : "",
                check_usedsoftdep ? "softupdates" : "");
        printf("** Phase 1b - Rescan For More DUPS\n");
        check_pass1b();
    }

    /*
     * 2: traverse directories from root to mark all connected directories
     */
    if (check_preen == 0)
        printf("** Phase 2 - Check Pathnames\n");
    check_pass2();

    /*
     * 3: scan inodes looking for disconnected directories
     */
    if (check_preen == 0)
        printf("** Phase 3 - Check Connectivity\n");
    check_pass3();

    /*
     * 4: scan inodes looking for disconnected files; check reference counts
     */
    if (check_preen == 0)
        printf("** Phase 4 - Check Reference Counts\n");
    check_pass4();

    /*
     * 5: check and repair resource counts in cylinder groups
     */
    if (check_preen == 0)
        printf("** Phase 5 - Check Cyl groups\n");
    check_pass5();

    /*
     * print out summary statistics
     */
    n_ffree = check_sblk.b_un.b_fs->fs_cstotal.cs_nffree;
    n_bfree = check_sblk.b_un.b_fs->fs_cstotal.cs_nbfree;
    files = check_maxino - ROOTINO - check_sblk.b_un.b_fs->fs_cstotal.cs_nifree - check_n_files;
    blks = check_n_blks +
        check_sblk.b_un.b_fs->fs_ncg * (cgdmin(check_sblk.b_un.b_fs, 0) - cgsblock(check_sblk.b_un.b_fs, 0));
    blks += cgsblock(check_sblk.b_un.b_fs, 0) - cgbase(check_sblk.b_un.b_fs, 0);
    blks += howmany(check_sblk.b_un.b_fs->fs_cssize, check_sblk.b_un.b_fs->fs_fsize);
    blks = check_maxfsblock - (n_ffree + check_sblk.b_un.b_fs->fs_frag * n_bfree) - blks;
    check_warn("%ld files, %jd used, %ju free ",
        (long)check_n_files, (intmax_t)check_n_blks,
        (uintmax_t)(n_ffree + check_sblk.b_un.b_fs->fs_frag * n_bfree));
    printf("(%ju frags, %ju blocks, %.1f%% fragmentation)\n",
        (uintmax_t)n_ffree, (uintmax_t)n_bfree,
        n_ffree * 100.0 / check_sblk.b_un.b_fs->fs_dsize);
    if (check_debug) {
        if (files < 0)
            printf("%jd inodes missing\n", -files);
        if (blks < 0)
            printf("%jd blocks missing\n", -blks);
        if (check_duplist != NULL) {
            printf("The following duplicate blocks remain:");
            for (dp = check_duplist; dp; dp = dp->next)
                printf(" %jd,", (intmax_t)dp->dup);
            printf("\n");
        }
    }
    check_duplist = (struct dups *)0;
    check_muldup = (struct dups *)0;
    check_inocleanup();
    if (check_fsmodified) {
        check_sblk.b_un.b_fs->fs_time = time(NULL);
        dirty(&check_sblk);
    }
    if (check_cvtlevel && check_sblk.b_dirty) {
        /*
         * Write out the duplicate super blocks
         */
        for (cylno = 0; cylno < check_sblk.b_un.b_fs->fs_ncg; cylno++)
            check_blwrite(check_fswritefd, (char *)check_sblk.b_un.b_fs,
                fsbtodb(check_sblk.b_un.b_fs, cgsblock(check_sblk.b_un.b_fs, cylno)),
                SBLOCKSIZE);
    }
    if (check_rerun)
        check_resolved = 0;

    /*
     * Check to see if the file system is mounted read-write.
     */
    check_finish(check_resolved);

    for (cylno = 0; cylno < check_sblk.b_un.b_fs->fs_ncg; cylno++)
        if (check_inostathead[cylno].il_stat != NULL)
            free((char *)check_inostathead[cylno].il_stat);
    free((char *)check_inostathead);
    check_inostathead = NULL;
    if (check_fsmodified && !check_preen)
        printf("\n***** FILE SYSTEM WAS MODIFIED *****\n");
    if (check_rerun)
        printf("\n***** PLEASE RERUN FSCK *****\n");
    return (0);
}

static void
usage(void)
{
    fprintf(stderr,
        "usage: fsck [-BEFfnpry] [-b block] [-c level] [-m mode] filesystem ...\n");
    exit(1);
}
