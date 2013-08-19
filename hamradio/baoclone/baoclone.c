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
#include <sys/stat.h>

const char version[] = "1.0";
const char copyright[] = "Copyright (C) 2013 Serge Vakulenko";

const unsigned char UV5R_MODEL_ORIG[] = "\x50\xBB\xFF\x01\x25\x98\x4D";
const unsigned char UV5R_MODEL_291[] = "\x50\xBB\xFF\x20\x12\x07\x25";

char *progname;
int verbose;

struct termios oldtio, newtio;  // Mode of serial port

unsigned char ident [8];        // Radio: identifier
unsigned char mem [0x2000];     // Radio: memory contents
unsigned char image_ident [8];  // Image file: identifier
int progress;                   // Read/write progress counter

extern char *optarg;
extern int optind;

void usage ()
{
    fprintf (stderr, "Baofeng UV-5R Clone Utility, Version %s, %s\n", version, copyright);
    fprintf (stderr, "Usage:\n");
    fprintf (stderr, "    %s [option]...\n", progname);
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "    -d device file.img  dump device image to file\n");
    fprintf (stderr, "    -r device file.img  restore device image from file\n");
    fprintf (stderr, "    -c device file.cfg  configure device from text file\n");
    fprintf (stderr, "    -s device           show device configuration\n");
    fprintf (stderr, "    -s file.img         show configuration from image file\n");
    fprintf (stderr, "    -v                  verbose mode\n");
    exit (-1);
}

//
// Check for a regular file.
//
int is_file (char *filename)
{
    struct stat st;

    if (stat (filename, &st) < 0) {
        // File not exist: treat it as a regular file.
        return 1;
    }
    return (st.st_mode & S_IFMT) == S_IFREG;
}

//
// Print data in hex format.
//
void print_hex (const unsigned char *data, int len)
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

    // Use non-block flag to ignore carrier (DCD).
    fd = open (portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror (portname);
        exit (-1);
    }

    // Get terminal modes.
    tcgetattr (fd, &oldtio);
    newtio = oldtio;

    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;              // 8 data bits
    newtio.c_cflag |= CLOCAL | CREAD;   // enable receiver, set local mode
    newtio.c_cflag &= ~PARENB;          // no parity
    newtio.c_cflag &= ~CSTOPB;          // 1 stop bit
    newtio.c_cflag &= ~CRTSCTS;         // no h/w handshake
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // raw input
    newtio.c_oflag &= ~OPOST;           // raw output
    newtio.c_iflag &= ~IXON;            // software flow control disabled
    newtio.c_iflag &= ~ICRNL;           // do not translate CR to NL

    cfsetispeed(&newtio, B9600);        // Set baud rate.
    cfsetospeed(&newtio, B9600);

    // Set terminal modes.
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

    // Flush received data pending on the port.
    tcflush (fd, TCIFLUSH);
    return fd;
}

//
// Close the serial port.
//
void close_port (int fd)
{
    printf ("Close device.\n");

    // Restore the port mode.
    tcsetattr (fd, TCSANOW, &oldtio);
    close (fd);

    // Radio needs a timeout to reset to a normal state.
    usleep (2000000);
}

//
// Read data from serial port.
// Return 0 when no data available.
// Use 200-msec timeout.
//
int read_with_timeout (int fd, unsigned char *data, int len)
{
    fd_set rset, wset, xset;
    int nbytes, len0 = len;

    for (;;) {
        // Initialize file descriptor sets.
        FD_ZERO (&rset);
        FD_ZERO (&wset);
        FD_ZERO (&xset);
        FD_SET (fd, &rset);

        // Set timeout to 100 msec.
        struct timeval timo;
        timo.tv_sec = 0;
        timo.tv_usec = 200000;

        // Wait for input to become ready or until the time out.
        if (select (fd + 1, &rset, &wset, &xset, &timo) != 1)
            return 0;

        nbytes = read (fd, (unsigned char*) data, len);
        if (nbytes <= 0)
            return 0;

        len -= nbytes;
        if (len <= 0)
            return len0;

        data += nbytes;
    }
}

//
// Try to identify the device with a given magic command.
// Return 0 when failed.
//
int try_magic (int fd, const unsigned char *magic)
{
    unsigned char reply;
    int magic_len = strlen ((char*) magic);

    // Send magic.
    if (verbose) {
        printf ("Sending magic: ");
        print_hex (magic, magic_len);
        printf ("\n");
    }
    tcflush (fd, TCIFLUSH);
    write (fd, magic, magic_len);

    // Check response.
    if (read_with_timeout (fd, &reply, 1) != 1) {
        if (verbose)
            printf ("Radio did not respond.\n");
        return 0;
    }
    if (reply != 0x06) {
        printf ("Bad response: %02x\n", reply);
        return 0;
    }

    // Query for identifier..
    write (fd, "\x02", 1);
    if (read_with_timeout (fd, ident, 8) != 8) {
        printf ("Empty identifier.\n");
        return 0;
    }
    if (verbose) {
        printf ("Identifier: ");
        print_hex (ident, 8);
        printf ("\n");
    }

    // Enter clone mode.
    write (fd, "\x06", 1);
    if (read_with_timeout (fd, &reply, 1) != 1) {
        printf ("Radio refused to clone.\n");
        return 0;
    }
    if (reply != 0x06) {
        printf ("Radio refused to clone: %02x\n", reply);
        return 0;
    }
    return 1;
}

void print_firmware_version()
{
    char buf[17], *version = buf, *p;

    // Copy the string, trim spaces.
    strncpy (version, (char*)&mem[0x1EF0], 16);
    version [16] = 0;
    while (*version == ' ')
        version++;
    p = version + strlen(version);
    while (p > version && p[-1]==' ')
        *--p = 0;

    printf ("Firmware: '%s'.\n", version);
}

//
// Identify the type of device connected.
//
void identify (int fd)
{
    int retry;

    for (retry=0; retry<10; retry++) {
        if (try_magic (fd, UV5R_MODEL_291)) {
            printf ("Detected Baofeng UV-5R.\n");
            return;
        }
        usleep (500000);
        if (try_magic (fd, UV5R_MODEL_ORIG)) {
            printf ("Detected Baofeng UV-5R original.\n");
            return;
        }
        printf ("Retry #%d...\n", retry+1);
        usleep (500000);
    }
    printf ("Device not detected.\n");
    exit (-1);
}

//
// Read block of data, up to 64 bytes.
// Halt the program on any error.
//
void read_block (int fd, int start, unsigned char *data, int nbytes)
{
    unsigned char cmd[4], reply[4];
    int addr, len;

    // Send command.
    cmd[0] = 'S';
    cmd[1] = start >> 8;
    cmd[2] = start;
    cmd[3] = nbytes;
    write (fd, cmd, 4);

    // Read reply.
    if (read_with_timeout (fd, reply, 4) != 4) {
        printf ("Radio refused to send block 0x%04x.\n", start);
        exit(-1);
    }
    addr = reply[1] << 8 | reply[2];
    if (reply[0] != 'X' || addr != start || reply[3] != nbytes) {
        printf ("Bad reply for block 0x%04x of %d bytes: %02x-%02x-%02x-%02x\n",
            start, nbytes, reply[0], reply[1], reply[2], reply[3]);
        exit(-1);
    }

    // Read data.
    len = read_with_timeout (fd, data, 0x40);
    if (len != nbytes) {
        printf ("Reading block 0x%04x: got only %d bytes.\n", start, len);
        exit(-1);
    }

    // Get acknowledge.
    write (fd, "\x06", 1);
    if (read_with_timeout (fd, reply, 1) != 1) {
        printf ("No acknowledge after block 0x%04x.\n", start);
        exit(-1);
    }
    if (reply[0] != 0x06) {
        printf ("Bad acknowledge after block 0x%04x: %02x\n", start, reply[0]);
        exit(-1);
    }
    if (verbose) {
        printf ("0x%04x: ", start);
        print_hex (data, nbytes);
        printf ("\n");
    } else {
        ++progress;
        if (progress % 2 == 0) {
            printf ("#");
            fflush (stdout);
        }
    }
}

void read_device (int fd)
{
    int addr;

    progress = 0;
    if (! verbose)
        printf ("Read device: ");

    // Main block.
    for (addr=0; addr<0x1800; addr+=0x40)
        read_block (fd, addr, &mem[addr], 0x40);

    // Auxiliary block starts at 0x1EC0.
    for (addr=0x1EC0; addr<0x2000; addr+=0x40)
        read_block (fd, addr, &mem[addr], 0x40);

    if (! verbose)
        printf (" done.\n");
}

void write_device (int fd)
{
    // TODO
    printf ("Write to device: NOT IMPLEMENTED YET.\n");
}

void load_image (char *filename)
{
    FILE *img;

    printf ("Read image from file '%s'.\n", filename);
    img = fopen (filename, "r");
    if (! img) {
        perror (filename);
        exit (-1);
    }
    if (fread (image_ident, 1, 8, img) != 8) {
        fprintf (stderr, "Error reading image header.\n");
        exit (-1);
    }
    if (fread (&mem[0], 1, 0x1800, img) != 0x1800) {
        fprintf (stderr, "Error reading image data.\n");
        exit (-1);
    }
    if (fread (&mem[0x1EC0], 1, 0x2000-0x1EC0, img) != 0x2000-0x1EC0) {
        fprintf (stderr, "Error reading image footer.\n");
        exit (-1);
    }
    fclose (img);
}

void save_image (char *filename)
{
    FILE *img;

    printf ("Write image to file '%s'.\n", filename);
    img = fopen (filename, "w");
    if (! img) {
        perror (filename);
        exit (-1);
    }
    fwrite (ident, 1, 8, img);
    fwrite (&mem[0], 1, 0x1800, img);
    fwrite (&mem[0x1EC0], 1, 0x2000-0x1EC0, img);
    fclose (img);
}

void read_config (char *filename)
{
    // TODO
    printf ("Read configuration from file '%s'.\n", filename);
}

void print_config ()
{
    // TODO
    printf ("Print configuration: NOT IMPLEMENTED YET.\n");
}

int main (int argc, char **argv)
{
    int dump_flag = 0, restore_flag = 0;
    int config_flag = 0, show_flag = 0;

    progname = *argv;
    for (;;) {
        switch (getopt (argc, argv, "vdrcs")) {
        case 'v': ++verbose;      continue;
        case 'd': ++dump_flag;    continue;
        case 'r': ++restore_flag; continue;
        case 'c': ++config_flag;  continue;
        case 's': ++show_flag;    continue;
        default:
            usage ();
        case EOF:
            break;
        }
        break;
    }
    argc -= optind;
    argv += optind;
    if (dump_flag + restore_flag + config_flag + show_flag == 0)
        usage();
    if (dump_flag + restore_flag + config_flag + show_flag > 1) {
        printf ("Only one of -d -r -c -s options is allowed.\n");
        usage();
    }

    if (dump_flag) {
        // Dump device to image file.
        if (argc != 2)
            usage();

        int fd = open_port (argv[0]);
        identify (fd);
        read_device (fd);
        print_firmware_version();
        save_image (argv[1]);
        close_port (fd);

    } else if (restore_flag) {
        // Restore image file to device.
        if (argc != 2)
            usage();

        int fd = open_port (argv[0]);
        identify (fd);
        load_image (argv[1]);
        print_firmware_version();
        write_device (fd);
        close_port (fd);

    } else if (config_flag) {
        // Update device from text config file.
        if (argc != 2)
            usage();

        int fd = open_port (argv[0]);
        identify (fd);
        read_device (fd);
        print_firmware_version();
        save_image ("save.img");
        read_config (argv[1]);
        write_device (fd);
        close_port (fd);

    } else if (show_flag) {
        // Print device or image configuration in readable format.
        if (argc != 1)
            usage();

        if (is_file (argv[0])) {
            // Load image from file.
            load_image (argv[0]);
            print_firmware_version();
            memcpy (ident, image_ident, sizeof(ident));
        } else {
            // Use real device.
            int fd = open_port (argv[0]);
            identify (fd);
            read_device (fd);
            print_firmware_version();
            close_port (fd);
        }
        print_config();
    }
    return (0);
}
