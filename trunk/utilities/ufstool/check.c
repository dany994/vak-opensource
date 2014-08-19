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

#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <time.h>

#include "dir.h"
#include "fs.h"
#include "fsck.h"

struct bufarea asblk;
#define altsblock (*asblk.b_un.b_fs)
#define POWEROF2(num)	(((num) & ((num) - 1)) == 0)

static void badsb(int listerr, const char *s);
static int calcsb(int part_num, int devfd, struct fs *fs);

static void slowio_start(void);
static void slowio_end(void);
static void printIOstats(void);

static long diskreads, totaldiskreads, totalreads; /* Disk cache statistics */
struct timeval slowio_starttime;
int slowio_delay_usec = 10000;	/* Initial IO delay for background fsck */
int slowio_pollcnt;
static struct bufarea cgblk;	/* backup buffer for cylinder group blocks */
static TAILQ_HEAD(buflist, bufarea) bufhead;	/* head of buffer cache list */
static int numbufs;				/* size of buffer cache */
static char *buftype[BT_NUMBUFTYPES] = BT_NAMES;

static ino_t startinum;

static int iblock(struct inodesc *, long ilevel, off_t isize, int type);

const char	*lfname = "lost+found";
int	lfmode = 0700;
struct	dirtemplate emptydir = {
	0, DIRBLKSIZ, DT_UNKNOWN, 0, "",
	0, 0, DT_UNKNOWN, 0, ""
};
struct	dirtemplate dirhead = {
	0, 12, DT_DIR, 1, ".",
	0, DIRBLKSIZ - 12, DT_DIR, 2, ".."
};

static int chgino(struct inodesc *);
static int dircheck(struct inodesc *, struct direct *);
static int expanddir(union dinode *dp, char *name);
static void freedir(ino_t ino, ino_t parent);
static struct direct *fsck_readdir(struct inodesc *);
static struct bufarea *getdirblk(ufs2_daddr_t blkno, long size);
static int lftempname(char *bufp, ino_t ino);
static int mkentry(struct inodesc *);

/*
 * Read in a superblock finding an alternate if necessary.
 * Return 1 if successful, 0 if unsuccessful, -1 if file system
 * is already clean (ckclean and preen mode only).
 */
int
setup(char *dev, int part_num)
{
	long cg, asked, i, j;
	long bmapsize;
	struct stat statb;
	struct fs proto;
	size_t size;

	havesb = 0;
	fswritefd = -1;
	if (stat(dev, &statb) < 0) {
		printf("Can't stat %s: %s\n", dev, strerror(errno));
		return (0);
	}
	if ((statb.st_mode & S_IFMT) != S_IFREG) {
		pfatal("%s is not a regular file", dev);
		if (reply("CONTINUE") == 0) {
			return (0);
                }
	}
	if ((fsreadfd = open(dev, O_RDONLY)) < 0) {
		printf("Can't open %s: %s\n", dev, strerror(errno));
		return (0);
	}
	if (preen == 0)
		printf("** %s", dev);
	if (nflag || (fswritefd = open(dev, O_WRONLY)) < 0) {
		fswritefd = -1;
		if (preen)
			pfatal("NO WRITE ACCESS");
		printf(" (NO WRITE)");
	}
	if (preen == 0)
		printf("\n");
	/*
	 * Read in the superblock, looking for alternates if necessary
	 */
	if (readsb(1) == 0) {
		skipclean = 0;
		if (bflag || preen || calcsb(part_num, fsreadfd, &proto) == 0)
			return(0);
		if (reply("LOOK FOR ALTERNATE SUPERBLOCKS") == 0)
			return (0);
		for (cg = 0; cg < proto.fs_ncg; cg++) {
			bflag = fsbtodb(&proto, cgsblock(&proto, cg));
			if (readsb(0) != 0)
				break;
		}
		if (cg >= proto.fs_ncg) {
			printf("%s %s\n%s %s\n%s %s\n",
				"SEARCH FOR ALTERNATE SUPER-BLOCK",
				"FAILED. YOU MUST USE THE",
				"-b OPTION TO FSCK TO SPECIFY THE",
				"LOCATION OF AN ALTERNATE",
				"SUPER-BLOCK TO SUPPLY NEEDED",
				"INFORMATION; SEE fsck_ffs(8).");
			bflag = 0;
			return(0);
		}
		pwarn("USING ALTERNATE SUPERBLOCK AT %d\n", bflag);
		bflag = 0;
	}
	if (skipclean && ckclean && sblock.fs_clean) {
		pwarn("FILE SYSTEM CLEAN; SKIPPING CHECKS\n");
		return (-1);
	}
	maxfsblock = sblock.fs_size;
	maxino = sblock.fs_ncg * sblock.fs_ipg;
	/*
	 * Check and potentially fix certain fields in the super block.
	 */
	if (sblock.fs_optim != FS_OPTTIME && sblock.fs_optim != FS_OPTSPACE) {
		pfatal("UNDEFINED OPTIMIZATION IN SUPERBLOCK");
		if (reply("SET TO DEFAULT") == 1) {
			sblock.fs_optim = FS_OPTTIME;
			sbdirty();
		}
	}
	if ((sblock.fs_minfree < 0 || sblock.fs_minfree > 99)) {
		pfatal("IMPOSSIBLE MINFREE=%d IN SUPERBLOCK",
			sblock.fs_minfree);
		if (reply("SET TO DEFAULT") == 1) {
			sblock.fs_minfree = 10;
			sbdirty();
		}
	}
	if (sblock.fs_magic == FS_UFS1_MAGIC &&
	    sblock.fs_old_inodefmt < FS_44INODEFMT) {
		pwarn("Format of file system is too old.\n");
		pwarn("Must update to modern format using a version of fsck\n");
		pfatal("from before 2002 with the command ``fsck -c 2''\n");
		exit(EEXIT);
	}
	if (asblk.b_dirty && !bflag) {
		memmove(&altsblock, &sblock, (size_t)sblock.fs_sbsize);
		flush(fswritefd, &asblk);
	}
	/*
	 * read in the summary info.
	 */
	asked = 0;
	sblock.fs_csp = Calloc(1, sblock.fs_cssize);
	if (sblock.fs_csp == NULL) {
		printf("cannot alloc %u bytes for cg summary info\n",
		    (unsigned)sblock.fs_cssize);
		goto badsb;
	}
	for (i = 0, j = 0; i < sblock.fs_cssize; i += sblock.fs_bsize, j++) {
		size = sblock.fs_cssize - i < sblock.fs_bsize ?
		    sblock.fs_cssize - i : sblock.fs_bsize;
		readcnt[sblk.b_type]++;
		if (blread(fsreadfd, (char *)sblock.fs_csp + i,
		    fsbtodb(&sblock, sblock.fs_csaddr + j * sblock.fs_frag),
		    size) != 0 && !asked) {
			pfatal("BAD SUMMARY INFORMATION");
			if (reply("CONTINUE") == 0) {
				ckfini(0);
				exit(EEXIT);
			}
			asked++;
		}
	}
	/*
	 * allocate and initialize the necessary maps
	 */
	bmapsize = roundup(howmany(maxfsblock, CHAR_BIT), sizeof(short));
	blockmap = Calloc((unsigned)bmapsize, sizeof (char));
	if (blockmap == NULL) {
		printf("cannot alloc %u bytes for blockmap\n",
		    (unsigned)bmapsize);
		goto badsb;
	}
	inostathead = Calloc((unsigned)(sblock.fs_ncg),
	    sizeof(struct inostatlist));
	if (inostathead == NULL) {
		printf("cannot alloc %u bytes for inostathead\n",
		    (unsigned)(sizeof(struct inostatlist) * (sblock.fs_ncg)));
		goto badsb;
	}
	numdirs = MAX(sblock.fs_cstotal.cs_ndir, 128);
	dirhash = numdirs;
	inplast = 0;
	listmax = numdirs + 10;
	inpsort = (struct inoinfo **)Calloc((unsigned)listmax,
	    sizeof(struct inoinfo *));
	inphead = (struct inoinfo **)Calloc((unsigned)numdirs,
	    sizeof(struct inoinfo *));
	if (inpsort == NULL || inphead == NULL) {
		printf("cannot alloc %ju bytes for inphead\n",
		    (uintmax_t)numdirs * sizeof(struct inoinfo *));
		goto badsb;
	}
	bufinit();
	if (sblock.fs_flags & FS_DOSOFTDEP)
		usedsoftdep = 1;
	else
		usedsoftdep = 0;
	return (1);

badsb:
	ckfini(0);
	return (0);
}

/*
 * Possible superblock locations ordered from most to least likely.
 */
static int sblock_try[] = SBLOCKSEARCH;

#define BAD_MAGIC_MSG \
"The previous newfs operation on this volume did not complete.\n" \
"You must complete newfs before mounting this volume.\n"

/*
 * Read in the super block and its summary info.
 */
int
readsb(int listerr)
{
	ufs2_daddr_t super;
	int i;

	if (bflag) {
		super = bflag;
		readcnt[sblk.b_type]++;
		if ((blread(fsreadfd, (char *)&sblock, super, (long)SBLOCKSIZE)))
			return (0);
		if (sblock.fs_magic == FS_BAD_MAGIC) {
			fprintf(stderr, BAD_MAGIC_MSG);
			exit(11);
		}
		if (sblock.fs_magic != FS_UFS1_MAGIC &&
		    sblock.fs_magic != FS_UFS2_MAGIC) {
			fprintf(stderr, "%d is not a file system superblock\n",
			    bflag);
			return (0);
		}
	} else {
		for (i = 0; sblock_try[i] != -1; i++) {
			super = sblock_try[i] / dev_bsize;
			readcnt[sblk.b_type]++;
			if ((blread(fsreadfd, (char *)&sblock, super,
			    (long)SBLOCKSIZE)))
				return (0);
			if (sblock.fs_magic == FS_BAD_MAGIC) {
				fprintf(stderr, BAD_MAGIC_MSG);
				exit(11);
			}
			if ((sblock.fs_magic == FS_UFS1_MAGIC ||
			     (sblock.fs_magic == FS_UFS2_MAGIC &&
			      sblock.fs_sblockloc == sblock_try[i])) &&
			    sblock.fs_ncg >= 1 &&
			    sblock.fs_bsize >= MINBSIZE &&
			    sblock.fs_sbsize >= roundup(sizeof(struct fs), dev_bsize))
				break;
		}
		if (sblock_try[i] == -1) {
			fprintf(stderr, "Cannot find file system superblock\n");
			return (0);
		}
	}
	/*
	 * Compute block size that the file system is based on,
	 * according to fsbtodb, and adjust superblock block number
	 * so we can tell if this is an alternate later.
	 */
	super *= dev_bsize;
	dev_bsize = sblock.fs_fsize / fsbtodb(&sblock, 1);
	sblk.b_bno = super / dev_bsize;
	sblk.b_size = SBLOCKSIZE;
	if (bflag)
		goto out;
	/*
	 * Compare all fields that should not differ in alternate super block.
	 * When an alternate super-block is specified this check is skipped.
	 */
	getblk(&asblk, cgsblock(&sblock, sblock.fs_ncg - 1), sblock.fs_sbsize);
	if (asblk.b_errs)
		return (0);
	if (altsblock.fs_sblkno != sblock.fs_sblkno ||
	    altsblock.fs_cblkno != sblock.fs_cblkno ||
	    altsblock.fs_iblkno != sblock.fs_iblkno ||
	    altsblock.fs_dblkno != sblock.fs_dblkno ||
	    altsblock.fs_ncg != sblock.fs_ncg ||
	    altsblock.fs_bsize != sblock.fs_bsize ||
	    altsblock.fs_fsize != sblock.fs_fsize ||
	    altsblock.fs_frag != sblock.fs_frag ||
	    altsblock.fs_bmask != sblock.fs_bmask ||
	    altsblock.fs_fmask != sblock.fs_fmask ||
	    altsblock.fs_bshift != sblock.fs_bshift ||
	    altsblock.fs_fshift != sblock.fs_fshift ||
	    altsblock.fs_fragshift != sblock.fs_fragshift ||
	    altsblock.fs_fsbtodb != sblock.fs_fsbtodb ||
	    altsblock.fs_sbsize != sblock.fs_sbsize ||
	    altsblock.fs_nindir != sblock.fs_nindir ||
	    altsblock.fs_inopb != sblock.fs_inopb ||
	    altsblock.fs_cssize != sblock.fs_cssize ||
	    altsblock.fs_ipg != sblock.fs_ipg ||
	    altsblock.fs_fpg != sblock.fs_fpg ||
	    altsblock.fs_magic != sblock.fs_magic) {
		badsb(listerr,
		"VALUES IN SUPER BLOCK DISAGREE WITH THOSE IN FIRST ALTERNATE");
		return (0);
	}
out:
	/*
	 * If not yet done, update UFS1 superblock with new wider fields.
	 */
	if (sblock.fs_magic == FS_UFS1_MAGIC &&
	    sblock.fs_maxbsize != sblock.fs_bsize) {
		sblock.fs_maxbsize = sblock.fs_bsize;
		sblock.fs_time = sblock.fs_old_time;
		sblock.fs_size = sblock.fs_old_size;
		sblock.fs_dsize = sblock.fs_old_dsize;
		sblock.fs_csaddr = sblock.fs_old_csaddr;
		sblock.fs_cstotal.cs_ndir = sblock.fs_old_cstotal.cs_ndir;
		sblock.fs_cstotal.cs_nbfree = sblock.fs_old_cstotal.cs_nbfree;
		sblock.fs_cstotal.cs_nifree = sblock.fs_old_cstotal.cs_nifree;
		sblock.fs_cstotal.cs_nffree = sblock.fs_old_cstotal.cs_nffree;
	}
	havesb = 1;
	return (1);
}

static void
badsb(int listerr, const char *s)
{

	if (!listerr)
		return;
	if (preen)
		printf("%s: ", cdevname);
	pfatal("BAD SUPER BLOCK: %s\n", s);
}

void
sblock_init(void)
{
	fswritefd = -1;
	fsmodified = 0;
	lfdir = 0;
	initbarea(&sblk, BT_SUPERBLK);
	initbarea(&asblk, BT_SUPERBLK);
	sblk.b_un.b_buf = Malloc(SBLOCKSIZE);
	asblk.b_un.b_buf = Malloc(SBLOCKSIZE);
	if (sblk.b_un.b_buf == NULL || asblk.b_un.b_buf == NULL)
		errx(EEXIT, "cannot allocate space for superblock");
	dev_bsize = secsize = DEV_BSIZE;
}

/*
 * Calculate a prototype superblock based on information in the disk label.
 * When done the cgsblock macro can be calculated and the fs_ncg field
 * can be used. Do NOT attempt to use other macros without verifying that
 * their needed information is available!
 */
static int
calcsb(int part_num, int devfd, struct fs *fs)
{
	int i, nspf;
	struct stat statb;
        unsigned disk_nsectors, disk_ntracks, disk_secpercyl;
        unsigned part_size, part_fsize, part_frag, part_cpg;

        /* Get disk parameters. */
	fstat(devfd, &statb);
        disk_nsectors = statb.st_size / DEV_BSIZE;
        disk_ntracks = 1;
        disk_secpercyl = 2048;  /* 1 Mbyte per cylinder */

	if (part_num == 0) {
	        /* Use whole disk. */
                part_size = disk_nsectors;
	} else {
	        /* Get a partition. */
                part_size = 0; // TODO
        }
        part_fsize = DEV_BSIZE;
        part_frag = 1;
        part_cpg = 204800;  /* 100 Mbytes per group */

	memset(fs, 0, sizeof(struct fs));
	fs->fs_fsize = part_fsize;
	fs->fs_frag = part_frag;
	fs->fs_size = part_size;
	fs->fs_sblkno = 0;

	nspf = fs->fs_fsize / DEV_BSIZE;
	for (fs->fs_fsbtodb = 0, i = nspf; i > 1; i >>= 1)
		fs->fs_fsbtodb++;

	dev_bsize = DEV_BSIZE;
	if (fs->fs_magic == FS_UFS2_MAGIC) {
	        /* Never used? */
		fs->fs_fpg = part_cpg;
		fs->fs_ncg = howmany(fs->fs_size, fs->fs_fpg);
	} else /* if (fs->fs_magic == FS_UFS1_MAGIC) */ {
		fs->fs_old_cpg = part_cpg;
		fs->fs_old_cgmask = 0xffffffff;
		for (i = disk_ntracks; i > 1; i >>= 1)
			fs->fs_old_cgmask <<= 1;
		if (!POWEROF2(disk_ntracks))
			fs->fs_old_cgmask <<= 1;
		fs->fs_old_cgoffset = roundup(howmany(disk_nsectors, nspf),
		    fs->fs_frag);
		fs->fs_fpg = (fs->fs_old_cpg * disk_secpercyl) / nspf;
		fs->fs_ncg = howmany(fs->fs_size / disk_secpercyl,
		    fs->fs_old_cpg);
	}
	return (1);
}

int
ftypeok(union dinode *dp)
{
	switch (DIP(dp, di_mode) & IFMT) {

	case IFDIR:
	case IFREG:
	case IFBLK:
	case IFCHR:
	case IFLNK:
	case IFSOCK:
	case IFIFO:
		return (1);

	default:
		if (debug)
			printf("bad file type 0%o\n", DIP(dp, di_mode));
		return (0);
	}
}

int
reply(const char *question)
{
	int persevere;
	char c;

	if (preen)
		pfatal("INTERNAL ERROR: GOT TO reply()");
	persevere = !strcmp(question, "CONTINUE");
	printf("\n");
	if (!persevere && (nflag || fswritefd < 0)) {
		printf("%s? no\n\n", question);
		resolved = 0;
		return (0);
	}
	if (yflag || (persevere && nflag)) {
		printf("%s? yes\n\n", question);
		return (1);
	}
	do	{
		printf("%s? [yn] ", question);
		(void) fflush(stdout);
		c = getc(stdin);
		while (c != '\n' && getc(stdin) != '\n') {
			if (feof(stdin)) {
				resolved = 0;
				return (0);
			}
		}
	} while (c != 'y' && c != 'Y' && c != 'n' && c != 'N');
	printf("\n");
	if (c == 'y' || c == 'Y')
		return (1);
	resolved = 0;
	return (0);
}

/*
 * Look up state information for an inode.
 */
struct inostat *
inoinfo(ino_t inum)
{
	static struct inostat unallocated = { USTATE, 0, 0 };
	struct inostatlist *ilp;
	int iloff;

	if (inum > maxino)
		errx(EEXIT, "inoinfo: inumber %ju out of range",
		    (uintmax_t)inum);
	ilp = &inostathead[inum / sblock.fs_ipg];
	iloff = inum % sblock.fs_ipg;
	if (iloff >= ilp->il_numalloced)
		return (&unallocated);
	return (&ilp->il_stat[iloff]);
}

/*
 * Malloc buffers and set up cache.
 */
void
bufinit(void)
{
	struct bufarea *bp;
	long bufcnt, i;
	char *bufp;

	pbp = pdirbp = (struct bufarea *)0;
	bufp = Malloc((unsigned int)sblock.fs_bsize);
	if (bufp == 0)
		errx(EEXIT, "cannot allocate buffer pool");
	cgblk.b_un.b_buf = bufp;
	initbarea(&cgblk, BT_CYLGRP);
	TAILQ_INIT(&bufhead);
	bufcnt = MAXBUFS;
	if (bufcnt < MINBUFS)
		bufcnt = MINBUFS;
	for (i = 0; i < bufcnt; i++) {
		bp = (struct bufarea *)Malloc(sizeof(struct bufarea));
		bufp = Malloc((unsigned int)sblock.fs_bsize);
		if (bp == NULL || bufp == NULL) {
			if (i >= MINBUFS)
				break;
			errx(EEXIT, "cannot allocate buffer pool");
		}
		bp->b_un.b_buf = bufp;
		TAILQ_INSERT_HEAD(&bufhead, bp, b_list);
		initbarea(bp, BT_UNKNOWN);
	}
	numbufs = i;	/* save number of buffers */
	for (i = 0; i < BT_NUMBUFTYPES; i++) {
		readtime[i].tv_sec = totalreadtime[i].tv_sec = 0;
		readtime[i].tv_nsec = totalreadtime[i].tv_nsec = 0;
		readcnt[i] = totalreadcnt[i] = 0;
	}
}

/*
 * Manage cylinder group buffers.
 */
static struct bufarea *cgbufs;	/* header for cylinder group cache */
static int flushtries;		/* number of tries to reclaim memory */

struct bufarea *
cgget(int cg)
{
	struct bufarea *cgbp;
	struct cg *cgp;

	if (cgbufs == NULL) {
		cgbufs = Calloc(sblock.fs_ncg, sizeof(struct bufarea));
		if (cgbufs == NULL)
			errx(EEXIT, "cannot allocate cylinder group buffers");
	}
	cgbp = &cgbufs[cg];
	if (cgbp->b_un.b_cg != NULL)
		return (cgbp);
	cgp = NULL;
	if (flushtries == 0)
		cgp = malloc((unsigned int)sblock.fs_cgsize);
	if (cgp == NULL) {
		getblk(&cgblk, cgtod(&sblock, cg), sblock.fs_cgsize);
		return (&cgblk);
	}
	cgbp->b_un.b_cg = cgp;
	initbarea(cgbp, BT_CYLGRP);
	getblk(cgbp, cgtod(&sblock, cg), sblock.fs_cgsize);
	return (cgbp);
}

/*
 * Attempt to flush a cylinder group cache entry.
 * Return whether the flush was successful.
 */
int
flushentry(void)
{
	struct bufarea *cgbp;

	cgbp = &cgbufs[flushtries++];
	if (cgbp->b_un.b_cg == NULL)
		return (0);
	flush(fswritefd, cgbp);
	free(cgbp->b_un.b_buf);
	cgbp->b_un.b_buf = NULL;
	return (1);
}

/*
 * Manage a cache of directory blocks.
 */
struct bufarea *
getdatablk(ufs2_daddr_t blkno, long size, int type)
{
	struct bufarea *bp;

	TAILQ_FOREACH(bp, &bufhead, b_list)
		if (bp->b_bno == fsbtodb(&sblock, blkno))
			goto foundit;
	TAILQ_FOREACH_REVERSE(bp, &bufhead, buflist, b_list)
		if ((bp->b_flags & B_INUSE) == 0)
			break;
	if (bp == NULL)
		errx(EEXIT, "deadlocked buffer pool");
	bp->b_type = type;
	getblk(bp, blkno, size);
	/* fall through */
foundit:
	if (debug && bp->b_type != type)
		printf("Buffer type changed from %s to %s\n",
		    buftype[bp->b_type], buftype[type]);
	TAILQ_REMOVE(&bufhead, bp, b_list);
	TAILQ_INSERT_HEAD(&bufhead, bp, b_list);
	bp->b_flags |= B_INUSE;
	return (bp);
}

/*
 * Timespec operations (from <sys/time.h>).
 */
#define	timespecsub(vvp, uvp)						\
	do {								\
		(vvp)->tv_sec -= (uvp)->tv_sec;				\
		(vvp)->tv_nsec -= (uvp)->tv_nsec;			\
		if ((vvp)->tv_nsec < 0) {				\
			(vvp)->tv_sec--;				\
			(vvp)->tv_nsec += 1000000000;			\
		}							\
	} while (0)
#define	timespecadd(vvp, uvp)						\
	do {								\
		(vvp)->tv_sec += (uvp)->tv_sec;				\
		(vvp)->tv_nsec += (uvp)->tv_nsec;			\
		if ((vvp)->tv_nsec >= 1000000000) {			\
			(vvp)->tv_sec++;				\
			(vvp)->tv_nsec -= 1000000000;			\
		}							\
	} while (0)

void
getblk(struct bufarea *bp, ufs2_daddr_t blk, long size)
{
	ufs2_daddr_t dblk;

	dblk = fsbtodb(&sblock, blk);
	if (bp->b_bno == dblk) {
		totalreads++;
	} else {
		flush(fswritefd, bp);
		bp->b_errs = blread(fsreadfd, bp->b_un.b_buf, dblk, size);
		bp->b_bno = dblk;
		bp->b_size = size;
	}
}

void
flush(int fd, struct bufarea *bp)
{
	int i, j;

	if (!bp->b_dirty)
		return;
	bp->b_dirty = 0;
	if (fswritefd < 0) {
		pfatal("WRITING IN READ_ONLY MODE.\n");
		return;
	}
	if (bp->b_errs != 0)
		pfatal("WRITING %sZERO'ED BLOCK %lld TO DISK\n",
		    (bp->b_errs == bp->b_size / dev_bsize) ? "" : "PARTIALLY ",
		    (long long)bp->b_bno);
	bp->b_errs = 0;
	blwrite(fd, bp->b_un.b_buf, bp->b_bno, bp->b_size);
	if (bp != &sblk)
		return;
	for (i = 0, j = 0; i < sblock.fs_cssize; i += sblock.fs_bsize, j++) {
		blwrite(fswritefd, (char *)sblock.fs_csp + i,
		    fsbtodb(&sblock, sblock.fs_csaddr + j * sblock.fs_frag),
		    sblock.fs_cssize - i < sblock.fs_bsize ?
		    sblock.fs_cssize - i : sblock.fs_bsize);
	}
}

void
rwerror(const char *mesg, ufs2_daddr_t blk)
{
	if (preen == 0)
		printf("\n");
	pfatal("CANNOT %s: %ld", mesg, (long)blk);
	if (reply("CONTINUE") == 0)
		exit(EEXIT);
}

void
ckfini(int markclean)
{
	struct bufarea *bp, *nbp;
	int ofsmodified, cnt;

	if (debug && totalreads > 0)
		printf("cache with %d buffers missed %ld of %ld (%d%%)\n",
		    numbufs, totaldiskreads, totalreads,
		    (int)(totaldiskreads * 100 / totalreads));
	if (fswritefd < 0) {
		(void)close(fsreadfd);
		return;
	}
	flush(fswritefd, &sblk);
	if (havesb && sblock.fs_magic == FS_UFS2_MAGIC &&
	    sblk.b_bno != sblock.fs_sblockloc / dev_bsize &&
	    !preen && reply("UPDATE STANDARD SUPERBLOCK")) {
		sblk.b_bno = sblock.fs_sblockloc / dev_bsize;
		sbdirty();
		flush(fswritefd, &sblk);
	}
	flush(fswritefd, &cgblk);
	free(cgblk.b_un.b_buf);
	cnt = 0;
	TAILQ_FOREACH_REVERSE_SAFE(bp, &bufhead, buflist, b_list, nbp) {
		TAILQ_REMOVE(&bufhead, bp, b_list);
		cnt++;
		flush(fswritefd, bp);
		free(bp->b_un.b_buf);
		free((char *)bp);
	}
	if (numbufs != cnt)
		errx(EEXIT, "panic: lost %d buffers", numbufs - cnt);
	for (cnt = 0; cnt < sblock.fs_ncg; cnt++) {
		if (cgbufs[cnt].b_un.b_cg == NULL)
			continue;
		flush(fswritefd, &cgbufs[cnt]);
		free(cgbufs[cnt].b_un.b_cg);
	}
	free(cgbufs);
	pbp = pdirbp = (struct bufarea *)0;
	if (sblock.fs_clean != markclean) {
		if ((sblock.fs_clean = markclean) != 0) {
			sblock.fs_flags &= ~(FS_UNCLEAN | FS_NEEDSFSCK);
			sblock.fs_pendingblocks = 0;
			sblock.fs_pendinginodes = 0;
		}
		sbdirty();
		ofsmodified = fsmodified;
		flush(fswritefd, &sblk);
		fsmodified = ofsmodified;
		if (!preen) {
			printf("\n***** FILE SYSTEM MARKED %s *****\n",
			    markclean ? "CLEAN" : "DIRTY");
			if (!markclean)
				rerun = 1;
		}
	} else if (!preen) {
		if (markclean) {
			printf("\n***** FILE SYSTEM IS CLEAN *****\n");
		} else {
			printf("\n***** FILE SYSTEM STILL DIRTY *****\n");
			rerun = 1;
		}
	}
	(void)close(fsreadfd);
	(void)close(fswritefd);
}

/*
 * Print out I/O statistics.
 */
void
IOstats(char *what)
{
	int i;

	if (debug == 0)
		return;
	if (diskreads == 0) {
		printf("%s: no I/O\n\n", what);
		return;
	}
	printf("%s: I/O statistics\n", what);
	printIOstats();
	totaldiskreads += diskreads;
	diskreads = 0;
	for (i = 0; i < BT_NUMBUFTYPES; i++) {
		timespecadd(&totalreadtime[i], &readtime[i]);
		totalreadcnt[i] += readcnt[i];
		readtime[i].tv_sec = readtime[i].tv_nsec = 0;
		readcnt[i] = 0;
	}
}

void
finalIOstats(void)
{
	int i;

	if (debug == 0)
		return;
	printf("Final I/O statistics\n");
	totaldiskreads += diskreads;
	diskreads = totaldiskreads;
	for (i = 0; i < BT_NUMBUFTYPES; i++) {
		timespecadd(&totalreadtime[i], &readtime[i]);
		totalreadcnt[i] += readcnt[i];
		readtime[i] = totalreadtime[i];
		readcnt[i] = totalreadcnt[i];
	}
	printIOstats();
}

static void printIOstats(void)
{
	long long msec, totalmsec;
	int i;

	printf("buffer reads by type:\n");
	for (totalmsec = 0, i = 0; i < BT_NUMBUFTYPES; i++)
		totalmsec += readtime[i].tv_sec * 1000 +
		    readtime[i].tv_nsec / 1000000;
	if (totalmsec == 0)
		totalmsec = 1;
	for (i = 0; i < BT_NUMBUFTYPES; i++) {
		if (readcnt[i] == 0)
			continue;
		msec =
		    readtime[i].tv_sec * 1000 + readtime[i].tv_nsec / 1000000;
		printf("%21s:%8ld %2ld.%ld%% %4jd.%03ld sec %2lld.%lld%%\n",
		    buftype[i], readcnt[i], readcnt[i] * 100 / diskreads,
		    (readcnt[i] * 1000 / diskreads) % 10,
		    (intmax_t)readtime[i].tv_sec, readtime[i].tv_nsec / 1000000,
		    msec * 100 / totalmsec, (msec * 1000 / totalmsec) % 10);
	}
	printf("\n");
}

int
blread(int fd, char *buf, ufs2_daddr_t blk, long size)
{
	char *cp;
	int i, errs;
	off_t offset;

	offset = blk;
	offset *= dev_bsize;
	totalreads++;
	diskreads++;
	if (lseek(fd, offset, 0) < 0)
		rwerror("SEEK BLK", blk);
	else if (read(fd, buf, (int)size) == size) {
		return (0);
	}

	/*
	 * This is handled specially here instead of in rwerror because
	 * rwerror is used for all sorts of errors, not just true read/write
	 * errors.  It should be refactored and fixed.
	 */
	if (surrender) {
		pfatal("CANNOT READ_BLK: %ld", (long)blk);
		errx(EEXIT, "ABORTING DUE TO READ ERRORS");
	} else
		rwerror("READ BLK", blk);

	if (lseek(fd, offset, 0) < 0)
		rwerror("SEEK BLK", blk);
	errs = 0;
	memset(buf, 0, (size_t)size);
	printf("THE FOLLOWING DISK SECTORS COULD NOT BE READ:");
	for (cp = buf, i = 0; i < size; i += secsize, cp += secsize) {
		if (read(fd, cp, (int)secsize) != secsize) {
			(void)lseek(fd, offset + i + secsize, 0);
			if (secsize != dev_bsize && dev_bsize != 1)
				printf(" %jd (%jd),",
				    (intmax_t)(blk * dev_bsize + i) / secsize,
				    (intmax_t)blk + i / dev_bsize);
			else
				printf(" %jd,", (intmax_t)blk + i / dev_bsize);
			errs++;
		}
	}
	printf("\n");
	if (errs)
		resolved = 0;
	return (errs);
}

void
blwrite(int fd, char *buf, ufs2_daddr_t blk, ssize_t size)
{
	int i;
	char *cp;
	off_t offset;

	if (fd < 0)
		return;
	offset = blk;
	offset *= dev_bsize;
	if (lseek(fd, offset, 0) < 0)
		rwerror("SEEK BLK", blk);
	else if (write(fd, buf, size) == size) {
		fsmodified = 1;
		return;
	}
	resolved = 0;
	rwerror("WRITE BLK", blk);
	if (lseek(fd, offset, 0) < 0)
		rwerror("SEEK BLK", blk);
	printf("THE FOLLOWING SECTORS COULD NOT BE WRITTEN:");
	for (cp = buf, i = 0; i < size; i += dev_bsize, cp += dev_bsize)
		if (write(fd, cp, dev_bsize) != dev_bsize) {
			(void)lseek(fd, offset + i + dev_bsize, 0);
			printf(" %jd,", (intmax_t)blk + i / dev_bsize);
		}
	printf("\n");
	return;
}

void
blerase(int fd, ufs2_daddr_t blk, long size)
{
	off_t ioarg[2];

	if (fd < 0)
		return;
#ifdef DIOCGDELETE
	ioarg[0] = blk * dev_bsize;
	ioarg[1] = size;
	ioctl(fd, DIOCGDELETE, ioarg);
	/* we don't really care if we succeed or not */
#endif
}

/*
 * Fill a contiguous region with all-zeroes.  Note ZEROBUFSIZE is by
 * definition a multiple of dev_bsize.
 */
void
blzero(int fd, ufs2_daddr_t blk, long size)
{
	static char *zero;
	off_t offset, len;

	if (fd < 0)
		return;
	if (zero == NULL) {
		zero = calloc(ZEROBUFSIZE, 1);
		if (zero == NULL)
			errx(EEXIT, "cannot allocate buffer pool");
	}
	offset = blk * dev_bsize;
	if (lseek(fd, offset, 0) < 0)
		rwerror("SEEK BLK", blk);
	while (size > 0) {
		len = size > ZEROBUFSIZE ? ZEROBUFSIZE : size;
		if (write(fd, zero, len) != len)
			rwerror("WRITE BLK", blk);
		blk += len / dev_bsize;
		size -= len;
	}
}

/*
 * Verify cylinder group's magic number and other parameters.  If the
 * test fails, offer an option to rebuild the whole cylinder group.
 */
int
check_cgmagic(int cg, struct bufarea *cgbp)
{
	struct cg *cgp = cgbp->b_un.b_cg;

	/*
	 * Extended cylinder group checks.
	 */
	if (cg_chkmagic(cgp) &&
	    ((sblock.fs_magic == FS_UFS1_MAGIC &&
	      cgp->cg_old_niblk == sblock.fs_ipg &&
	      cgp->cg_ndblk <= sblock.fs_fpg &&
	      cgp->cg_old_ncyl <= sblock.fs_old_cpg) ||
	     (sblock.fs_magic == FS_UFS2_MAGIC &&
	      cgp->cg_niblk == sblock.fs_ipg &&
	      cgp->cg_ndblk <= sblock.fs_fpg &&
	      cgp->cg_initediblk <= sblock.fs_ipg))) {
		return (1);
	}
	pfatal("CYLINDER GROUP %d: BAD MAGIC NUMBER", cg);
	if (!reply("REBUILD CYLINDER GROUP")) {
		printf("YOU WILL NEED TO RERUN FSCK.\n");
		rerun = 1;
		return (1);
	}
	/*
	 * Zero out the cylinder group and then initialize critical fields.
	 * Bit maps and summaries will be recalculated by later passes.
	 */
	memset(cgp, 0, (size_t)sblock.fs_cgsize);
	cgp->cg_magic = CG_MAGIC;
	cgp->cg_cgx = cg;
	cgp->cg_niblk = sblock.fs_ipg;
	cgp->cg_initediblk = sblock.fs_ipg < 2 * INOPB(&sblock) ?
	    sblock.fs_ipg : 2 * INOPB(&sblock);
	if (cgbase(&sblock, cg) + sblock.fs_fpg < sblock.fs_size)
		cgp->cg_ndblk = sblock.fs_fpg;
	else
		cgp->cg_ndblk = sblock.fs_size - cgbase(&sblock, cg);
	cgp->cg_iusedoff = &cgp->cg_space[0] - (u_char *)(&cgp->cg_firstfield);
	if (sblock.fs_magic == FS_UFS1_MAGIC) {
		cgp->cg_niblk = 0;
		cgp->cg_initediblk = 0;
		cgp->cg_old_ncyl = sblock.fs_old_cpg;
		cgp->cg_old_niblk = sblock.fs_ipg;
		cgp->cg_old_btotoff = cgp->cg_iusedoff;
		cgp->cg_old_boff = cgp->cg_old_btotoff +
		    sblock.fs_old_cpg * sizeof(int32_t);
		cgp->cg_iusedoff = cgp->cg_old_boff +
		    sblock.fs_old_cpg * sizeof(u_int16_t);
	}
	cgp->cg_freeoff = cgp->cg_iusedoff + howmany(sblock.fs_ipg, CHAR_BIT);
	cgp->cg_nextfreeoff = cgp->cg_freeoff + howmany(sblock.fs_fpg,CHAR_BIT);
	if (sblock.fs_contigsumsize > 0) {
		cgp->cg_nclusterblks = cgp->cg_ndblk / sblock.fs_frag;
		cgp->cg_clustersumoff =
		    roundup(cgp->cg_nextfreeoff, sizeof(u_int32_t));
		cgp->cg_clustersumoff -= sizeof(u_int32_t);
		cgp->cg_clusteroff = cgp->cg_clustersumoff +
		    (sblock.fs_contigsumsize + 1) * sizeof(u_int32_t);
		cgp->cg_nextfreeoff = cgp->cg_clusteroff +
		    howmany(fragstoblks(&sblock, sblock.fs_fpg), CHAR_BIT);
	}
	dirty(cgbp);
	return (0);
}

/*
 * allocate a data block with the specified number of fragments
 */
ufs2_daddr_t
allocblk(long frags)
{
	int i, j, k, cg, baseblk;
	struct bufarea *cgbp;
	struct cg *cgp;

	if (frags <= 0 || frags > sblock.fs_frag)
		return (0);
	for (i = 0; i < maxfsblock - sblock.fs_frag; i += sblock.fs_frag) {
		for (j = 0; j <= sblock.fs_frag - frags; j++) {
			if (testbmap(i + j))
				continue;
			for (k = 1; k < frags; k++)
				if (testbmap(i + j + k))
					break;
			if (k < frags) {
				j += k;
				continue;
			}
			cg = dtog(&sblock, i + j);
			cgbp = cgget(cg);
			cgp = cgbp->b_un.b_cg;
			if (!check_cgmagic(cg, cgbp))
				return (0);
			baseblk = dtogd(&sblock, i + j);
			for (k = 0; k < frags; k++) {
				setbmap(i + j + k);
				clrbit(cg_blksfree(cgp), baseblk + k);
			}
			n_blks += frags;
			if (frags == sblock.fs_frag)
				cgp->cg_cs.cs_nbfree--;
			else
				cgp->cg_cs.cs_nffree -= frags;
			dirty(cgbp);
			return (i + j);
		}
	}
	return (0);
}

/*
 * Free a previously allocated block
 */
void
freeblk(ufs2_daddr_t blkno, long frags)
{
	struct inodesc idesc;

	idesc.id_blkno = blkno;
	idesc.id_numfrags = frags;
	(void)pass4check(&idesc);
}

/* Slow down IO so as to leave some disk bandwidth for other processes */
void
slowio_start()
{

	/* Delay one in every 8 operations */
	slowio_pollcnt = (slowio_pollcnt + 1) & 7;
	if (slowio_pollcnt == 0) {
		gettimeofday(&slowio_starttime, NULL);
	}
}

void
slowio_end()
{
	struct timeval tv;
	int delay_usec;

	if (slowio_pollcnt != 0)
		return;

	/* Update the slowdown interval. */
	gettimeofday(&tv, NULL);
	delay_usec = (tv.tv_sec - slowio_starttime.tv_sec) * 1000000 +
	    (tv.tv_usec - slowio_starttime.tv_usec);
	if (delay_usec < 64)
		delay_usec = 64;
	if (delay_usec > 2500000)
		delay_usec = 2500000;
	slowio_delay_usec = (slowio_delay_usec * 63 + delay_usec) >> 6;
	/* delay by 8 times the average IO delay */
	if (slowio_delay_usec > 64)
		usleep(slowio_delay_usec * 8);
}

/*
 * Find a pathname
 */
void
getpathname(char *namebuf, ino_t curdir, ino_t ino)
{
	int len;
	char *cp;
	struct inodesc idesc;
	static int busy = 0;

	if (curdir == ino && ino == ROOTINO) {
		(void)strcpy(namebuf, "/");
		return;
	}
	if (busy || !INO_IS_DVALID(curdir)) {
		(void)strcpy(namebuf, "?");
		return;
	}
	busy = 1;
	memset(&idesc, 0, sizeof(struct inodesc));
	idesc.id_type = DATA;
	idesc.id_fix = IGNORE;
	cp = &namebuf[MAXPATHLEN - 1];
	*cp = '\0';
	if (curdir != ino) {
		idesc.id_parent = curdir;
		goto namelookup;
	}
	while (ino != ROOTINO) {
		idesc.id_number = ino;
		idesc.id_func = findino;
		idesc.id_name = strdup("..");
		if ((ckinode(ginode(ino), &idesc) & FOUND) == 0)
			break;
	namelookup:
		idesc.id_number = idesc.id_parent;
		idesc.id_parent = ino;
		idesc.id_func = findname;
		idesc.id_name = namebuf;
		if ((ckinode(ginode(idesc.id_number), &idesc)&FOUND) == 0)
			break;
		len = strlen(namebuf);
		cp -= len;
		memmove(cp, namebuf, (size_t)len);
		*--cp = '/';
		if (cp < &namebuf[MAXNAMLEN])
			break;
		ino = idesc.id_number;
	}
	busy = 0;
	if (ino != ROOTINO)
		*--cp = '?';
	memmove(namebuf, cp, (size_t)(&namebuf[MAXPATHLEN] - cp));
}

void
catch(int sig)
{
	ckfini(0);
	exit(12);
}

/*
 * When preening, allow a single quit to signal
 * a special exit after file system checks complete
 * so that reboot sequence may be interrupted.
 */
void
catchquit(int sig)
{
	printf("returning to single-user after file system check\n");
	returntosingle = 1;
	(void)signal(SIGQUIT, SIG_DFL);
}

/*
 * determine whether an inode should be fixed.
 */
int
dofix(struct inodesc *idesc, const char *msg)
{

	switch (idesc->id_fix) {

	case DONTKNOW:
		if (idesc->id_type == DATA)
			direrror(idesc->id_number, msg);
		else
			pwarn("%s", msg);
		if (preen) {
			printf(" (SALVAGED)\n");
			idesc->id_fix = FIX;
			return (ALTERED);
		}
		if (reply("SALVAGE") == 0) {
			idesc->id_fix = NOFIX;
			return (0);
		}
		idesc->id_fix = FIX;
		return (ALTERED);

	case FIX:
		return (ALTERED);

	case NOFIX:
	case IGNORE:
		return (0);

	default:
		errx(EEXIT, "UNKNOWN INODESC FIX MODE %d", idesc->id_fix);
	}
	/* NOTREACHED */
	return (0);
}

#include <stdarg.h>

/*
 * An unexpected inconsistency occurred.
 * Die if preening or file system is running with soft dependency protocol,
 * otherwise just print message and continue.
 */
void
pfatal(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (!preen) {
		(void)vfprintf(stdout, fmt, ap);
		va_end(ap);
		if (usedsoftdep)
			(void)fprintf(stdout,
			    "\nUNEXPECTED SOFT UPDATE INCONSISTENCY\n");
		/*
		 * Force foreground fsck to clean up inconsistency.
		 */
		return;
	}
	if (cdevname == NULL)
		cdevname = strdup("fsck");
	(void)fprintf(stdout, "%s: ", cdevname);
	(void)vfprintf(stdout, fmt, ap);
	(void)fprintf(stdout,
	    "\n%s: UNEXPECTED%sINCONSISTENCY; RUN fsck MANUALLY.\n",
	    cdevname, usedsoftdep ? " SOFT UPDATE " : " ");
	/*
	 * Force foreground fsck to clean up inconsistency.
	 */
	ckfini(0);
	exit(EEXIT);
}

/*
 * Pwarn just prints a message when not preening or running soft dependency
 * protocol, or a warning (preceded by filename) when preening.
 */
void
pwarn(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (preen)
		(void)fprintf(stdout, "%s: ", cdevname);
	(void)vfprintf(stdout, fmt, ap);
	va_end(ap);
}

/*
 * Stub for routines from kernel.
 */
void
panic(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	pfatal("INTERNAL INCONSISTENCY:");
	(void)vfprintf(stdout, fmt, ap);
	va_end(ap);
	exit(EEXIT);
}

void
alarmhandler(int sig)
{
	got_sigalarm = 1;
}

int
ckinode(union dinode *dp, struct inodesc *idesc)
{
	off_t remsize, sizepb;
	int i, offset, ret;
	union dinode dino;
	ufs2_daddr_t ndb;
	mode_t mode;
	char pathbuf[MAXPATHLEN + 1];

	if (idesc->id_fix != IGNORE)
		idesc->id_fix = DONTKNOW;
	idesc->id_lbn = -1;
	idesc->id_entryno = 0;
	idesc->id_filesize = DIP(dp, di_size);
	mode = DIP(dp, di_mode) & IFMT;
	if (mode == IFBLK || mode == IFCHR || (mode == IFLNK &&
	    DIP(dp, di_size) < (unsigned)sblock.fs_maxsymlinklen))
		return (KEEPON);
	if (sblock.fs_magic == FS_UFS1_MAGIC)
		dino.dp1 = dp->dp1;
	else
		dino.dp2 = dp->dp2;
	ndb = howmany(DIP(&dino, di_size), sblock.fs_bsize);
	for (i = 0; i < NDADDR; i++) {
		idesc->id_lbn++;
		if (--ndb == 0 &&
		    (offset = blkoff(&sblock, DIP(&dino, di_size))) != 0)
			idesc->id_numfrags =
				numfrags(&sblock, fragroundup(&sblock, offset));
		else
			idesc->id_numfrags = sblock.fs_frag;
		if (DIP(&dino, di_db[i]) == 0) {
			if (idesc->id_type == DATA && ndb >= 0) {
				/* An empty block in a directory XXX */
				getpathname(pathbuf, idesc->id_number,
						idesc->id_number);
				pfatal("DIRECTORY %s: CONTAINS EMPTY BLOCKS",
					pathbuf);
				if (reply("ADJUST LENGTH") == 1) {
					dp = ginode(idesc->id_number);
					DIP_SET(dp, di_size,
					    i * sblock.fs_bsize);
					printf(
					    "YOU MUST RERUN FSCK AFTERWARDS\n");
					rerun = 1;
					inodirty();

				}
			}
			continue;
		}
		idesc->id_blkno = DIP(&dino, di_db[i]);
		if (idesc->id_type != DATA)
			ret = (*idesc->id_func)(idesc);
		else
			ret = dirscan(idesc);
		if (ret & STOP)
			return (ret);
	}
	idesc->id_numfrags = sblock.fs_frag;
	remsize = DIP(&dino, di_size) - sblock.fs_bsize * NDADDR;
	sizepb = sblock.fs_bsize;
	for (i = 0; i < NIADDR; i++) {
		sizepb *= NINDIR(&sblock);
		if (DIP(&dino, di_ib[i])) {
			idesc->id_blkno = DIP(&dino, di_ib[i]);
			ret = iblock(idesc, i + 1, remsize, BT_LEVEL1 + i);
			if (ret & STOP)
				return (ret);
		} else {
			idesc->id_lbn += sizepb / sblock.fs_bsize;
			if (idesc->id_type == DATA && remsize > 0) {
				/* An empty block in a directory XXX */
				getpathname(pathbuf, idesc->id_number,
						idesc->id_number);
				pfatal("DIRECTORY %s: CONTAINS EMPTY BLOCKS",
					pathbuf);
				if (reply("ADJUST LENGTH") == 1) {
					dp = ginode(idesc->id_number);
					DIP_SET(dp, di_size,
					    DIP(dp, di_size) - remsize);
					remsize = 0;
					printf(
					    "YOU MUST RERUN FSCK AFTERWARDS\n");
					rerun = 1;
					inodirty();
					break;
				}
			}
		}
		remsize -= sizepb;
	}
	return (KEEPON);
}

static int
iblock(struct inodesc *idesc, long ilevel, off_t isize, int type)
{
	struct bufarea *bp;
	int i, n, (*func)(struct inodesc *), nif;
	off_t sizepb;
	char buf[BUFSIZ];
	char pathbuf[MAXPATHLEN + 1];
	union dinode *dp;

	if (idesc->id_type != DATA) {
		func = idesc->id_func;
		if (((n = (*func)(idesc)) & KEEPON) == 0)
			return (n);
	} else
		func = dirscan;
	if (chkrange(idesc->id_blkno, idesc->id_numfrags))
		return (SKIP);
	bp = getdatablk(idesc->id_blkno, sblock.fs_bsize, type);
	ilevel--;
	for (sizepb = sblock.fs_bsize, i = 0; i < ilevel; i++)
		sizepb *= NINDIR(&sblock);
	if (howmany(isize, sizepb) > NINDIR(&sblock))
		nif = NINDIR(&sblock);
	else
		nif = howmany(isize, sizepb);
	if (idesc->id_func == pass1check && nif < NINDIR(&sblock)) {
		for (i = nif; i < NINDIR(&sblock); i++) {
			if (IBLK(bp, i) == 0)
				continue;
			(void)sprintf(buf, "PARTIALLY TRUNCATED INODE I=%lu",
			    (u_long)idesc->id_number);
			if (preen) {
				pfatal("%s", buf);
			} else if (dofix(idesc, buf)) {
				IBLK_SET(bp, i, 0);
				dirty(bp);
			}
		}
		flush(fswritefd, bp);
	}
	for (i = 0; i < nif; i++) {
		if (ilevel == 0)
			idesc->id_lbn++;
		if (IBLK(bp, i)) {
			idesc->id_blkno = IBLK(bp, i);
			if (ilevel == 0)
				n = (*func)(idesc);
			else
				n = iblock(idesc, ilevel, isize, type);
			if (n & STOP) {
				bp->b_flags &= ~B_INUSE;
				return (n);
			}
		} else {
			if (idesc->id_type == DATA && isize > 0) {
				/* An empty block in a directory XXX */
				getpathname(pathbuf, idesc->id_number,
						idesc->id_number);
				pfatal("DIRECTORY %s: CONTAINS EMPTY BLOCKS",
					pathbuf);
				if (reply("ADJUST LENGTH") == 1) {
					dp = ginode(idesc->id_number);
					DIP_SET(dp, di_size,
					    DIP(dp, di_size) - isize);
					isize = 0;
					printf(
					    "YOU MUST RERUN FSCK AFTERWARDS\n");
					rerun = 1;
					inodirty();
					bp->b_flags &= ~B_INUSE;
					return(STOP);
				}
			}
		}
		isize -= sizepb;
	}
	bp->b_flags &= ~B_INUSE;
	return (KEEPON);
}

/*
 * Check that a block in a legal block number.
 * Return 0 if in range, 1 if out of range.
 */
int
chkrange(ufs2_daddr_t blk, int cnt)
{
	int c;

	if (cnt <= 0 || blk <= 0 || blk > maxfsblock ||
	    cnt - 1 > maxfsblock - blk)
		return (1);
	if (cnt > sblock.fs_frag ||
	    fragnum(&sblock, blk) + cnt > sblock.fs_frag) {
		if (debug)
			printf("bad size: blk %ld, offset %i, size %d\n",
			    (long)blk, (int)fragnum(&sblock, blk), cnt);
		return (1);
	}
	c = dtog(&sblock, blk);
	if (blk < cgdmin(&sblock, c)) {
		if ((blk + cnt) > cgsblock(&sblock, c)) {
			if (debug) {
				printf("blk %ld < cgdmin %ld;",
				    (long)blk, (long)cgdmin(&sblock, c));
				printf(" blk + cnt %ld > cgsbase %ld\n",
				    (long)(blk + cnt),
				    (long)cgsblock(&sblock, c));
			}
			return (1);
		}
	} else {
		if ((blk + cnt) > cgbase(&sblock, c+1)) {
			if (debug)  {
				printf("blk %ld >= cgdmin %ld;",
				    (long)blk, (long)cgdmin(&sblock, c));
				printf(" blk + cnt %ld > sblock.fs_fpg %ld\n",
				    (long)(blk + cnt), (long)sblock.fs_fpg);
			}
			return (1);
		}
	}
	return (0);
}

/*
 * General purpose interface for reading inodes.
 */
union dinode *
ginode(ino_t inumber)
{
	ufs2_daddr_t iblk;

	if (inumber < ROOTINO || inumber > maxino)
		errx(EEXIT, "bad inode number %ju to ginode",
		    (uintmax_t)inumber);
	if (startinum == 0 ||
	    inumber < startinum || inumber >= startinum + INOPB(&sblock)) {
		iblk = ino_to_fsba(&sblock, inumber);
		if (pbp != 0)
			pbp->b_flags &= ~B_INUSE;
		pbp = getdatablk(iblk, sblock.fs_bsize, BT_INODES);
		startinum = (inumber / INOPB(&sblock)) * INOPB(&sblock);
	}
	if (sblock.fs_magic == FS_UFS1_MAGIC)
		return ((union dinode *)
		    &pbp->b_un.b_dinode1[inumber % INOPB(&sblock)]);
	return ((union dinode *)&pbp->b_un.b_dinode2[inumber % INOPB(&sblock)]);
}

/*
 * Special purpose version of ginode used to optimize first pass
 * over all the inodes in numerical order.
 */
static ino_t nextino, lastinum, lastvalidinum;
static long readcount, readpercg, fullcnt, inobufsize, partialcnt, partialsize;
static struct bufarea inobuf;

union dinode *
getnextinode(ino_t inumber, int rebuildcg)
{
	int j;
	long size;
	mode_t mode;
	ufs2_daddr_t ndb, blk;
	union dinode *dp;
	static caddr_t nextinop;

	if (inumber != nextino++ || inumber > lastvalidinum)
		errx(EEXIT, "bad inode number %ju to nextinode",
		    (uintmax_t)inumber);
	if (inumber >= lastinum) {
		readcount++;
		blk = ino_to_fsba(&sblock, lastinum);
		if (readcount % readpercg == 0) {
			size = partialsize;
			lastinum += partialcnt;
		} else {
			size = inobufsize;
			lastinum += fullcnt;
		}
		/*
		 * If getblk encounters an error, it will already have zeroed
		 * out the buffer, so we do not need to do so here.
		 */
		getblk(&inobuf, blk, size);
		nextinop = inobuf.b_un.b_buf;
	}
	dp = (union dinode *)nextinop;
	if (rebuildcg && nextinop == inobuf.b_un.b_buf) {
		/*
		 * Try to determine if we have reached the end of the
		 * allocated inodes.
		 */
		mode = DIP(dp, di_mode) & IFMT;
		if (mode == 0) {
			if (memcmp(dp->dp2.di_db, ufs2_zino.di_db,
				NDADDR * sizeof(ufs2_daddr_t)) ||
			      memcmp(dp->dp2.di_ib, ufs2_zino.di_ib,
				NIADDR * sizeof(ufs2_daddr_t)) ||
			      dp->dp2.di_mode || dp->dp2.di_size)
				return (NULL);
			goto inodegood;
		}
		if (!ftypeok(dp))
			return (NULL);
		ndb = howmany(DIP(dp, di_size), sblock.fs_bsize);
		if (ndb < 0)
			return (NULL);
		if (mode == IFBLK || mode == IFCHR)
			ndb++;
		if (mode == IFLNK) {
			/*
			 * Fake ndb value so direct/indirect block checks below
			 * will detect any garbage after symlink string.
			 */
			if (DIP(dp, di_size) < (off_t)sblock.fs_maxsymlinklen) {
				ndb = howmany(DIP(dp, di_size),
				    sizeof(ufs2_daddr_t));
				if (ndb > NDADDR) {
					j = ndb - NDADDR;
					for (ndb = 1; j > 1; j--)
						ndb *= NINDIR(&sblock);
					ndb += NDADDR;
				}
			}
		}
		for (j = ndb; ndb < NDADDR && j < NDADDR; j++)
			if (DIP(dp, di_db[j]) != 0)
				return (NULL);
		for (j = 0, ndb -= NDADDR; ndb > 0; j++)
			ndb /= NINDIR(&sblock);
		for (; j < NIADDR; j++)
			if (DIP(dp, di_ib[j]) != 0)
				return (NULL);
	}
inodegood:
	if (sblock.fs_magic == FS_UFS1_MAGIC)
		nextinop += sizeof(struct ufs1_dinode);
	else
		nextinop += sizeof(struct ufs2_dinode);
	return (dp);
}

void
setinodebuf(ino_t inum)
{

	if (inum % sblock.fs_ipg != 0)
		errx(EEXIT, "bad inode number %ju to setinodebuf",
		    (uintmax_t)inum);
	lastvalidinum = inum + sblock.fs_ipg - 1;
	startinum = 0;
	nextino = inum;
	lastinum = inum;
	readcount = 0;
	if (inobuf.b_un.b_buf != NULL)
		return;
	inobufsize = blkroundup(&sblock, INOBUFSIZE);
	fullcnt = inobufsize / ((sblock.fs_magic == FS_UFS1_MAGIC) ?
	    sizeof(struct ufs1_dinode) : sizeof(struct ufs2_dinode));
	readpercg = sblock.fs_ipg / fullcnt;
	partialcnt = sblock.fs_ipg % fullcnt;
	partialsize = partialcnt * ((sblock.fs_magic == FS_UFS1_MAGIC) ?
	    sizeof(struct ufs1_dinode) : sizeof(struct ufs2_dinode));
	if (partialcnt != 0) {
		readpercg++;
	} else {
		partialcnt = fullcnt;
		partialsize = inobufsize;
	}
	initbarea(&inobuf, BT_INODES);
	if ((inobuf.b_un.b_buf = Malloc((unsigned)inobufsize)) == NULL)
		errx(EEXIT, "cannot allocate space for inode buffer");
}

void
freeinodebuf(void)
{

	if (inobuf.b_un.b_buf != NULL)
		free((char *)inobuf.b_un.b_buf);
	inobuf.b_un.b_buf = NULL;
}

/*
 * Routines to maintain information about directory inodes.
 * This is built during the first pass and used during the
 * second and third passes.
 *
 * Enter inodes into the cache.
 */
void
cacheino(union dinode *dp, ino_t inumber)
{
	struct inoinfo *inp, **inpp;
	int i, blks;

	if (howmany(DIP(dp, di_size), sblock.fs_bsize) > NDADDR)
		blks = NDADDR + NIADDR;
	else
		blks = howmany(DIP(dp, di_size), sblock.fs_bsize);
	inp = (struct inoinfo *)
		Malloc(sizeof(*inp) + (blks - 1) * sizeof(ufs2_daddr_t));
	if (inp == NULL)
		errx(EEXIT, "cannot increase directory list");
	inpp = &inphead[inumber % dirhash];
	inp->i_nexthash = *inpp;
	*inpp = inp;
	inp->i_parent = inumber == ROOTINO ? ROOTINO : (ino_t)0;
	inp->i_dotdot = (ino_t)0;
	inp->i_number = inumber;
	inp->i_isize = DIP(dp, di_size);
	inp->i_numblks = blks;
	for (i = 0; i < (blks < NDADDR ? blks : NDADDR); i++)
		inp->i_blks[i] = DIP(dp, di_db[i]);
	if (blks > NDADDR)
		for (i = 0; i < NIADDR; i++)
			inp->i_blks[NDADDR + i] = DIP(dp, di_ib[i]);
	if (inplast == listmax) {
		listmax += 100;
		inpsort = (struct inoinfo **)realloc((char *)inpsort,
		    (unsigned)listmax * sizeof(struct inoinfo *));
		if (inpsort == NULL)
			errx(EEXIT, "cannot increase directory list");
	}
	inpsort[inplast++] = inp;
}

/*
 * Look up an inode cache structure.
 */
struct inoinfo *
getinoinfo(ino_t inumber)
{
	struct inoinfo *inp;

	for (inp = inphead[inumber % dirhash]; inp; inp = inp->i_nexthash) {
		if (inp->i_number != inumber)
			continue;
		return (inp);
	}
	errx(EEXIT, "cannot find inode %ju", (uintmax_t)inumber);
	return ((struct inoinfo *)0);
}

/*
 * Clean up all the inode cache structure.
 */
void
inocleanup(void)
{
	struct inoinfo **inpp;

	if (inphead == NULL)
		return;
	for (inpp = &inpsort[inplast - 1]; inpp >= inpsort; inpp--)
		free((char *)(*inpp));
	free((char *)inphead);
	free((char *)inpsort);
	inphead = inpsort = NULL;
}

void
inodirty(void)
{

	dirty(pbp);
}

void
clri(struct inodesc *idesc, const char *type, int flag)
{
	union dinode *dp;

	dp = ginode(idesc->id_number);
	if (flag == 1) {
		pwarn("%s %s", type,
		    (DIP(dp, di_mode) & IFMT) == IFDIR ? "DIR" : "FILE");
		pinode(idesc->id_number);
	}
	if (preen || reply("CLEAR") == 1) {
		if (preen)
			printf(" (CLEARED)\n");
		n_files--;
		(void)ckinode(dp, idesc);
		inoinfo(idesc->id_number)->ino_state = USTATE;
		clearinode(dp);
		inodirty();
	}
}

int
findname(struct inodesc *idesc)
{
	struct direct *dirp = idesc->id_dirp;

	if (dirp->d_ino != idesc->id_parent || idesc->id_entryno < 2) {
		idesc->id_entryno++;
		return (KEEPON);
	}
	memmove(idesc->id_name, dirp->d_name, (size_t)dirp->d_namlen + 1);
	return (STOP|FOUND);
}

int
findino(struct inodesc *idesc)
{
	struct direct *dirp = idesc->id_dirp;

	if (dirp->d_ino == 0)
		return (KEEPON);
	if (strcmp(dirp->d_name, idesc->id_name) == 0 &&
	    dirp->d_ino >= ROOTINO && dirp->d_ino <= maxino) {
		idesc->id_parent = dirp->d_ino;
		return (STOP|FOUND);
	}
	return (KEEPON);
}

int
clearentry(struct inodesc *idesc)
{
	struct direct *dirp = idesc->id_dirp;

	if (dirp->d_ino != idesc->id_parent || idesc->id_entryno < 2) {
		idesc->id_entryno++;
		return (KEEPON);
	}
	dirp->d_ino = 0;
	return (STOP|FOUND|ALTERED);
}

void
pinode(ino_t ino)
{
	union dinode *dp;
	char *p;
	struct passwd *pw;
	time_t t;

	printf(" I=%lu ", (u_long)ino);
	if (ino < ROOTINO || ino > maxino)
		return;
	dp = ginode(ino);
	printf(" OWNER=");
	if ((pw = getpwuid((int)DIP(dp, di_uid))) != 0)
		printf("%s ", pw->pw_name);
	else
		printf("%u ", (unsigned)DIP(dp, di_uid));
	printf("MODE=%o\n", DIP(dp, di_mode));
	if (preen)
		printf("%s: ", cdevname);
	printf("SIZE=%ju ", (uintmax_t)DIP(dp, di_size));
	t = DIP(dp, di_mtime);
	p = ctime(&t);
	printf("MTIME=%12.12s %4.4s ", &p[4], &p[20]);
}

void
blkerror(ino_t ino, const char *type, ufs2_daddr_t blk)
{

	pfatal("%jd %s I=%ju", (intmax_t)blk, type, (uintmax_t)ino);
	printf("\n");
	switch (inoinfo(ino)->ino_state) {

	case FSTATE:
	case FZLINK:
		inoinfo(ino)->ino_state = FCLEAR;
		return;

	case DSTATE:
	case DZLINK:
		inoinfo(ino)->ino_state = DCLEAR;
		return;

	case FCLEAR:
	case DCLEAR:
		return;

	default:
		errx(EEXIT, "BAD STATE %d TO BLKERR", inoinfo(ino)->ino_state);
		/* NOTREACHED */
	}
}

/*
 * allocate an unused inode
 */
ino_t
allocino(ino_t request, int type)
{
	ino_t ino;
	union dinode *dp;
	struct bufarea *cgbp;
	struct cg *cgp;
	int cg;

	if (request == 0)
		request = ROOTINO;
	else if (inoinfo(request)->ino_state != USTATE)
		return (0);
	for (ino = request; ino < maxino; ino++)
		if (inoinfo(ino)->ino_state == USTATE)
			break;
	if (ino == maxino)
		return (0);
	cg = ino_to_cg(&sblock, ino);
	cgbp = cgget(cg);
	cgp = cgbp->b_un.b_cg;
	if (!check_cgmagic(cg, cgbp))
		return (0);
	setbit(cg_inosused(cgp), ino % sblock.fs_ipg);
	cgp->cg_cs.cs_nifree--;
	switch (type & IFMT) {
	case IFDIR:
		inoinfo(ino)->ino_state = DSTATE;
		cgp->cg_cs.cs_ndir++;
		break;
	case IFREG:
	case IFLNK:
		inoinfo(ino)->ino_state = FSTATE;
		break;
	default:
		return (0);
	}
	dirty(cgbp);
	dp = ginode(ino);
	DIP_SET(dp, di_db[0], allocblk((long)1));
	if (DIP(dp, di_db[0]) == 0) {
		inoinfo(ino)->ino_state = USTATE;
		return (0);
	}
	DIP_SET(dp, di_mode, type);
	DIP_SET(dp, di_flags, 0);
	DIP_SET(dp, di_atime, time(NULL));
	DIP_SET(dp, di_ctime, DIP(dp, di_atime));
	DIP_SET(dp, di_mtime, DIP(dp, di_ctime));
	DIP_SET(dp, di_mtimensec, 0);
	DIP_SET(dp, di_ctimensec, 0);
	DIP_SET(dp, di_atimensec, 0);
	DIP_SET(dp, di_size, sblock.fs_fsize);
	DIP_SET(dp, di_blocks, btodb(sblock.fs_fsize));
	n_files++;
	inodirty();
	inoinfo(ino)->ino_type = IFTODT(type);
	return (ino);
}

/*
 * deallocate an inode
 */
void
freeino(ino_t ino)
{
	struct inodesc idesc;
	union dinode *dp;

	memset(&idesc, 0, sizeof(struct inodesc));
	idesc.id_type = ADDR;
	idesc.id_func = pass4check;
	idesc.id_number = ino;
	dp = ginode(ino);
	(void)ckinode(dp, &idesc);
	clearinode(dp);
	inodirty();
	inoinfo(ino)->ino_state = USTATE;
	n_files--;
}

/*
 * Propagate connected state through the tree.
 */
void
propagate(void)
{
	struct inoinfo **inpp, *inp;
	struct inoinfo **inpend;
	long change;

	inpend = &inpsort[inplast];
	do {
		change = 0;
		for (inpp = inpsort; inpp < inpend; inpp++) {
			inp = *inpp;
			if (inp->i_parent == 0)
				continue;
			if (inoinfo(inp->i_parent)->ino_state == DFOUND &&
			    INO_IS_DUNFOUND(inp->i_number)) {
				inoinfo(inp->i_number)->ino_state = DFOUND;
				change++;
			}
		}
	} while (change > 0);
}

/*
 * Scan each entry in a directory block.
 */
int
dirscan(struct inodesc *idesc)
{
	struct direct *dp;
	struct bufarea *bp;
	u_int dsize, n;
	long blksiz;
	char dbuf[DIRBLKSIZ];

	if (idesc->id_type != DATA)
		errx(EEXIT, "wrong type to dirscan %d", idesc->id_type);
	if (idesc->id_entryno == 0 &&
	    (idesc->id_filesize & (DIRBLKSIZ - 1)) != 0)
		idesc->id_filesize = roundup(idesc->id_filesize, DIRBLKSIZ);
	blksiz = idesc->id_numfrags * sblock.fs_fsize;
	if (chkrange(idesc->id_blkno, idesc->id_numfrags)) {
		idesc->id_filesize -= blksiz;
		return (SKIP);
	}
	idesc->id_loc = 0;
	for (dp = fsck_readdir(idesc); dp != NULL; dp = fsck_readdir(idesc)) {
		dsize = dp->d_reclen;
		if (dsize > sizeof(dbuf))
			dsize = sizeof(dbuf);
		memmove(dbuf, dp, (size_t)dsize);
		idesc->id_dirp = (struct direct *)dbuf;
		if ((n = (*idesc->id_func)(idesc)) & ALTERED) {
			bp = getdirblk(idesc->id_blkno, blksiz);
			memmove(bp->b_un.b_buf + idesc->id_loc - dsize, dbuf,
			    (size_t)dsize);
			dirty(bp);
			sbdirty();
		}
		if (n & STOP)
			return (n);
	}
	return (idesc->id_filesize > 0 ? KEEPON : STOP);
}

/*
 * get next entry in a directory.
 */
static struct direct *
fsck_readdir(struct inodesc *idesc)
{
	struct direct *dp, *ndp;
	struct bufarea *bp;
	long size, blksiz, fix, dploc;

	blksiz = idesc->id_numfrags * sblock.fs_fsize;
	bp = getdirblk(idesc->id_blkno, blksiz);
	if (idesc->id_loc % DIRBLKSIZ == 0 && idesc->id_filesize > 0 &&
	    idesc->id_loc < blksiz) {
		dp = (struct direct *)(bp->b_un.b_buf + idesc->id_loc);
		if (dircheck(idesc, dp))
			goto dpok;
		if (idesc->id_fix == IGNORE)
			return (0);
		fix = dofix(idesc, "DIRECTORY CORRUPTED");
		bp = getdirblk(idesc->id_blkno, blksiz);
		dp = (struct direct *)(bp->b_un.b_buf + idesc->id_loc);
		dp->d_reclen = DIRBLKSIZ;
		dp->d_ino = 0;
		dp->d_type = 0;
		dp->d_namlen = 0;
		dp->d_name[0] = '\0';
		if (fix)
			dirty(bp);
		idesc->id_loc += DIRBLKSIZ;
		idesc->id_filesize -= DIRBLKSIZ;
		return (dp);
	}
dpok:
	if (idesc->id_filesize <= 0 || idesc->id_loc >= blksiz)
		return NULL;
	dploc = idesc->id_loc;
	dp = (struct direct *)(bp->b_un.b_buf + dploc);
	idesc->id_loc += dp->d_reclen;
	idesc->id_filesize -= dp->d_reclen;
	if ((idesc->id_loc % DIRBLKSIZ) == 0)
		return (dp);
	ndp = (struct direct *)(bp->b_un.b_buf + idesc->id_loc);
	if (idesc->id_loc < blksiz && idesc->id_filesize > 0 &&
	    dircheck(idesc, ndp) == 0) {
		size = DIRBLKSIZ - (idesc->id_loc % DIRBLKSIZ);
		idesc->id_loc += size;
		idesc->id_filesize -= size;
		if (idesc->id_fix == IGNORE)
			return (0);
		fix = dofix(idesc, "DIRECTORY CORRUPTED");
		bp = getdirblk(idesc->id_blkno, blksiz);
		dp = (struct direct *)(bp->b_un.b_buf + dploc);
		dp->d_reclen += size;
		if (fix)
			dirty(bp);
	}
	return (dp);
}

/*
 * Verify that a directory entry is valid.
 * This is a superset of the checks made in the kernel.
 */
static int
dircheck(struct inodesc *idesc, struct direct *dp)
{
	size_t size;
	char *cp;
	u_char type;
	u_int8_t namlen;
	int spaceleft;

	spaceleft = DIRBLKSIZ - (idesc->id_loc % DIRBLKSIZ);
	if (dp->d_reclen == 0 ||
	    dp->d_reclen > spaceleft ||
	    (dp->d_reclen & 0x3) != 0)
		goto bad;
	if (dp->d_ino == 0)
		return (1);
	size = DIRSIZ(dp);
	namlen = dp->d_namlen;
	type = dp->d_type;
	if (dp->d_reclen < size ||
	    idesc->id_filesize < size ||
	    namlen == 0 ||
	    type > 15)
		goto bad;
	for (cp = dp->d_name, size = 0; size < namlen; size++)
		if (*cp == '\0' || (*cp++ == '/'))
			goto bad;
	if (*cp != '\0')
		goto bad;
	return (1);
bad:
	if (debug)
		printf("Bad dir: ino %d reclen %d namlen %d type %d name %s\n",
		    dp->d_ino, dp->d_reclen, dp->d_namlen, dp->d_type,
		    dp->d_name);
	return (0);
}

void
direrror(ino_t ino, const char *errmesg)
{

	fileerror(ino, ino, errmesg);
}

void
fileerror(ino_t cwd, ino_t ino, const char *errmesg)
{
	union dinode *dp;
	char pathbuf[MAXPATHLEN + 1];

	pwarn("%s ", errmesg);
	pinode(ino);
	printf("\n");
	getpathname(pathbuf, cwd, ino);
	if (ino < ROOTINO || ino > maxino) {
		pfatal("NAME=%s\n", pathbuf);
		return;
	}
	dp = ginode(ino);
	if (ftypeok(dp))
		pfatal("%s=%s\n",
		    (DIP(dp, di_mode) & IFMT) == IFDIR ? "DIR" : "FILE",
		    pathbuf);
	else
		pfatal("NAME=%s\n", pathbuf);
}

void
adjust(struct inodesc *idesc, int lcnt)
{
	union dinode *dp;
	int saveresolved;

	dp = ginode(idesc->id_number);
	if (DIP(dp, di_nlink) == lcnt) {
		/*
		 * If we have not hit any unresolved problems, are running
		 * in preen mode, and are on a file system using soft updates,
		 * then just toss any partially allocated files.
		 */
		if (resolved && preen && usedsoftdep) {
			clri(idesc, "UNREF", 1);
			return;
		} else {
			/*
			 * The file system can be marked clean even if
			 * a file is not linked up, but is cleared.
			 * Hence, resolved should not be cleared when
			 * linkup is answered no, but clri is answered yes.
			 */
			saveresolved = resolved;
			if (linkup(idesc->id_number, (ino_t)0, NULL) == 0) {
				resolved = saveresolved;
				clri(idesc, "UNREF", 0);
				return;
			}
			/*
			 * Account for the new reference created by linkup().
			 */
			dp = ginode(idesc->id_number);
			lcnt--;
		}
	}
	if (lcnt != 0) {
		pwarn("LINK COUNT %s", (lfdir == idesc->id_number) ? lfname :
			((DIP(dp, di_mode) & IFMT) == IFDIR ? "DIR" : "FILE"));
		pinode(idesc->id_number);
		printf(" COUNT %d SHOULD BE %d",
			DIP(dp, di_nlink), DIP(dp, di_nlink) - lcnt);
		if (preen || usedsoftdep) {
			if (lcnt < 0) {
				printf("\n");
				pfatal("LINK COUNT INCREASING");
			}
			if (preen)
				printf(" (ADJUSTED)\n");
		}
		if (preen || reply("ADJUST") == 1) {
			DIP_SET(dp, di_nlink, DIP(dp, di_nlink) - lcnt);
			inodirty();
		}
	}
}

static int
mkentry(struct inodesc *idesc)
{
	struct direct *dirp = idesc->id_dirp;
	struct direct newent;
	int newlen, oldlen;

	newent.d_namlen = strlen(idesc->id_name);
	newlen = DIRSIZ(&newent);
	if (dirp->d_ino != 0)
		oldlen = DIRSIZ(dirp);
	else
		oldlen = 0;
	if (dirp->d_reclen - oldlen < newlen)
		return (KEEPON);
	newent.d_reclen = dirp->d_reclen - oldlen;
	dirp->d_reclen = oldlen;
	dirp = (struct direct *)(((char *)dirp) + oldlen);
	dirp->d_ino = idesc->id_parent;	/* ino to be entered is in id_parent */
	dirp->d_reclen = newent.d_reclen;
	dirp->d_type = inoinfo(idesc->id_parent)->ino_type;
	dirp->d_namlen = newent.d_namlen;
	memmove(dirp->d_name, idesc->id_name, (size_t)newent.d_namlen + 1);
	return (ALTERED|STOP);
}

static int
chgino(struct inodesc *idesc)
{
	struct direct *dirp = idesc->id_dirp;

	if (memcmp(dirp->d_name, idesc->id_name, (int)dirp->d_namlen + 1))
		return (KEEPON);
	dirp->d_ino = idesc->id_parent;
	dirp->d_type = inoinfo(idesc->id_parent)->ino_type;
	return (ALTERED|STOP);
}

int
linkup(ino_t orphan, ino_t parentdir, char *name)
{
	union dinode *dp;
	int lostdir;
	ino_t oldlfdir;
	struct inodesc idesc;
	char tempname[BUFSIZ];

	memset(&idesc, 0, sizeof(struct inodesc));
	dp = ginode(orphan);
	lostdir = (DIP(dp, di_mode) & IFMT) == IFDIR;
	pwarn("UNREF %s ", lostdir ? "DIR" : "FILE");
	pinode(orphan);
	if (preen && DIP(dp, di_size) == 0)
		return (0);

	if (preen)
		printf(" (RECONNECTED)\n");
	else
		if (reply("RECONNECT") == 0)
			return (0);

	if (lfdir == 0) {
		dp = ginode(ROOTINO);
		idesc.id_name = strdup(lfname);
		idesc.id_type = DATA;
		idesc.id_func = findino;
		idesc.id_number = ROOTINO;
		if ((ckinode(dp, &idesc) & FOUND) != 0) {
			lfdir = idesc.id_parent;
		} else {
			pwarn("NO lost+found DIRECTORY");
			if (preen || reply("CREATE")) {
				lfdir = allocdir(ROOTINO, (ino_t)0, lfmode);
				if (lfdir != 0) {
					if (makeentry(ROOTINO, lfdir, lfname) != 0) {
						numdirs++;
						if (preen)
							printf(" (CREATED)\n");
					} else {
						freedir(lfdir, ROOTINO);
						lfdir = 0;
						if (preen)
							printf("\n");
					}
				}
			}
		}
		if (lfdir == 0) {
			pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY");
			printf("\n\n");
			return (0);
		}
	}
	dp = ginode(lfdir);
	if ((DIP(dp, di_mode) & IFMT) != IFDIR) {
		pfatal("lost+found IS NOT A DIRECTORY");
		if (reply("REALLOCATE") == 0)
			return (0);
		oldlfdir = lfdir;
		if ((lfdir = allocdir(ROOTINO, (ino_t)0, lfmode)) == 0) {
			pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n\n");
			return (0);
		}
		if ((changeino(ROOTINO, lfname, lfdir) & ALTERED) == 0) {
			pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n\n");
			return (0);
		}
		inodirty();
		idesc.id_type = ADDR;
		idesc.id_func = pass4check;
		idesc.id_number = oldlfdir;
		adjust(&idesc, inoinfo(oldlfdir)->ino_linkcnt + 1);
		inoinfo(oldlfdir)->ino_linkcnt = 0;
		dp = ginode(lfdir);
	}
	if (inoinfo(lfdir)->ino_state != DFOUND) {
		pfatal("SORRY. NO lost+found DIRECTORY\n\n");
		return (0);
	}
	(void)lftempname(tempname, orphan);
	if (makeentry(lfdir, orphan, (name ? name : tempname)) == 0) {
		pfatal("SORRY. NO SPACE IN lost+found DIRECTORY");
		printf("\n\n");
		return (0);
	}
	inoinfo(orphan)->ino_linkcnt--;
	if (lostdir) {
		if ((changeino(orphan, "..", lfdir) & ALTERED) == 0 &&
		    parentdir != (ino_t)-1)
			(void)makeentry(orphan, lfdir, "..");
		dp = ginode(lfdir);
		DIP_SET(dp, di_nlink, DIP(dp, di_nlink) + 1);
		inodirty();
		inoinfo(lfdir)->ino_linkcnt++;
		pwarn("DIR I=%lu CONNECTED. ", (u_long)orphan);
		if (parentdir != (ino_t)-1) {
			printf("PARENT WAS I=%lu\n", (u_long)parentdir);
			/*
			 * The parent directory, because of the ordering
			 * guarantees, has had the link count incremented
			 * for the child, but no entry was made.  This
			 * fixes the parent link count so that fsck does
			 * not need to be rerun.
			 */
			inoinfo(parentdir)->ino_linkcnt++;
		}
		if (preen == 0)
			printf("\n");
	}
	return (1);
}

/*
 * fix an entry in a directory.
 */
int
changeino(ino_t dir, const char *name, ino_t newnum)
{
	struct inodesc idesc;

	memset(&idesc, 0, sizeof(struct inodesc));
	idesc.id_type = DATA;
	idesc.id_func = chgino;
	idesc.id_number = dir;
	idesc.id_fix = DONTKNOW;
	idesc.id_name = strdup(name);
	idesc.id_parent = newnum;	/* new value for name */
	return (ckinode(ginode(dir), &idesc));
}

/*
 * make an entry in a directory
 */
int
makeentry(ino_t parent, ino_t ino, const char *name)
{
	union dinode *dp;
	struct inodesc idesc;
	char pathbuf[MAXPATHLEN + 1];

	if (parent < ROOTINO || parent >= maxino ||
	    ino < ROOTINO || ino >= maxino)
		return (0);
	memset(&idesc, 0, sizeof(struct inodesc));
	idesc.id_type = DATA;
	idesc.id_func = mkentry;
	idesc.id_number = parent;
	idesc.id_parent = ino;	/* this is the inode to enter */
	idesc.id_fix = DONTKNOW;
	idesc.id_name = strdup(name);
	dp = ginode(parent);
	if (DIP(dp, di_size) % DIRBLKSIZ) {
		DIP_SET(dp, di_size, roundup(DIP(dp, di_size), DIRBLKSIZ));
		inodirty();
	}
	if ((ckinode(dp, &idesc) & ALTERED) != 0)
		return (1);
	getpathname(pathbuf, parent, parent);
	dp = ginode(parent);
	if (expanddir(dp, pathbuf) == 0)
		return (0);
	return (ckinode(dp, &idesc) & ALTERED);
}

/*
 * Attempt to expand the size of a directory
 */
static int
expanddir(union dinode *dp, char *name)
{
	ufs2_daddr_t lastbn, newblk;
	struct bufarea *bp;
	char *cp, firstblk[DIRBLKSIZ];

	lastbn = lblkno(&sblock, DIP(dp, di_size));
	if (lastbn >= NDADDR - 1 || DIP(dp, di_db[lastbn]) == 0 ||
	    DIP(dp, di_size) == 0)
		return (0);
	if ((newblk = allocblk(sblock.fs_frag)) == 0)
		return (0);
	DIP_SET(dp, di_db[lastbn + 1], DIP(dp, di_db[lastbn]));
	DIP_SET(dp, di_db[lastbn], newblk);
	DIP_SET(dp, di_size, DIP(dp, di_size) + sblock.fs_bsize);
	DIP_SET(dp, di_blocks, DIP(dp, di_blocks) + btodb(sblock.fs_bsize));
	bp = getdirblk(DIP(dp, di_db[lastbn + 1]),
		sblksize(&sblock, DIP(dp, di_size), lastbn + 1));
	if (bp->b_errs)
		goto bad;
	memmove(firstblk, bp->b_un.b_buf, DIRBLKSIZ);
	bp = getdirblk(newblk, sblock.fs_bsize);
	if (bp->b_errs)
		goto bad;
	memmove(bp->b_un.b_buf, firstblk, DIRBLKSIZ);
	for (cp = &bp->b_un.b_buf[DIRBLKSIZ];
	     cp < &bp->b_un.b_buf[sblock.fs_bsize];
	     cp += DIRBLKSIZ)
		memmove(cp, &emptydir, sizeof emptydir);
	dirty(bp);
	bp = getdirblk(DIP(dp, di_db[lastbn + 1]),
		sblksize(&sblock, DIP(dp, di_size), lastbn + 1));
	if (bp->b_errs)
		goto bad;
	memmove(bp->b_un.b_buf, &emptydir, sizeof emptydir);
	pwarn("NO SPACE LEFT IN %s", name);
	if (preen)
		printf(" (EXPANDED)\n");
	else if (reply("EXPAND") == 0)
		goto bad;
	dirty(bp);
	inodirty();
	return (1);
bad:
	DIP_SET(dp, di_db[lastbn], DIP(dp, di_db[lastbn + 1]));
	DIP_SET(dp, di_db[lastbn + 1], 0);
	DIP_SET(dp, di_size, DIP(dp, di_size) - sblock.fs_bsize);
	DIP_SET(dp, di_blocks, DIP(dp, di_blocks) - btodb(sblock.fs_bsize));
	freeblk(newblk, sblock.fs_frag);
	return (0);
}

/*
 * allocate a new directory
 */
ino_t
allocdir(ino_t parent, ino_t request, int mode)
{
	ino_t ino;
	char *cp;
	union dinode *dp;
	struct bufarea *bp;
	struct inoinfo *inp;
	struct dirtemplate *dirp;

	ino = allocino(request, IFDIR|mode);
	dirp = &dirhead;
	dirp->dot_ino = ino;
	dirp->dotdot_ino = parent;
	dp = ginode(ino);
	bp = getdirblk(DIP(dp, di_db[0]), sblock.fs_fsize);
	if (bp->b_errs) {
		freeino(ino);
		return (0);
	}
	memmove(bp->b_un.b_buf, dirp, sizeof(struct dirtemplate));
	for (cp = &bp->b_un.b_buf[DIRBLKSIZ];
	     cp < &bp->b_un.b_buf[sblock.fs_fsize];
	     cp += DIRBLKSIZ)
		memmove(cp, &emptydir, sizeof emptydir);
	dirty(bp);
	DIP_SET(dp, di_nlink, 2);
	inodirty();
	if (ino == ROOTINO) {
		inoinfo(ino)->ino_linkcnt = DIP(dp, di_nlink);
		cacheino(dp, ino);
		return(ino);
	}
	if (!INO_IS_DVALID(parent)) {
		freeino(ino);
		return (0);
	}
	cacheino(dp, ino);
	inp = getinoinfo(ino);
	inp->i_parent = parent;
	inp->i_dotdot = parent;
	inoinfo(ino)->ino_state = inoinfo(parent)->ino_state;
	if (inoinfo(ino)->ino_state == DSTATE) {
		inoinfo(ino)->ino_linkcnt = DIP(dp, di_nlink);
		inoinfo(parent)->ino_linkcnt++;
	}
	dp = ginode(parent);
	DIP_SET(dp, di_nlink, DIP(dp, di_nlink) + 1);
	inodirty();
	return (ino);
}

/*
 * free a directory inode
 */
static void
freedir(ino_t ino, ino_t parent)
{
	union dinode *dp;

	if (ino != parent) {
		dp = ginode(parent);
		DIP_SET(dp, di_nlink, DIP(dp, di_nlink) - 1);
		inodirty();
	}
	freeino(ino);
}

/*
 * generate a temporary name for the lost+found directory.
 */
static int
lftempname(char *bufp, ino_t ino)
{
	ino_t in;
	char *cp;
	int namlen;

	cp = bufp + 2;
	for (in = maxino; in > 0; in /= 10)
		cp++;
	*--cp = 0;
	namlen = cp - bufp;
	in = ino;
	while (cp > bufp) {
		*--cp = (in % 10) + '0';
		in /= 10;
	}
	*cp = '#';
	return (namlen);
}

/*
 * Get a directory block.
 * Insure that it is held until another is requested.
 */
static struct bufarea *
getdirblk(ufs2_daddr_t blkno, long size)
{

	if (pdirbp != 0)
		pdirbp->b_flags &= ~B_INUSE;
	pdirbp = getdatablk(blkno, size, BT_DIRDATA);
	return (pdirbp);
}
