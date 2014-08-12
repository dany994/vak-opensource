/*
 * Copyright (c) 1980, 1986, 1993
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
 */

#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/uio.h>
#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <fstab.h>
#include <grp.h>
#include <paths.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "dinode.h"
#include "fs.h"

#include "fsck.h"

static void usage(void);
static int argtoi(int flag, const char *req, const char *str, int base);
static int checkfilesys(char *filesys);

int
main(int argc, char *argv[])
{
	int ch;
	struct rlimit rlimit;
	struct itimerval itimerval;
	int ret = 0;

	sync();
	skipclean = 1;
	inoopt = 0;
	while ((ch = getopt(argc, argv, "b:c:CdEfm:nprSyZ")) != -1) {
		switch (ch) {
		case 'b':
			skipclean = 0;
			bflag = argtoi('b', "number", optarg, 10);
			printf("Alternate super block location: %d\n", bflag);
			break;

		case 'c':
			skipclean = 0;
			cvtlevel = argtoi('c', "conversion level", optarg, 10);
			if (cvtlevel < 3)
				errx(EEXIT, "cannot do level %d conversion",
				    cvtlevel);
			break;

		case 'd':
			debug++;
			break;

		case 'E':
			Eflag++;
			break;

		case 'f':
			skipclean = 0;
			break;

		case 'm':
			lfmode = argtoi('m', "mode", optarg, 8);
			if (lfmode &~ 07777)
				errx(EEXIT, "bad mode to -m: %o", lfmode);
			printf("** lost+found creation mode %o\n", lfmode);
			break;

		case 'n':
			nflag++;
			yflag = 0;
			break;

		case 'p':
			preen++;
			/*FALLTHROUGH*/

		case 'C':
			ckclean++;
			break;

		case 'r':
			inoopt++;
			break;

		case 'S':
			surrender = 1;
			break;

		case 'y':
			yflag++;
			nflag = 0;
			break;

		case 'Z':
			Zflag++;
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
		(void)signal(SIGINT, catch);
	if (ckclean)
		(void)signal(SIGQUIT, catchquit);

	/*
	 * Push up our allowed memory limit so we can cope
	 * with huge file systems.
	 */
	if (getrlimit(RLIMIT_DATA, &rlimit) == 0) {
		rlimit.rlim_cur = rlimit.rlim_max;
		(void)setrlimit(RLIMIT_DATA, &rlimit);
	}
	while (argc-- > 0)
		(void)checkfilesys(*argv++);

	if (returntosingle)
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
	struct stat snapdir;
	struct group *grp;
	struct iovec *iov;
	char errmsg[255];
	int iovlen;
	int cylno;
	intmax_t blks, files;
	size_t size;

	iov = NULL;
	iovlen = 0;
	errmsg[0] = '\0';

	cdevname = filesys;
	if (debug && ckclean)
		pwarn("starting\n");
	/*
	 * Make best effort to get the disk name. Check first to see
	 * if it is listed among the mounted file systems. Failing that
	 * check to see if it is listed in /etc/fstab.
	 */
	filesys = blockcheck(filesys);

	sblock_init();
	if (ckclean && skipclean) {
		/*
		 * If file system is gjournaled, check it here.
		 */
		if ((fsreadfd = open(filesys, O_RDONLY)) < 0 || readsb(0) == 0)
			exit(3);	/* Cannot read superblock */
		close(fsreadfd);
		if ((sblock.fs_flags & FS_GJOURNAL) != 0) {
			//printf("GJournaled file system detected on %s.\n",
			//    filesys);
			if (sblock.fs_clean == 1) {
				pwarn("FILE SYSTEM CLEAN; SKIPPING CHECKS\n");
				exit(0);
			}
			if ((sblock.fs_flags & (FS_UNCLEAN | FS_NEEDSFSCK)) == 0) {
				gjournal_check(filesys);
				exit(0);
			} else {
				pfatal(
			    "UNEXPECTED INCONSISTENCY, CANNOT RUN FAST FSCK\n");
			}
		}
	}

	switch (setup(filesys)) {
	case 0:
		if (preen)
			pfatal("CAN'T CHECK FILE SYSTEM.");
		return (0);
	case -1:
	clean:
		pwarn("clean, %ld free ", (long)(sblock.fs_cstotal.cs_nffree +
		    sblock.fs_frag * sblock.fs_cstotal.cs_nbfree));
		printf("(%jd frags, %jd blocks, %.1f%% fragmentation)\n",
		    (intmax_t)sblock.fs_cstotal.cs_nffree,
		    (intmax_t)sblock.fs_cstotal.cs_nbfree,
		    sblock.fs_cstotal.cs_nffree * 100.0 / sblock.fs_dsize);
		return (0);
	}
	/*
	 * Determine if we can and should do journal recovery.
	 */
	if ((sblock.fs_flags & FS_SUJ) == FS_SUJ) {
		if ((sblock.fs_flags & FS_NEEDSFSCK) != FS_NEEDSFSCK && skipclean) {
			if (preen || reply("USE JOURNAL")) {
				if (suj_check(filesys) == 0) {
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
		sblock.fs_mtime = time(NULL);
		sbdirty();
	}

	/*
	 * Cleared if any questions answered no. Used to decide if
	 * the superblock should be marked clean.
	 */
	resolved = 1;
	/*
	 * 1: scan inodes tallying blocks used
	 */
	if (preen == 0) {
		printf("** Last Mounted on %s\n", sblock.fs_fsmnt);
		printf("** Phase 1 - Check Blocks and Sizes\n");
	}
	pass1();
	IOstats("Pass1");

	/*
	 * 1b: locate first references to duplicates, if any
	 */
	if (duplist) {
		if (preen || usedsoftdep)
			pfatal("INTERNAL ERROR: dups with %s%s%s",
			    preen ? "-p" : "",
			    (preen && usedsoftdep) ? " and " : "",
			    usedsoftdep ? "softupdates" : "");
		printf("** Phase 1b - Rescan For More DUPS\n");
		pass1b();
		IOstats("Pass1b");
	}

	/*
	 * 2: traverse directories from root to mark all connected directories
	 */
	if (preen == 0)
		printf("** Phase 2 - Check Pathnames\n");
	pass2();
	IOstats("Pass2");

	/*
	 * 3: scan inodes looking for disconnected directories
	 */
	if (preen == 0)
		printf("** Phase 3 - Check Connectivity\n");
	pass3();
	IOstats("Pass3");

	/*
	 * 4: scan inodes looking for disconnected files; check reference counts
	 */
	if (preen == 0)
		printf("** Phase 4 - Check Reference Counts\n");
	pass4();
	IOstats("Pass4");

	/*
	 * 5: check and repair resource counts in cylinder groups
	 */
	if (preen == 0)
		printf("** Phase 5 - Check Cyl groups\n");
	pass5();
	IOstats("Pass5");

	/*
	 * print out summary statistics
	 */
	n_ffree = sblock.fs_cstotal.cs_nffree;
	n_bfree = sblock.fs_cstotal.cs_nbfree;
	files = maxino - ROOTINO - sblock.fs_cstotal.cs_nifree - n_files;
	blks = n_blks +
	    sblock.fs_ncg * (cgdmin(&sblock, 0) - cgsblock(&sblock, 0));
	blks += cgsblock(&sblock, 0) - cgbase(&sblock, 0);
	blks += howmany(sblock.fs_cssize, sblock.fs_fsize);
	blks = maxfsblock - (n_ffree + sblock.fs_frag * n_bfree) - blks;
	pwarn("%ld files, %jd used, %ju free ",
	    (long)n_files, (intmax_t)n_blks,
	    (uintmax_t)(n_ffree + sblock.fs_frag * n_bfree));
	printf("(%ju frags, %ju blocks, %.1f%% fragmentation)\n",
	    (uintmax_t)n_ffree, (uintmax_t)n_bfree,
	    n_ffree * 100.0 / sblock.fs_dsize);
	if (debug) {
		if (files < 0)
			printf("%jd inodes missing\n", -files);
		if (blks < 0)
			printf("%jd blocks missing\n", -blks);
		if (duplist != NULL) {
			printf("The following duplicate blocks remain:");
			for (dp = duplist; dp; dp = dp->next)
				printf(" %jd,", (intmax_t)dp->dup);
			printf("\n");
		}
	}
	duplist = (struct dups *)0;
	muldup = (struct dups *)0;
	inocleanup();
	if (fsmodified) {
		sblock.fs_time = time(NULL);
		sbdirty();
	}
	if (cvtlevel && sblk.b_dirty) {
		/*
		 * Write out the duplicate super blocks
		 */
		for (cylno = 0; cylno < sblock.fs_ncg; cylno++)
			blwrite(fswritefd, (char *)&sblock,
			    fsbtodb(&sblock, cgsblock(&sblock, cylno)),
			    SBLOCKSIZE);
	}
	if (rerun)
		resolved = 0;
	finalIOstats();

	/*
	 * Check to see if the file system is mounted read-write.
	 */
	ckfini(resolved);

	for (cylno = 0; cylno < sblock.fs_ncg; cylno++)
		if (inostathead[cylno].il_stat != NULL)
			free((char *)inostathead[cylno].il_stat);
	free((char *)inostathead);
	inostathead = NULL;
	if (fsmodified && !preen)
		printf("\n***** FILE SYSTEM WAS MODIFIED *****\n");
	if (rerun)
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
