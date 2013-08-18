/*
 * Baofeng UV-5R Clone Utility
 *
 * Copyright (C) 2013 Serge Vakulenko, <serge@vak.ru>
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
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <termios.h>

const char version[] = "1.0";
const char copyright[] = "Copyright (C) 2013 Serge Vakulenko";

char *progname;
char *portname;
int verbose;
int trace;
int debug;

struct termios oldtio, newtio;

extern char *optarg;
extern int optind;

void usage ()
{
    fprintf (stderr, "Baofeng UV-5R Clone Utility, Version %s, %s\n", version, copyright);
    fprintf (stderr, "Usage:\n\t%s [-vtd] file\n", progname);
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "  -v          verbose mode\n");
    fprintf (stderr, "  -t          trace mode\n");
    fprintf (stderr, "  -d          debug\n");
    exit (-1);
}

void print_hex (const char *data, int len)
{
    int i;

    printf ("%02x", (unsigned char) data[0]);
    for (i=1; i<len; i++)
        printf ("-%02x", (unsigned char) data[i]);
}

//
// Open the serial port.
//
int open_port (char *portname)
{
    int fd;

    // Don't wait for carrier (DCD).
    fd = open (portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror (portname);
        exit (-1);
    }

    // Flush received data pending on the port.
    tcflush (fd, TCIFLUSH);

    tcgetattr (fd, &oldtio);
    newtio = oldtio;

    // 8 data bits
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;
    // enable receiver, set local mode
    newtio.c_cflag |= CLOCAL | CREAD;
    // no parity
    newtio.c_cflag &= ~PARENB;
    // 1 stop bit
    newtio.c_cflag &= ~CSTOPB;
    // no h/w handshake
    newtio.c_cflag &= ~CRTSCTS;
    // raw input
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // raw output
    newtio.c_oflag &= ~OPOST;
    // software flow control disabled
    newtio.c_iflag &= ~IXON;
    // do not translate CR to NL
    newtio.c_iflag &= ~ICRNL;

    cfsetispeed(&newtio, 9600);
    cfsetospeed(&newtio, 9600);

    tcsetattr (fd, TCSANOW, &newtio);

    // Clear the non-block flag.
    int flags = fcntl (fd, F_GETFL, 0);
    if (flags < 0) {
        perror("F_GETFL");
        exit (-1);
    }
    flags &= ~O_NONBLOCK;
    if (fcntl (fd, F_SETFL, flags) < 0) {
        perror("F_SETFL");
        exit (-1);
    }
    return fd;
}

//
// Close the serial port.
//
void close_port (int fd)
{
    // Restore the port mode.
    tcsetattr (fd, TCSANOW, &oldtio);
    close (fd);
}

int read_with_timeout (int fd, char *data, int len)
{
    // TODO: use timeouts.
    return read (fd, data, len);
}

int try_magic (int fd, const char *magic)
{
    char reply[8];

    // Send magic.
    printf ("Sending magic: ");
    print_hex (magic, strlen(magic));
    printf ("\n");
    write (fd, magic, strlen(magic));

    // Check response.
    if (read_with_timeout (fd, reply, 1) != 1) {
        printf ("Radio did not respond.\n");
        return 0;
    }
    if (reply[0] != 0x06) {
        printf ("Bad response: %02x\n", reply[0]);
        return 0;
    }

    // Query for identifier..
    write (fd, "\x02", 1);
    if (read_with_timeout (fd, reply, 8) != 8) {
        printf ("Empty identifier.\n");
        return 0;
    }
    printf ("Identifier: ");
    print_hex (reply, 8);
    printf ("\n");

    // Enter clone mode.
    write (fd, "\x06", 1);
    if (read_with_timeout (fd, reply, 1) != 1) {
        printf ("Radio refused to clone.\n");
        return 0;
    }
    if (reply[0] != 0x06) {
        printf ("Radio refused to clone: %02x\n", reply[0]);
        return 0;
    }
    return 1;
}

//
// Identify the type of device connected.
//
void identify (int fd)
{
    static const char *UV5R_MODEL_ORIG = "\x50\xBB\xFF\x01\x25\x98\x4D";
    static const char *UV5R_MODEL_291 = "\x50\xBB\xFF\x20\x12\x07\x25";

    if (try_magic (fd, UV5R_MODEL_291)) {
        printf ("UV-5R firmware BFB291 detected.\n");
        return;
    }
    if (try_magic (fd, UV5R_MODEL_ORIG)) {
        printf ("Original UV-5R detected.\n");
        return;
    }
    printf ("Device not detected.\n");
    exit (-1);
}

int main (int argc, char **argv)
{
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
    if (argc != 1)
        usage ();

    // Open serial port.
    char *portname = argv[0];
    int fd = open_port (portname);

    identify (fd);

    // TODO

    close_port (fd);
    return (0);
}
