/*
 * $Header: daemon.c,v 2.0 92/08/05 08:42:46 vak Exp $
 * $Log:	daemon.c,v $
 * Revision 2.0  92/08/05  08:42:46  vak
 * 
 * 
 * Revision 1.2  92/06/19  22:19:52  usenet
 * Fix errors. Rudnev
 * 
 */
/*
 * Демон почтового сервера.
 * Постоянно крутится и читает почтовый ящик, принадлежащий серверу.
 * Приходящая почта раздраконивается на отдельные письма,
 * и на каждое письмо ставится в очередь.
 * Затем, если очередь непуста, выбирает заявки из очереди и на каждую
 * из них вызывает обработчик заявок server.
 * Таким образом, обработка заявок происходит последовательно,
 * друг за другом. Это облегчает синхронизацию и уменьшает
 * пиковую нагрузку на процессор.
 *
 * Copyright (C) 1991 Сергей Вакуленко.
 */

# include <stdio.h>
# include "config.h"

# define TMPFNAME "/tmp/NSdXXXXXX"

static char tmpf [] = TMPFNAME;

static void runqueuer (), runbatcher (), enqueue ();
static int getident (), dequeue (), batch (), feed (), notify ();

extern FILE *f2open ();
extern long filesize (), time ();
extern char *mktemp (), *strcpy ();

main (argc, argv)
char **argv;
{
	int pid;
	char qactivefile [256];

	timeinit ();
	umask (002);
	if (argc > 2) {
		fprintf (stderr, "usage: %s [config]\n", argv [0]);
		exit (-1);
	}
	if (! config (argv [1])) {
		fprintf (stderr, "cannot read config file\n");
		exit (-1);
	}

	/* let's fork */
	pid = fork ();
	if (pid < 0) {
		fprintf (stderr, "%s: cannot fork\n", argv [0]);
		exit (-1);
	}
	if (pid)
		exit (0);
	/* now we are a child */

	/* make temp file name */
	mktemp (tmpf);

	/* catch signals */
	sigign ();

	/* save our process id to file */
	savepid ("daemonpid",2);

	messg ("daemon started");

	runcheck ();
	messg ("database checked");
# ifdef REMOTEDB
	pinggroups ();
# endif
	sprintf (qactivefile, "%s/queueactive", SERVDIR);
	for (;;) {
		static long lastfeed = 0;
		static long lastnotify = 0;
		long curtime;

		if (filesize (MAILBOX))
			runqueuer ();
		if (filesize (qactivefile))
			runbatcher ();
		curtime = time ((long *) 0);
		if (FEEDTIME && (! lastfeed || lastfeed + FEEDTIME*60L < curtime)) {
			feed ();
			lastfeed = curtime;
		}
		if (NOTIFYTIME && (! lastnotify || lastnotify + NOTIFYTIME*60L < curtime)) {
			notify ();
			lastnotify = curtime;
		}
		sleep (DAEMONDELAY);
	}

	/* NOTREACHED */
}

int runcheck ()
{
# ifdef notdef
	char cmd [256];

	sprintf (cmd, "exec %s/bin/nscheck -s %s/groups", SERVDIR, SERVDIR);
	if (system (cmd) != 0) {
		error ("groups check failed, daemon halted");
		quit ();
	}
# endif
}

/*
 * Abnormal termination of program.
 */

quit ()
{
	exit (-1);
}

/*
 * VARARGS1
 * Output error message to log file.
 */

messg (s, a, b, c, d)
char *s, *a, *b, *c, *d;
{
	log ("d", s, a, b, c, d);
}
/*
 * VARARGS1
 * Output error message to log file.
 */

error (s, a, b, c, d)
char *s, *a, *b, *c, *d;
{
	log ("d +ERR+", s, a, b, c, d);
}

/*
 * Scan line from specefied offset to find
 * string, beginning with "From ".
 * Return offset.
 */

static long findfrom (fd, off, fromptr)
FILE *fd;
long off;
char **fromptr;
{
	register c, boln, bof;
	register long ret;
	static char from [100];
	char *p;

	fseek (fd, off, 0);
	bof = 1;
	boln = 1;
	ret = off;
	*fromptr = "";
	for (;;) {
		c = getc (fd);
		if (c < 0)
			return (bof ? -1 : off);
		bof = 0;
		++off;
		if (c == '\n') {
			boln = 1;
			ret = off;
			continue;
		}
		if (boln && c=='F') {
			if (getc (fd) == 'r' && getc (fd) == 'o' &&
			    getc (fd) == 'm' && getc (fd) == ' ') {
				fgets (from, sizeof (from), fd);
				*fromptr = from;
				for (p=from; *p && *p!=' ' && *p!='\n' &&
					*p!='\r'; ++p);
				*p = 0;
				return (ret);
			}
			off = ret + 1;
			fseek (fd, off, 0);
		}
		boln = 0;
	}
}

static void runqueuer ()
{
	FILE *fd, *tfd;
	register long beg, next;
	int ident;
	char name [100];
	char from [100], *fromptr;

	fd = fopen (MAILBOX, "r");
	if (! fd) {
		error ("cannot read %s", MAILBOX);
		return;
	}
	beg = findfrom (fd, 0L, &fromptr);
	strcpy (from, fromptr);
	for (;;) {
		next = findfrom (fd, beg+1, &fromptr);
		if (next < 0)
			break;

		/* get next free queue ident */
		ident = getident ();

		/* make queue file name */
		sprintf (name, "%s/queue/q%d", SERVDIR, ident);

		/* create queue file */
		tfd = fopen (name, "w");
		if (! tfd) {
			error ("cannot open %s", name);
			quit ();
		}

		/* save query into queue file */
		fseek (fd, beg, 0);
		ffncopy (fd, tfd, next - beg);
		if (ferror (tfd)) {
			error ("error writing to %s", name);
			quit ();
		}
		fclose (tfd);

		/* append name of query to queue active file */
		enqueue (name);

		messg ("queued %d from %s", ident, from);
		beg = next;
		strcpy (from, fromptr);
	}
	fclose (fd);
	if (close (creat (MAILBOX, 0600)) < 0) {
		error ("cannot rewrite %s", MAILBOX);
		quit ();
	}
}

/*
 * Get next free queue ident from ident file.
 * Increment ident in file.
 */

static int getident ()
{
	FILE *fd;
	int ident;

	/* if queue file does not exist, create it */
	if (f2access (SERVDIR, "queueident", 0))
		f2clean (SERVDIR, "queueident");

	/* open queue file */
	fd = f2open (SERVDIR, "queueident", "r+");
	if (! fd) {
		error ("cannot open %s/queueident", SERVDIR);
		quit ();
	}

	/* get next free ident from ident file */
	ident = 0;
	fscanf (fd, "%d", &ident);

	/* increment ident */
	rewind (fd);
	fprintf (fd, "%d          \n", ident + 1);
	fclose (fd);

	return (ident);
}

/*
 * Append batch file name to atcive queue file.
 */

static void enqueue (name)
char *name;
{
	FILE *fd;

	/* open active file for appending */
	fd = f2open (SERVDIR, "queueactive", "a");
	if (! fd) {
		error ("cannot open %s/queueactive", SERVDIR);
		quit ();
	}
	fprintf (fd, "%s\n", name);
	fclose (fd);
}

static void runbatcher ()
{
	FILE *actf;
	char name [100];
	long begtime;

	/* open temp file on read/write */
	close (creat (tmpf, 0664));
	actf = fopen (tmpf, "r+");
	if (! actf) {
		error ("cannot create %s", tmpf);
		quit ();
	}
	unlink (tmpf);

	/* get queue active file into temp file */
	if (! dequeue (actf))
		quit ();

	/* remembre the start time */
	begtime = time ((long *) 0);
	
	/* process batches */
	while (getstr (actf, name)) {
		if (! *name)
			continue;
# ifdef REMOTEDB
		pinggroups ();
# endif
		if (batch (name)) {
			messg ("processed %s", name);
			unlink (name);
		}
		/* don't run more than 30 min */
		if (begtime + 30*60 < time ((long *) 0))
			break;
	}
	/* enqueue the rest back */
	while (getstr (actf, name)) 
		enqueue (name);
	fclose (actf);
}

/*
 * Get content of active file into temp file,
 * then clean active file.
 * Leave temp file rewinded.
 */

static int dequeue (actfd)
FILE *actfd;
{
	FILE *fd;

	/* open active file for reading */
	fd = f2open (SERVDIR, "queueactive", "r");
	if (! fd)
		return (1);     /* no active jobs in queue */

	/* copy active file to temp file */
	if (ffcopy (fd, actfd) < 0) {
		fclose (fd);
		return (0);
	}
	fclose (fd);
	rewind (actfd);

	/* clean active file */
	f2clean (SERVDIR, "queueactive");
	return (1);
}

/*
 * Process batch job.
 */

static int batch (name)
char *name;
{
	char buf [256];
	int rez;

	if (f2access (SERVDIR, "bin/server", 1)) {
		error ("cannot execute %s/bin/server", SERVDIR);
		return (0);
	}
	sprintf (buf, "exec %s/bin/server %s < %s", SERVDIR, CONFIGFILE, name);
	rez = system (buf);
	if (rez) {
		error ("error %d while executing '%s'", rez, buf);
		return (0);
	}
	return (1);
}

static int feed ()
{
	char buf [256];
	int rez;

	if (f2access (SERVDIR, "bin/newnews", 1)) {
		error ("cannot execute %s/bin/newnews", SERVDIR);
		return (0);
	}
	sprintf (buf, "exec %s/bin/newnews %s", SERVDIR, CONFIGFILE);
	rez = system (buf);
	if (rez) {
		error ("error %d while executing '%s'", rez, buf);
		return (0);
	}
	return (1);
}

static int notify ()
{
	char buf [256];
	int rez;

	if (f2access (SERVDIR, "bin/notify", 1)) {
		error ("cannot execute %s/bin/notify", SERVDIR);
		return (0);
	}
	sprintf (buf, "exec %s/bin/notify %s", SERVDIR, CONFIGFILE);
	rez = system (buf);
	if (rez) {
		error ("error %d while executing '%s'", rez, buf);
		return (0);
	}
	return (1);
}
