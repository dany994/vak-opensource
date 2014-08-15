/*
 * Utility for dealing with 2.xBSD filesystem images.
 *
 * Copyright (C) 2006-2011 Serge Vakulenko, <serge@vak.ru>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <time.h>
//#include <sys/stat.h>
//#include <errno.h>
#include <getopt.h>
//#include <fts.h>

#include "libufs.h"
#include "manifest.h"

int verbose;
int extract;
int add;
int newfs;
int check;
int fix;
int mount;
int scan;
unsigned kbytes;

static const char *program_version =
    "BSD 4.4 file system tool, version 0.0\n"
    "Copyright (C) 2014 Serge Vakulenko";

static const char *program_bug_address = "<serge@vak.ru>";

static struct option program_options[] = {
    { "help",       no_argument,        0,  'h' },
    { "version",    no_argument,        0,  'V' },
    { "verbose",    no_argument,        0,  'v' },
    { "add",        no_argument,        0,  'a' },
    { "extract",    no_argument,        0,  'x' },
    { "check",      no_argument,        0,  'c' },
    { "fix",        no_argument,        0,  'f' },
    { "mount",      no_argument,        0,  'm' },
    { "scan",       no_argument,        0,  'S' },
    { "new",        required_argument,  0,  'n' },
    { "manifest",   required_argument,  0,  'M' },
    { 0 }
};

static void print_help (char *progname)
{
    char *p = strrchr (progname, '/');
    if (p)
        progname = p+1;

    printf ("%s\n", program_version);
    printf ("This program is free software; it comes with ABSOLUTELY NO WARRANTY;\n"
            "see the BSD 3-Clause License for more details.\n");
    printf ("\n");
    printf ("Usage:\n");
    printf ("  %s [--verbose] filesys.img\n", progname);
    printf ("  %s --check [--fix] filesys.img\n", progname);
    printf ("  %s --new=kbytes [--manifest=file] filesys.img [dir]\n", progname);
    printf ("  %s --mount filesys.img dir\n", progname);
    printf ("  %s --add filesys.img files...\n", progname);
    printf ("  %s --extract filesys.img\n", progname);
    printf ("  %s --scan dir > file\n", progname);
    printf ("\n");
    printf ("Options:\n");
    printf ("  -c, --check         Check filesystem, use -c -f to fix.\n");
    printf ("  -f, --fix           Fix bugs in filesystem.\n");
    printf ("  -n NUM, --new=NUM   Create new filesystem, size in kbytes.\n");
    printf ("                      Add files from specified directory (optional)\n");
    printf ("  -M file, --manifest=file  List of files and attributes to create.\n");
    printf ("  -m, --mount         Mount the filesystem.\n");
    printf ("  -a, --add           Add files to filesystem.\n");
    printf ("  -x, --extract       Extract all files.\n");
    printf ("  -S, --scan          Create a manifest from directory contents.\n");
    printf ("  -v, --verbose       Be verbose.\n");
    printf ("  -V, --version       Print version information and then exit.\n");
    printf ("  -h, --help          Print this message.\n");
    printf ("\n");
    printf ("Report bugs to \"%s\".\n", program_bug_address);
}

#if 0
void print_inode (fs_inode_t *inode,
    char *dirname, char *filename, FILE *out)
{
    fprintf (out, "%s/%s", dirname, filename);
    switch (inode->mode & INODE_MODE_FMT) {
    case INODE_MODE_FDIR:
        if (filename[0] != 0)
            fprintf (out, "/");
        break;
    case INODE_MODE_FCHR:
        fprintf (out, " - char %d %d",
            inode->addr[1] >> 8, inode->addr[1] & 0xff);
        break;
    case INODE_MODE_FBLK:
        fprintf (out, " - block %d %d",
            inode->addr[1] >> 8, inode->addr[1] & 0xff);
        break;
    default:
        fprintf (out, " - %lu bytes", inode->size);
        break;
    }
    fprintf (out, "\n");
}

void print_indirect_block (struct uufsd *disk, unsigned int bno, FILE *out)
{
    unsigned short nb;
    unsigned char data [BSDFS_BSIZE];
    int i;

    fprintf (out, " [%d]", bno);
    if (! fs_read_block (disk, bno, data)) {
        fprintf (stderr, "read error at block %d\n", bno);
        return;
    }
    for (i=0; i<BSDFS_BSIZE-2; i+=2) {
        nb = data [i+1] << 8 | data [i];
        if (nb)
            fprintf (out, " %d", nb);
    }
}

void print_double_indirect_block (struct uufsd *disk, unsigned int bno, FILE *out)
{
    unsigned short nb;
    unsigned char data [BSDFS_BSIZE];
    int i;

    fprintf (out, " [%d]", bno);
    if (! fs_read_block (disk, bno, data)) {
        fprintf (stderr, "read error at block %d\n", bno);
        return;
    }
    for (i=0; i<BSDFS_BSIZE-2; i+=2) {
        nb = data [i+1] << 8 | data [i];
        if (nb)
            print_indirect_block (disk, nb, out);
    }
}

void print_triple_indirect_block (struct uufsd *disk, unsigned int bno, FILE *out)
{
    unsigned short nb;
    unsigned char data [BSDFS_BSIZE];
    int i;

    fprintf (out, " [%d]", bno);
    if (! fs_read_block (disk, bno, data)) {
        fprintf (stderr, "read error at block %d\n", bno);
        return;
    }
    for (i=0; i<BSDFS_BSIZE-2; i+=2) {
        nb = data [i+1] << 8 | data [i];
        if (nb)
            print_indirect_block (disk, nb, out);
    }
}

void print_inode_blocks (fs_inode_t *inode, FILE *out)
{
    int i;

    if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FCHR ||
        (inode->mode & INODE_MODE_FMT) == INODE_MODE_FBLK)
        return;

    fprintf (out, "    ");
    for (i=0; i<NDADDR; ++i) {
        if (inode->addr[i] == 0)
            continue;
        fprintf (out, " %d", inode->addr[i]);
    }
    if (inode->addr[NDADDR] != 0)
        print_indirect_block (inode->fs, inode->addr[NDADDR], out);
    if (inode->addr[NDADDR+1] != 0)
        print_double_indirect_block (inode->fs,
            inode->addr[NDADDR+1], out);
    if (inode->addr[NDADDR+2] != 0)
        print_triple_indirect_block (inode->fs,
            inode->addr[NDADDR+2], out);
    fprintf (out, "\n");
}

void extract_inode (fs_inode_t *inode, char *path)
{
    int fd, n, mode;
    unsigned long offset;
    unsigned char data [BSDFS_BSIZE];

    /* Allow read/write for user. */
    mode = (inode->mode & 0777) | 0600;
    fd = open (path, O_CREAT | O_RDWR, mode);
    if (fd < 0) {
        perror (path);
        return;
    }
    for (offset = 0; offset < inode->size; offset += BSDFS_BSIZE) {
        n = inode->size - offset;
        if (n > BSDFS_BSIZE)
            n = BSDFS_BSIZE;
        if (! fs_inode_read (inode, offset, data, n)) {
            fprintf (stderr, "%s: read error at offset %ld\n",
                path, offset);
            break;
        }
        if (write (fd, data, n) != n) {
            fprintf (stderr, "%s: write error\n", path);
            break;
        }
    }
    close (fd);
}

void extractor (fs_inode_t *dir, fs_inode_t *inode,
    char *dirname, char *filename, void *arg)
{
    FILE *out = arg;
    char *path, *relpath;

    if (verbose)
        print_inode (inode, dirname, filename, out);

    if ((inode->mode & INODE_MODE_FMT) != INODE_MODE_FDIR &&
        (inode->mode & INODE_MODE_FMT) != INODE_MODE_FREG)
        return;

    path = alloca (strlen (dirname) + strlen (filename) + 2);
    strcpy (path, dirname);
    strcat (path, "/");
    strcat (path, filename);
    for (relpath=path; *relpath == '/'; relpath++)
        continue;

    if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
        if (mkdir (relpath, 0775) < 0 && errno != EEXIST)
            perror (relpath);
        /* Scan subdirectory. */
        fs_directory_scan (inode, path, extractor, arg);
    } else {
        extract_inode (inode, relpath);
    }
}

void scanner (fs_inode_t *dir, fs_inode_t *inode,
    char *dirname, char *filename, void *arg)
{
    FILE *out = arg;
    char *path;

    print_inode (inode, dirname, filename, out);

    if (verbose > 1) {
        /* Print a list of blocks. */
        print_inode_blocks (inode, out);
        if (verbose > 2) {
            fs_inode_print (inode, out);
            printf ("--------\n");
        }
    }
    if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FDIR &&
        inode->number != BSDFS_ROOT_INODE) {
        /* Scan subdirectory. */
        path = alloca (strlen (dirname) + strlen (filename) + 2);
        strcpy (path, dirname);
        strcat (path, "/");
        strcat (path, filename);
        fs_directory_scan (inode, path, scanner, arg);
    }
}

/*
 * Create a directory.
 */
void add_directory (struct uufsd *disk, char *name, int mode, int owner, int group)
{
    fs_inode_t dir, parent;
    char buf [BSDFS_BSIZE], *p;

    /* Open parent directory. */
    strcpy (buf, name);
    p = strrchr (buf, '/');
    if (p)
        *p = 0;
    else
        *buf = 0;
    if (! fs_inode_lookup (disk, &parent, buf)) {
        fprintf (stderr, "%s: cannot open directory\n", buf);
        return;
    }

    /* Create directory. */
    mode &= 07777;
    mode |= INODE_MODE_FDIR;
    int done = fs_inode_create (disk, &dir, name, mode);
    if (! done) {
        fprintf (stderr, "%s: directory inode create failed\n", name);
        return;
    }
    if (done == 1) {
        /* The directory already existed. */
        return;
    }
    dir.uid = owner;
    dir.gid = group;
    fs_inode_save (&dir, 1);

    /* Make parent link '..' */
    strcpy (buf, name);
    strcat (buf, "/..");
    if (! fs_inode_link (disk, &dir, buf, parent.number)) {
        fprintf (stderr, "%s: dotdot link failed\n", name);
        return;
    }
    if (! fs_inode_get (disk, &parent, parent.number)) {
        fprintf (stderr, "inode %d: cannot open parent\n", parent.number);
        return;
    }
    ++parent.nlink;
    fs_inode_save (&parent, 1);
/*printf ("*** inode %d: increment link counter to %d\n", parent.number, parent.nlink);*/
}

/*
 * Create a device node.
 */
void add_device (struct uufsd *disk, char *name, int mode, int owner, int group,
    int type, int majr, int minr)
{
    fs_inode_t dev;

    mode &= 07777;
    mode |= (type == 'b') ? INODE_MODE_FBLK : INODE_MODE_FCHR;
    if (! fs_inode_create (disk, &dev, name, mode)) {
        fprintf (stderr, "%s: device inode create failed\n", name);
        return;
    }
    dev.addr[1] = majr << 8 | minr;
    dev.uid = owner;
    dev.gid = group;
    time (&dev.mtime);
    fs_inode_save (&dev, 1);
}

/*
 * Copy regular file to filesystem.
 */
void add_file (struct uufsd *disk, const char *path, const char *dirname,
    int mode, int owner, int group)
{
    fs_file_t file;
    FILE *fd;
    char accpath [BSDFS_BSIZE];
    unsigned char data [BSDFS_BSIZE];
    struct stat st;
    int len;

    if (dirname && *dirname) {
        /* Concatenate directory name and file name. */
        strcpy (accpath, dirname);
        len = strlen (accpath);
        if (accpath[len-1] != '/' && path[0] != '/')
            strcat (accpath, "/");
        strcat (accpath, path);
    } else {
        /* Use filename relative to current directory. */
        strcpy (accpath, path);
    }
    fd = fopen (accpath, "r");
    if (! fd) {
        perror (accpath);
        return;
    }
    fstat (fileno(fd), &st);
    if (mode == -1)
        mode = st.st_mode;
    mode &= 07777;
    mode |= INODE_MODE_FREG;
    if (! fs_file_create (disk, &file, path, mode)) {
        fprintf (stderr, "%s: cannot create\n", path);
        return;
    }
    for (;;) {
        len = fread (data, 1, sizeof (data), fd);
/*      printf ("read %d bytes from %s\n", len, accpath);*/
        if (len < 0)
            perror (accpath);
        if (len <= 0)
            break;
        if (! fs_file_write (&file, data, len)) {
            fprintf (stderr, "%s: write error\n", path);
            break;
        }
    }
    file.inode.uid = owner;
    file.inode.gid = group;
    file.inode.mtime = st.st_mtime;
    file.inode.dirty = 1;
    fs_file_close (&file);
    fclose (fd);
}

/*
 * Create a symlink.
 */
void add_symlink (struct uufsd *disk, const char *path, const char *link,
    int mode, int owner, int group)
{
    fs_file_t file;
    int len;

    mode &= 07777;
    mode |= INODE_MODE_FLNK;
    if (! fs_file_create (disk, &file, path, mode)) {
        fprintf (stderr, "%s: cannot create\n", path);
        return;
    }
    len = strlen (link);
    if (! fs_file_write (&file, (unsigned char*) link, len)) {
        fprintf (stderr, "%s: write error\n", path);
        return;
    }
    file.inode.uid = owner;
    file.inode.gid = group;
    time (&file.inode.mtime);
    file.inode.dirty = 1;
    fs_file_close (&file);
}

/*
 * Create a hard link.
 */
void add_hardlink (struct uufsd *disk, const char *path, const char *link)
{
    fs_inode_t source, target;

    /* Find source. */
    if (! fs_inode_lookup (disk, &source, link)) {
        fprintf (stderr, "%s: link source not found\n", link);
        return;
    }
    if ((source.mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
        fprintf (stderr, "%s: cannot link directories\n", link);
        return;
    }

    /* Create target link. */
    if (! fs_inode_link (disk, &target, path, source.number)) {
        fprintf (stderr, "%s: link failed\n", path);
        return;
    }
    source.nlink++;
    fs_inode_save (&source, 1);
}

/*
 * Create a file/device/directory in the filesystem.
 * When name is ended by slash as "name/", directory is created.
 */
void add_object (struct uufsd *disk, char *name)
{
    int majr, minr;
    char type;
    char *p;

    if (verbose) {
        printf ("%s\n", name);
    }
    p = strrchr (name, '/');
    if (p && p[1] == 0) {
        *p = 0;
        add_directory (disk, name, 0777, 0, 0);
        return;
    }
    p = strrchr (name, '!');
    if (p) {
        *p++ = 0;
        if (sscanf (p, "%c%d:%d", &type, &majr, &minr) != 3 ||
            (type != 'c' && type != 'b') ||
            majr < 0 || majr > 255 || minr < 0 || minr > 255) {
            fprintf (stderr, "%s: invalid device specification\n", p);
            fprintf (stderr, "expected c<major>:<minor> or b<major>:<minor>\n");
            return;
        }
        add_device (disk, name, 0666, 0, 0, type, majr, minr);
        return;
    }
    add_file (disk, name, 0, -1, 0, 0);
}

/*
 * Add the contents from the specified directory.
 * Use the optional manifest file.
 */
void add_contents (struct uufsd *disk, const char *dirname, const char *manifest)
{
    manifest_t m;
    void *cursor;
    char *path, *link;
    int filetype, mode, owner, group, majr, minr;
    int ndirs = 0, nfiles = 0, nlinks = 0, nsymlinks = 0, ndevs = 0;

    if (manifest) {
        /* Load manifest from file. */
        if (! manifest_load (&m, manifest)) {
            fprintf (stderr, "%s: cannot read\n", manifest);
            return;
        }
    } else {
        /* Create manifest from directory contents. */
        if (! manifest_scan (&m, dirname)) {
            fprintf (stderr, "%s: cannot read\n", dirname);
            return;
        }
    }

    /* For every file in the manifest,
     * add it to the target filesystem. */
    cursor = 0;
    while ((filetype = manifest_iterate (&m, &cursor, &path, &link, &mode,
        &owner, &group, &majr, &minr)) != 0)
    {
        switch (filetype) {
        case 'd':
            add_directory (disk, path, mode, owner, group);
            ndirs++;
            break;
        case 'f':
            add_file (disk, path, dirname, mode, owner, group);
            nfiles++;
            break;
        case 'l':
            add_hardlink (disk, path, link);
            nlinks++;
            break;
        case 's':
            add_symlink (disk, path, link, mode, owner, group);
            nsymlinks++;
            break;
        case 'b':
            add_device (disk, path, mode, owner, group, 'b', majr, minr);
            ndevs++;
            break;
        case 'c':
            add_device (disk, path, mode, owner, group, 'c', majr, minr);
            ndevs++;
            break;
        }
    }
    fs_sync (disk, 0);
    ufs_disk_close (&disk);
    printf ("Installed %u directories, %u files, %u devices, %u links, %u symlinks\n",
        ndirs, nfiles, ndevs, nlinks, nsymlinks);
}
#endif

/*
 * Dump the superblock.
 */
void ufs_print(struct uufsd *disk, FILE *out)
{
    struct fs *sb = &disk->d_fs;
    int j;

    fprintf(out, "sblkno            int32_t          0x%08x\n",        sb->fs_sblkno);
    fprintf(out, "cblkno            int32_t          0x%08x\n",        sb->fs_cblkno);
    fprintf(out, "iblkno            int32_t          0x%08x\n",        sb->fs_iblkno);
    fprintf(out, "dblkno            int32_t          0x%08x\n",        sb->fs_dblkno);

    fprintf(out, "old_cgoffset      int32_t          0x%08x\n",        sb->fs_old_cgoffset);
    fprintf(out, "old_cgmask        int32_t          0x%08x\n",        sb->fs_old_cgmask);
    fprintf(out, "old_time          int32_t          %10u\n",          (unsigned int)sb->fs_old_time);
    fprintf(out, "old_size          int32_t          0x%08x\n",        sb->fs_old_size);
    fprintf(out, "old_dsize         int32_t          0x%08x\n",        sb->fs_old_dsize);
    fprintf(out, "ncg               int32_t          0x%08x\n",        sb->fs_ncg);
    fprintf(out, "bsize             int32_t          0x%08x\n",        sb->fs_bsize);
    fprintf(out, "fsize             int32_t          0x%08x\n",        sb->fs_fsize);
    fprintf(out, "frag              int32_t          0x%08x\n",        sb->fs_frag);

    fprintf(out, "minfree           int32_t          0x%08x\n",        sb->fs_minfree);
    fprintf(out, "old_rotdelay      int32_t          0x%08x\n",        sb->fs_old_rotdelay);
    fprintf(out, "old_rps           int32_t          0x%08x\n",        sb->fs_old_rps);

    fprintf(out, "bmask             int32_t          0x%08x\n",        sb->fs_bmask);
    fprintf(out, "fmask             int32_t          0x%08x\n",        sb->fs_fmask);
    fprintf(out, "bshift            int32_t          0x%08x\n",        sb->fs_bshift);
    fprintf(out, "fshift            int32_t          0x%08x\n",        sb->fs_fshift);

    fprintf(out, "maxcontig         int32_t          0x%08x\n",        sb->fs_maxcontig);
    fprintf(out, "maxbpg            int32_t          0x%08x\n",        sb->fs_maxbpg);

    fprintf(out, "fragshift         int32_t          0x%08x\n",        sb->fs_fragshift);
    fprintf(out, "fsbtodb           int32_t          0x%08x\n",        sb->fs_fsbtodb);
    fprintf(out, "sbsize            int32_t          0x%08x\n",        sb->fs_sbsize);
    fprintf(out, "spare1            int32_t[2]       0x%08x 0x%08x\n", sb->fs_spare1[0], sb->fs_spare1[1]);
    fprintf(out, "nindir            int32_t          0x%08x\n",        sb->fs_nindir);
    fprintf(out, "inopb             int32_t          0x%08x\n",        sb->fs_inopb);
    fprintf(out, "old_nspf          int32_t          0x%08x\n",        sb->fs_old_nspf);

    fprintf(out, "optim             int32_t          0x%08x\n",        sb->fs_optim);

    fprintf(out, "old_npsect        int32_t          0x%08x\n",        sb->fs_old_npsect);
    fprintf(out, "old_interleave    int32_t          0x%08x\n",        sb->fs_old_interleave);
    fprintf(out, "old_trackskew     int32_t          0x%08x\n",        sb->fs_old_trackskew);

    fprintf(out, "id                int32_t[2]       0x%08x 0x%08x\n", sb->fs_id[0], sb->fs_id[1]);

    fprintf(out, "old_csaddr        int32_t          0x%08x\n",        sb->fs_old_csaddr);
    fprintf(out, "cssize            int32_t          0x%08x\n",        sb->fs_cssize);
    fprintf(out, "cgsize            int32_t          0x%08x\n",        sb->fs_cgsize);

    fprintf(out, "spare2            int32_t          0x%08x\n",        sb->fs_spare2);
    fprintf(out, "old_nsect         int32_t          0x%08x\n",        sb->fs_old_nsect);
    fprintf(out, "old_spc           int32_t          0x%08x\n",        sb->fs_old_spc);

    fprintf(out, "old_ncyl          int32_t          0x%08x\n",        sb->fs_old_ncyl);

    fprintf(out, "old_cpg           int32_t          0x%08x\n",        sb->fs_old_cpg);
    fprintf(out, "ipg               int32_t          0x%08x\n",        sb->fs_ipg);
    fprintf(out, "fpg               int32_t          0x%08x\n",        sb->fs_fpg);

    //dbg_dump_csum("internal old_cstotal", &sb->fs_old_cstotal);

    fprintf(out, "fmod              int8_t           0x%02x\n",        sb->fs_fmod);
    fprintf(out, "clean             int8_t           0x%02x\n",        sb->fs_clean);
    fprintf(out, "ronly             int8_t           0x%02x\n",        sb->fs_ronly);
    fprintf(out, "old_flags         int8_t           0x%02x\n",        sb->fs_old_flags);
    fprintf(out, "fsmnt             u_char[MAXMNTLEN] \"%s\"\n",       sb->fs_fsmnt);
    fprintf(out, "volname           u_char[MAXVOLLEN] \"%s\"\n",       sb->fs_volname);
    fprintf(out, "swuid             u_int64_t        0x%08x%08x\n",    ((unsigned*)&(sb->fs_swuid))[1],
                                                                       ((unsigned*)&(sb->fs_swuid))[0]);

    fprintf(out, "pad               int32_t          0x%08x\n",        sb->fs_pad);

    fprintf(out, "cgrotor           int32_t          0x%08x\n",        sb->fs_cgrotor);
    /*
     * struct csum[MAXCSBUFS] - is only maintained in memory
     */
/*  fprintf(out, " int32_t\n", sb->*fs_maxcluster);*/
    fprintf(out, "old_cpc           int32_t          0x%08x\n",        sb->fs_old_cpc);
    /*
     * int16_t fs_opostbl[16][8] - is dumped when used in dbg_dump_sptbl
     */
    fprintf(out, "maxbsize          int32_t          0x%08x\n",        sb->fs_maxbsize);
    fprintf(out, "unrefs            int64_t          0x%08jx\n",       (uintmax_t)sb->fs_unrefs);
    fprintf(out, "sblockloc         int64_t          0x%08x%08x\n",    ((unsigned*)&(sb->fs_sblockloc))[1],
                                                                       ((unsigned*)&(sb->fs_sblockloc))[0]);

    //dbg_dump_csum_total("internal cstotal", &sb->fs_cstotal);

    fprintf(out, "time              ufs_time_t       %10u\n",          (unsigned)sb->fs_time);

    fprintf(out, "size              int64_t          0x%08x%08x\n",    ((unsigned*)&(sb->fs_size))[1],
                                                                       ((unsigned*)&(sb->fs_size))[0]);
    fprintf(out, "dsize             int64_t          0x%08x%08x\n",    ((unsigned*)&(sb->fs_dsize))[1],
                                                                       ((unsigned*)&(sb->fs_dsize))[0]);
    fprintf(out, "csaddr            ufs2_daddr_t     0x%08x%08x\n",    ((unsigned*)&(sb->fs_csaddr))[1],
                                                                       ((unsigned*)&(sb->fs_csaddr))[0]);
    fprintf(out, "pendingblocks     int64_t          0x%08x%08x\n",    ((unsigned*)&(sb->fs_pendingblocks))[1],
                                                                       ((unsigned*)&(sb->fs_pendingblocks))[0]);
    fprintf(out, "pendinginodes     int32_t          0x%08x\n",        sb->fs_pendinginodes);

    for (j = 0; j < FSMAXSNAP; j++) {
        fprintf(out, "snapinum          int32_t[%2d]      0x%08x\n", j, sb->fs_snapinum[j]);
        if (! sb->fs_snapinum[j]) { /* list is dense */
            break;
        }
    }
    fprintf(out, "avgfilesize       int32_t          0x%08x\n",        sb->fs_avgfilesize);
    fprintf(out, "avgfpdir          int32_t          0x%08x\n",        sb->fs_avgfpdir);
    fprintf(out, "save_cgsize       int32_t          0x%08x\n",        sb->fs_save_cgsize);
    fprintf(out, "flags             int32_t          0x%08x\n",        sb->fs_flags);
    fprintf(out, "contigsumsize     int32_t          0x%08x\n",        sb->fs_contigsumsize);
    fprintf(out, "maxsymlinklen     int32_t          0x%08x\n",        sb->fs_maxsymlinklen);
    fprintf(out, "old_inodefmt      int32_t          0x%08x\n",        sb->fs_old_inodefmt);
    fprintf(out, "maxfilesize       u_int64_t        0x%08x%08x\n",    ((unsigned*)&(sb->fs_maxfilesize))[1],
                                                                       ((unsigned*)&(sb->fs_maxfilesize))[0]);
    fprintf(out, "qbmask            int64_t          0x%08x%08x\n",    ((unsigned*)&(sb->fs_qbmask))[1],
                                                                       ((unsigned*)&(sb->fs_qbmask))[0]);
    fprintf(out, "qfmask            int64_t          0x%08x%08x\n",    ((unsigned*)&(sb->fs_qfmask))[1],
                                                                       ((unsigned*)&(sb->fs_qfmask))[0]);
    fprintf(out, "state             int32_t          0x%08x\n",        sb->fs_state);
    fprintf(out, "old_postblformat  int32_t          0x%08x\n",        sb->fs_old_postblformat);
    fprintf(out, "old_nrpos         int32_t          0x%08x\n",        sb->fs_old_nrpos);
    fprintf(out, "spare5            int32_t[2]       0x%08x 0x%08x\n", sb->fs_spare5[0], sb->fs_spare5[1]);
    fprintf(out, "magic             int32_t          0x%08x\n",        sb->fs_magic);
}

int main (int argc, char **argv)
{
    int i, key;
    struct uufsd disk;
    manifest_t m;
    const char *manifest = 0;

    for (;;) {
        key = getopt_long (argc, argv, "vaxmSn:cfM:",
            program_options, 0);
        if (key == -1)
            break;
        switch (key) {
        case 'v':
            ++verbose;
            break;
        case 'a':
            ++add;
            break;
        case 'x':
            ++extract;
            break;
        case 'n':
            ++newfs;
            kbytes = strtol (optarg, 0, 0);
            break;
        case 'c':
            ++check;
            break;
        case 'f':
            ++fix;
            break;
        case 'm':
            ++mount;
            break;
        case 'S':
            ++scan;
            break;
        case 'M':
            manifest = optarg;
            break;
        case 'V':
            printf ("%s\n", program_version);
            return 0;
        case 'h':
            print_help (argv[0]);
            return 0;
        default:
            print_help (argv[0]);
            return -1;
        }
    }
    i = optind;
    if (extract + newfs + check + add + mount + scan > 1) {
        print_help (argv[0]);
        return -1;
    }

    if (newfs) {
        /* Create new filesystem. */
        if (i != argc-1 && i != argc-2) {
            print_help (argv[0]);
            return -1;
        }
        if (kbytes < 64) {
            /* Need at least 16 blocks. */
            fprintf (stderr, "%s: too small\n", argv[i]);
            return -1;
        }
#if 0
        //TODO
        if (! fs_create (&disk, argv[i], kbytes)) {
            fprintf (stderr, "%s: cannot create filesystem\n", argv[i]);
            return -1;
        }
        printf ("Created filesystem %s - %u kbytes\n", argv[i], kbytes);

        if (i == argc-2) {
            /* Add the contents from the specified directory.
             * Use the optional manifest file. */
            add_contents (&disk, argv[i+1], manifest);
        }
        ufs_disk_close (&disk);
#endif
        return 0;
    }

    if (check) {
        /* Check filesystem for errors, and optionally fix them. */
        if (i != argc-1) {
            print_help (argv[0]);
            return -1;
        }
#if 0
        //TODO
        if (! fs_open (&disk, argv[i], fix)) {
            fprintf (stderr, "%s: cannot open\n", argv[i]);
            return -1;
        }
        fs_check (&disk);
        ufs_disk_close (&disk);
#endif
        return 0;
    }

    if (scan) {
        /* Create a manifest from directory contents. */
        if (i != argc-1) {
            print_help (argv[0]);
            return -1;
        }
        if (! manifest_scan (&m, argv[i])) {
            fprintf (stderr, "%s: cannot read\n", argv[i]);
            return -1;
        }
        manifest_print (&m);
        return 0;
    }

    /* Add or extract or info. */
    if (i >= argc) {
        print_help (argv[0]);
        return -1;
    }
    if (ufs_disk_fillout (&disk, argv[i]) < 0) {
        fprintf (stderr, "%s: cannot open\n", argv[i]);
        return -1;
    }

    if (extract) {
        /* Extract all files to current directory. */
        if (i != argc-1) {
            print_help (argv[0]);
            return -1;
        }
#if 0
        //TODO
        if (! fs_inode_get (&disk, &inode, ROOTINO)) {
            fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
            return -1;
        }
        fs_directory_scan (&inode, "", extractor, (void*) stdout);
        ufs_disk_close (&disk);
#endif
        return 0;
    }

    if (add) {
        /* Add files i+1..argc-1 to filesystem. */
        if (i >= argc) {
            print_help (argv[0]);
            return -1;
        }
#if 0
        //TODO
        while (++i < argc)
            add_object (&disk, argv[i]);
        fs_sync (&disk, 0);
        ufs_disk_close (&disk);
#endif
        return 0;
    }

    if (mount) {
        /* Mount the filesystem. */
        if (i != argc-2) {
            print_help (argv[0]);
            return -1;
        }
        return ufs_mount (&disk, argv[i+1]);
    }

    /* Print the structure of flesystem. */
    if (i != argc-1) {
        print_help (argv[0]);
        return -1;
    }
    ufs_print (&disk, stdout);
#if 0
    //TODO
    if (verbose) {
        fs_inode_t inode;

        if (! fs_inode_get (&disk, &inode, ROOTINO)) {
            fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
            return -1;
        }
        printf ("--------\n");
        printf ("/\n");
        if (verbose > 1) {
            /* Print a list of blocks. */
            print_inode_blocks (&inode, stdout);
            if (verbose > 2) {
                fs_inode_print (&inode, stdout);
                printf ("--------\n");
            }
        }
        fs_directory_scan (&inode, "", scanner, (void*) stdout);
    }
#endif
    ufs_disk_close (&disk);
    return 0;
}
