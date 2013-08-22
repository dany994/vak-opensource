/*
 * Clone Utility for Baofeng radios.
 *
 * Copyright (C) 2013 Serge Vakulenko, KK6ABQ
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. The name of the author may not be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include "radio.h"
#include "util.h"

int radio_port;                         // File descriptor of programming serial port
unsigned char radio_ident [8];          // Radio: identifier
unsigned char radio_mem [0x2000];       // Radio: memory contents
int radio_progress;                     // Read/write progress counter

static radio_device_t *device;          // Device-dependent interface
static struct termios oldtio, newtio;   // Mode of serial port
static unsigned char image_ident [8];   // Image file: identifier

//
// Open the serial port.
//
static int open_port (char *portname)
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
void radio_disconnect()
{
    fprintf (stderr, "Close device.\n");

    // Restore the port mode.
    tcsetattr (radio_port, TCSANOW, &oldtio);
    close (radio_port);
    radio_port = -1;

    // Radio needs a timeout to reset to a normal state.
    usleep (2000000);
}

//
// Print a generic information about the device.
//
void radio_print_version (FILE *out)
{
    fprintf (out, "Type: %s\n", device->name);
    device->print_version (out);
}

//
// Connect to the radio and identify the type of device.
//
void radio_connect (char *port_name)
{
    int retry;

    radio_port = open_port (port_name);
    for (retry=0; retry<10; retry++) {
        device = &radio_uv5r;
        if (device->connect()) {
            printf ("Detected %s.\n", device->name);
            return;
        }
        usleep (500000);
        device = &radio_uv5r_aged;
        if (device->connect()) {
            printf ("Detected %s.\n", device->name);
            return;
        }
        fprintf (stderr, "Retry #%d...\n", retry+1);
        usleep (500000);
    }
    fprintf (stderr, "Device not detected.\n");
    exit (-1);
}

//
// Read firmware image from the device.
//
void radio_download()
{
    radio_progress = 0;
    if (! verbose)
        fprintf (stderr, "Read device: ");

    device->download();

    if (! verbose)
        fprintf (stderr, " done.\n");

    // Copy device identifier to image identifier,
    // to allow writing it back to device.
    memcpy (image_ident, radio_ident, sizeof(radio_ident));
}

//
// Write firmware image to the device.
//
void radio_upload()
{
    // Check for compatibility.
    if (memcmp (image_ident, radio_ident, sizeof(radio_ident)) != 0) {
        fprintf (stderr, "Incompatible image - cannot upload.\n");
        exit(-1);
    }
    radio_progress = 0;
    if (! verbose)
        fprintf (stderr, "Write device: ");

    device->upload();

    if (! verbose)
        fprintf (stderr, " done.\n");
}

//
// Read firmware image from the binary file.
//
void radio_read_image (char *filename)
{
    FILE *img;
    struct stat st;

    fprintf (stderr, "Read image from file '%s'.\n", filename);

    // Guess device type by file size.
    if (stat (filename, &st) < 0) {
        perror (filename);
        exit (-1);
    }
    switch (st.st_size) {
    case 6472:
        device = &radio_uv5r;
        break;
    case 6152:
        device = &radio_uv5r_aged;
        break;
#if 0
    // TODO
    case 4144:
        device = &radio_uvb5;
        break;
    case ????:
        device = &radio_bf888s;
        break;
#endif
    default:
        fprintf (stderr, "%s: Unrecognized file size %u bytes.\n",
            filename, (int) st.st_size);
        exit (-1);
    }

    img = fopen (filename, "r");
    if (! img) {
        perror (filename);
        exit (-1);
    }
    device->read_image (img, image_ident);
    fclose (img);
}

//
// Save firmware image to the binary file.
//
void radio_save_image (char *filename)
{
    FILE *img;

    fprintf (stderr, "Write image to file '%s'.\n", filename);
    img = fopen (filename, "w");
    if (! img) {
        perror (filename);
        exit (-1);
    }
    device->save_image (img);
    fclose (img);
}

//
// Read the configuration from text file, and modify the firmware.
//
void radio_parse_config (char *filename)
{
    // TODO
    fprintf (stderr, "Read configuration from file '%s'.\n", filename);
}

//
// Print full information about the device configuration.
//
void radio_print_config (FILE *out)
{
    device->print_config (out);
}
