/*
 * Flash memory programmer for Microchip PIC32 microcontrollers.
 *
 * Copyright (C) 2014 Serge Vakulenko
 *
 * This file is part of PIC32PROG project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <locale.h>

#include "device.h"
#include "localize.h"

#define VERSION         "1."SVNVERSION

int debug_level;
char *progname;
const char *copyright;
device_t *device;

#if defined (__CYGWIN32__) || defined (MINGW32)
/*
 * Delay in milliseconds: Windows.
 */
#include <windows.h>

void mdelay (unsigned msec)
{
    Sleep (msec);
}
#else
/*
 * Delay in milliseconds: Unix.
 */
void mdelay (unsigned msec)
{
    usleep (msec * 1000);
}
#endif

void quit (void)
{
    if (device != 0) {
        device_close (device);
        free (device);
        device = 0;
    }
}

void interrupted (int signum)
{
    fprintf (stderr, _("\nInterrupted.\n"));
    quit();
    _exit (-1);
}

void do_status ()
{
    /* Open and detect the device. */
    atexit (quit);
    device = device_open (debug_level);
    if (! device) {
        fprintf (stderr, _("Error detecting device -- check cable!\n"));
        exit (1);
    }
    printf (_("Stack:\n"));
    // TODO
}

void do_program (char *filename)
{
#if 0
    // TODO
    if (! read_prog (filename)) {
        fprintf (stderr, _("%s: bad file format\n"), filename);
        exit (1);
    }
#endif
    /* Open and detect the device. */
    atexit (quit);
    device = device_open (debug_level);
    if (! device) {
        fprintf (stderr, _("Error detecting device -- check cable!\n"));
        exit (1);
    }

    printf (_("Write program: "));
    fflush (stdout);
    // TODO
    printf (_("done\n"));
}

void do_read (char *filename)
{
    FILE *fd;

    fd = fopen (filename, "wb");
    if (! fd) {
        perror (filename);
        exit (1);
    }

    /* Open and detect the device. */
    atexit (quit);
    device = device_open (debug_level);
    if (! device) {
        fprintf (stderr, _("Error detecting device -- check cable!\n"));
        exit (1);
    }
    printf ("Read program: " );
    fflush (stdout);
    // TODO
    printf (_("done\n"));
    fclose (fd);
}

/*
 * Print copying part of license
 */
static void gpl_show_copying (void)
{
    printf ("%s.\n\n", copyright);
    printf ("This program is free software; you can redistribute it and/or modify\n");
    printf ("it under the terms of the GNU General Public License as published by\n");
    printf ("the Free Software Foundation; either version 2 of the License, or\n");
    printf ("(at your option) any later version.\n");
    printf ("\n");
    printf ("This program is distributed in the hope that it will be useful,\n");
    printf ("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    printf ("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    printf ("GNU General Public License for more details.\n");
    printf ("\n");
}

/*
 * Print NO WARRANTY part of license
 */
static void gpl_show_warranty (void)
{
    printf ("%s.\n\n", copyright);
    printf ("BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n");
    printf ("FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n");
    printf ("OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n");
    printf ("PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n");
    printf ("OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n");
    printf ("MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n");
    printf ("TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n");
    printf ("PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n");
    printf ("REPAIR OR CORRECTION.\n");
    printf("\n");
    printf ("IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n");
    printf ("WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n");
    printf ("REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n");
    printf ("INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n");
    printf ("OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n");
    printf ("TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n");
    printf ("YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n");
    printf ("PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n");
    printf ("POSSIBILITY OF SUCH DAMAGES.\n");
    printf("\n");
}

int main (int argc, char **argv)
{
    int ch, read_mode = 0;
    static const struct option long_options[] = {
        { "help",        0, 0, 'h' },
        { "warranty",    0, 0, 'W' },
        { "copying",     0, 0, 'C' },
        { "version",     0, 0, 'V' },
        { "skip-verify", 0, 0, 'S' },
        { NULL,          0, 0, 0 },
    };

    /* Set locale and message catalogs. */
    setlocale (LC_ALL, "");
#if defined (__CYGWIN32__) || defined (MINGW32)
    /* Files with localized messages should be placed in
     * the current directory or in c:/Program Files/pmktool. */
    if (access ("./ru/LC_MESSAGES/pmktool.mo", R_OK) == 0)
        bindtextdomain ("pmktool", ".");
    else
        bindtextdomain ("pmktool", "c:/Program Files/pmktool");
#else
    bindtextdomain ("pmktool", "/usr/local/share/locale");
#endif
    textdomain ("pmktool");

    setvbuf (stdout, (char *)NULL, _IOLBF, 0);
    setvbuf (stderr, (char *)NULL, _IOLBF, 0);
    printf (_("Utility for MK-54 calculator, Version %s\n"), VERSION);
    progname = argv[0];
    copyright = _("    Copyright: (C) 2014 Serge Vakulenko");
    signal (SIGINT, interrupted);
#ifdef __linux__
    signal (SIGHUP, interrupted);
#endif
    signal (SIGTERM, interrupted);

    while ((ch = getopt_long (argc, argv, "vDhrCVW",
      long_options, 0)) != -1) {
        switch (ch) {
        case 'D':
            ++debug_level;
            continue;
        case 'r':
            ++read_mode;
            continue;
        case 'h':
            break;
        case 'V':
            /* Version already printed above. */
            return 0;
        case 'C':
            gpl_show_copying ();
            return 0;
        case 'W':
            gpl_show_warranty ();
            return 0;
        }
usage:
        printf ("%s.\n\n", copyright);
        printf ("PIC32prog comes with ABSOLUTELY NO WARRANTY; for details\n");
        printf ("use `--warranty' option. This is Open Source software. You are\n");
        printf ("welcome to redistribute it under certain conditions. Use the\n");
        printf ("'--copying' option for details.\n\n");
        printf ("Probe:\n");
        printf ("       pmktool\n");
        printf ("\nWrite flash memory:\n");
        printf ("       pmktool [-v] file.srec\n");
        printf ("       pmktool [-v] file.hex\n");
        printf ("\nRead memory:\n");
        printf ("       pmktool -r file.bin\n");
        printf ("\nArgs:\n");
        printf ("       file.prog           Code file in SREC format\n");
        printf ("       -r                  Read mode\n");
        printf ("       -D                  Debug mode\n");
        printf ("       -h, --help          Print this help message\n");
        printf ("       -V, --version       Print version\n");
        printf ("       -C, --copying       Print copying information\n");
        printf ("       -W, --warranty      Print warranty information\n");
        printf ("\n");
        return 0;
    }
    printf ("%s\n", copyright);
    argc -= optind;
    argv += optind;

    switch (argc) {
    case 0:
        do_status ();
        break;
    case 1:
        if (read_mode) {
            do_read (argv[0]);
        } else {
            do_program (argv[0]);
        }
        break;
    default:
        goto usage;
    }
    quit ();
    return 0;
}
