/*
 * Decode KOI-7 files from BESM-6.
 *
 * Copyright (C) 2009 Serge Vakulenko, <serge@vak.ru>
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this file and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING.txt" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

const char version[] = "1.0";
const char copyright[] = "Copyright (C) 2009 Serge Vakulenko";

char *progname;
int verbose;
int trace;
int debug;

void decode (char *filename)
{
	FILE *fd;
	int c;

	fd = fopen (filename, "r");
	for (;;) {
		c = getc (fd);
		switch (c) {
		case EOF:
			fclose (fd);
			return;
		case '\n':
			fputc ('\n', stdout);
			break;
                case '`':	fputs ("Ю", stdout); break;
                case 'a':       fputs ("А", stdout); break;
                case 'b':       fputs ("Б", stdout); break;
                case 'c':       fputs ("Ц", stdout); break;
                case 'd':       fputs ("Д", stdout); break;
                case 'e':       fputs ("Е", stdout); break;
                case 'f':       fputs ("Ф", stdout); break;
                case 'g':       fputs ("Г", stdout); break;
                case 'h':       fputs ("Х", stdout); break;
                case 'i':       fputs ("И", stdout); break;
                case 'j':       fputs ("Й", stdout); break;
                case 'k':       fputs ("К", stdout); break;
                case 'l':       fputs ("Л", stdout); break;
                case 'm':       fputs ("М", stdout); break;
                case 'n':       fputs ("Н", stdout); break;
                case 'o':       fputs ("О", stdout); break;
                case 'p':       fputs ("П", stdout); break;
                case 'q':       fputs ("Я", stdout); break;
                case 'r':       fputs ("Р", stdout); break;
                case 's':       fputs ("С", stdout); break;
                case 't':       fputs ("Т", stdout); break;
                case 'u':       fputs ("У", stdout); break;
                case 'v':       fputs ("Ж", stdout); break;
                case 'w':       fputs ("В", stdout); break;
                case 'x':       fputs ("Ь", stdout); break;
                case 'y':       fputs ("Ы", stdout); break;
                case 'z':       fputs ("З", stdout); break;
                case '{':       fputs ("Ш", stdout); break;
                case '|':       fputs ("Э", stdout); break;
                case '}':       fputs ("Щ", stdout); break;
                case '~':       fputs ("Ч", stdout); break;
		default:
			if (c >= ' ' && c <= '_') {
				/* Символ ASCII. */
				fputc (c, stdout);

			} else if (c > 0200) {
				/* Пробелы. */
				c -= 0200;
				while (c-- > 0)
					fputc (' ', stdout);
			} else {
				printf ("<%03o>", c);
			}
			break;
		}
	}
}

void usage ()
{
	fprintf (stderr, "Decode KOI7 to UTF8, Version %s, %s\n", version, copyright);
	fprintf (stderr, "Usage:\n\t%s [-vtd] file...\n", progname);
	fprintf (stderr, "Options:\n");
	fprintf (stderr, "\t-v\tverbose mode\n");
	fprintf (stderr, "\t-t\ttrace mode\n");
	fprintf (stderr, "\t-d\tdebug\n");
	exit (-1);
}

int main (int argc, char **argv)
{
	int i;

	progname = *argv;
	for (;;) {
		switch (getopt (argc, argv, "vtd")) {
		case EOF:
			break;
		case 'v':
			++verbose;
			continue;
		case 't':
			++trace;
			continue;
		case 'd':
			++debug;
			continue;
		default:
			usage ();
		}
		break;
	}
	argc -= optind;
	argv += optind;
	if (argc < 1)
		usage ();

	for (i=0; i<argc; ++i)
		decode (argv[i]);

	return (0);
}
