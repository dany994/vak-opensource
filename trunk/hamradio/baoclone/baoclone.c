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
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <sys/stat.h>

const char version[] = "1.0";
const char copyright[] = "Copyright (C) 2013 Serge Vakulenko KK6ABQ";

const unsigned char UV5R_MODEL_ORIG[] = "\x50\xBB\xFF\x01\x25\x98\x4D";
const unsigned char UV5R_MODEL_291[] = "\x50\xBB\xFF\x20\x12\x07\x25";

const int DCS_CODES[] = {
     23,  25,  26,  31,  32,  36,  43,  47,  51,  53,  54,
     65,  71,  72,  73,  74, 114, 115, 116, 122, 125, 131,
    132, 134, 143, 145, 152, 155, 156, 162, 165, 172, 174,
    205, 212, 223, 225, 226, 243, 244, 245, 246, 251, 252,
    255, 261, 263, 265, 266, 271, 274, 306, 311, 315, 325,
    331, 332, 343, 346, 351, 356, 364, 365, 371, 411, 412,
    413, 423, 431, 432, 445, 446, 452, 454, 455, 462, 464,
    465, 466, 503, 506, 516, 523, 526, 532, 546, 565, 606,
    612, 624, 627, 631, 632, 654, 662, 664, 703, 712, 723,
    731, 732, 734, 743, 754,
};

const char *PTTID_NAME[] = { "-", "Beg", "End", "Both" };

const char *STEP_NAME[] = { "2.5 ", "5.0 ", "6.25", "10.0",
                            "12.5", "25.0", "????", "????" };

const char *SAVER_NAME[] = { "Off", "1", "2", "3", "4", "?5?", "?6?", "?7?" };

const char *VOX_NAME[] = { "Off", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                           "10", "?11?", "?12?", "?13?", "?14?", "?15?" };

const char *ABR_NAME[] = { "Off", "1", "2", "3", "4", "5", "?6?", "?7?" };

const char *DTMF_SIDETONE_NAME[] = { "Off", "DTMF Only", "ANI Only", "DTMF+ANI" };

const char *SCAN_RESUME_NAME[] = { "After Timeout", "When Carrier Off", "Stop On Active", "??" };

const char *DISPLAY_MODE_NAME[] = { "Channel", "Name", "Frequency", "??" };

const char *COLOR_NAME[] = { "Off", "Blue", "Orange", "Purple" };

const char *ALARM_NAME[] = { "Site", "Tone", "Code", "??" };

const char *RPSTE_NAME[] = { "Off", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                             "10", "?11?", "?12?", "?13?", "?14?", "?15?" };

int verbose;

struct termios oldtio, newtio;  // Mode of serial port

unsigned char ident [8];        // Radio: identifier
unsigned char mem [0x2000];     // Radio: memory contents
unsigned char image_ident [8];  // Image file: identifier
int is_original;                // True for firmware older than 291
int progress;                   // Read/write progress counter

extern char *optarg;
extern int optind;

void usage ()
{
    fprintf (stderr, "Baofeng UV-5R Clone Utility, Version %s, %s\n", version, copyright);
    fprintf (stderr, "Usage:\n");
    fprintf (stderr, "    baoclone [-v] port                Save device binary image to file 'device.img',\n");
    fprintf (stderr, "                                      and text configuration to 'device.cfg'.\n");
    fprintf (stderr, "    baoclone -w [-v] port file.img    Write image to device.\n");
    fprintf (stderr, "    baoclone -c [-v] port file.cfg    Configure device from text file.\n");
    fprintf (stderr, "    baoclone file.img                 Show configuration from image file.\n");
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "    -v                                Verbose mode.\n");
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
    fprintf (stderr, "Close device.\n");

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
        printf ("# Sending magic: ");
        print_hex (magic, magic_len);
        printf ("\n");
    }
    tcflush (fd, TCIFLUSH);
    if (write (fd, magic, magic_len) != magic_len) {
        perror ("Serial port");
        exit (-1);
    }

    // Check response.
    if (read_with_timeout (fd, &reply, 1) != 1) {
        if (verbose)
            fprintf (stderr, "Radio did not respond.\n");
        return 0;
    }
    if (reply != 0x06) {
        fprintf (stderr, "Bad response: %02x\n", reply);
        return 0;
    }

    // Query for identifier..
    if (write (fd, "\x02", 1) != 1) {
        perror ("Serial port");
        exit (-1);
    }
    if (read_with_timeout (fd, ident, 8) != 8) {
        fprintf (stderr, "Empty identifier.\n");
        return 0;
    }
    if (verbose) {
        printf ("# Identifier: ");
        print_hex (ident, 8);
        printf ("\n");
    }

    // Enter clone mode.
    if (write (fd, "\x06", 1) != 1) {
        perror ("Serial port");
        exit (-1);
    }
    if (read_with_timeout (fd, &reply, 1) != 1) {
        fprintf (stderr, "Radio refused to clone.\n");
        return 0;
    }
    if (reply != 0x06) {
        fprintf (stderr, "Radio refused to clone: %02x\n", reply);
        return 0;
    }
    return 1;
}

void print_firmware_version(FILE *out)
{
    char buf[17], *version = buf, *p;

    if (is_original) {
        // No messages on old firmware.
        return;
    }

    // Copy the string, trim spaces.
    strncpy (version, (char*)&mem[0x1EC0+0x30], 16);
    version [16] = 0;
    while (*version == ' ')
        version++;
    p = version + strlen(version);
    while (p > version && p[-1]==' ')
        *--p = 0;

    fprintf (out, "Serial number: %.16s\n", &mem[0x1EC0+0x10]); // 6+poweron message
    fprintf (out, "Firmware: %s\n", version);                   // 3+poweron message
    fprintf (out, "Message: %.16s\n", &mem[0x1EC0+0x20]);       // poweron message
}

//
// Identify the type of device connected.
//
void identify (int fd)
{
    int retry;

    for (retry=0; retry<10; retry++) {
        if (try_magic (fd, UV5R_MODEL_291)) {
            is_original = 0;
            printf ("Detected Baofeng UV-5R.\n");
            return;
        }
        usleep (500000);
        if (try_magic (fd, UV5R_MODEL_ORIG)) {
            is_original = 1;
            printf ("Detected Baofeng UV-5R original.\n");
            return;
        }
        fprintf (stderr, "Retry #%d...\n", retry+1);
        usleep (500000);
    }
    fprintf (stderr, "Device not detected.\n");
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
    if (write (fd, cmd, 4) != 4) {
        perror ("Serial port");
        exit (-1);
    }

    // Read reply.
    if (read_with_timeout (fd, reply, 4) != 4) {
        fprintf (stderr, "Radio refused to send block 0x%04x.\n", start);
        exit(-1);
    }
    addr = reply[1] << 8 | reply[2];
    if (reply[0] != 'X' || addr != start || reply[3] != nbytes) {
        fprintf (stderr, "Bad reply for block 0x%04x of %d bytes: %02x-%02x-%02x-%02x\n",
            start, nbytes, reply[0], reply[1], reply[2], reply[3]);
        exit(-1);
    }

    // Read data.
    len = read_with_timeout (fd, data, 0x40);
    if (len != nbytes) {
        fprintf (stderr, "Reading block 0x%04x: got only %d bytes.\n", start, len);
        exit(-1);
    }

    // Get acknowledge.
    if (write (fd, "\x06", 1) != 1) {
        perror ("Serial port");
        exit (-1);
    }
    if (read_with_timeout (fd, reply, 1) != 1) {
        fprintf (stderr, "No acknowledge after block 0x%04x.\n", start);
        exit(-1);
    }
    if (reply[0] != 0x06) {
        fprintf (stderr, "Bad acknowledge after block 0x%04x: %02x\n", start, reply[0]);
        exit(-1);
    }
    if (verbose) {
        printf ("# Read 0x%04x: ", start);
        print_hex (data, nbytes);
        printf ("\n");
    } else {
        ++progress;
        if (progress % 2 == 0) {
            fprintf (stderr, "#");
            fflush (stderr);
        }
    }
}

//
// Write block of data, up to 16 bytes.
// Halt the program on any error.
//
void write_block (int fd, int start, const unsigned char *data, int nbytes)
{
    unsigned char cmd[4], reply;

    // Send command.
    cmd[0] = 'X';
    cmd[1] = start >> 8;
    cmd[2] = start;
    cmd[3] = nbytes;
    if (write (fd, cmd, 4) != 4) {
        perror ("Serial port");
        exit (-1);
    }
    if (write (fd, data, nbytes) != nbytes) {
        perror ("Serial port");
        exit (-1);
    }

    // Get acknowledge.
    if (read_with_timeout (fd, &reply, 1) != 1) {
        fprintf (stderr, "No acknowledge after block 0x%04x.\n", start);
        exit(-1);
    }
    if (reply != 0x06) {
        fprintf (stderr, "Bad acknowledge after block 0x%04x: %02x\n", start, reply);
        exit(-1);
    }

    if (verbose) {
        printf ("# Write 0x%04x: ", start);
        print_hex (data, nbytes);
        printf ("\n");
    } else {
        ++progress;
        if (progress % 8 == 0) {
            fprintf (stderr, "#");
            fflush (stderr);
        }
    }
}

void read_device (int fd)
{
    int addr;

    progress = 0;
    if (! verbose)
        fprintf (stderr, "Read device: ");

    // Main block.
    for (addr=0; addr<0x1800; addr+=0x40)
        read_block (fd, addr, &mem[addr], 0x40);

    if (! is_original) {
        // Auxiliary block starts at 0x1EC0.
        for (addr=0x1EC0; addr<0x2000; addr+=0x40)
            read_block (fd, addr, &mem[addr], 0x40);
    }

    if (! verbose)
        fprintf (stderr, " done.\n");

    // Copy device identifier to image identifier,
    // to allow writing it back to device.
    memcpy (image_ident, ident, sizeof(ident));
}

void write_device (int fd)
{
    int addr;

    // Check for compatibility.
    if (memcmp (image_ident, ident, sizeof(ident)) != 0) {
        fprintf (stderr, "Incompatible image - cannot upload.\n");
        exit(-1);
    }
    progress = 0;
    if (! verbose)
        fprintf (stderr, "Write device: ");

    // Main block.
    for (addr=0; addr<0x1800; addr+=0x10)
        write_block (fd, addr, &mem[addr], 0x10);

    if (! is_original) {
        // Auxiliary block starts at 0x1EC0.
        for (addr=0x1EC0; addr<0x2000; addr+=0x10)
            write_block (fd, addr, &mem[addr], 0x10);
    }

    if (! verbose)
        fprintf (stderr, " done.\n");
}

void load_image (char *filename)
{
    FILE *img;

    fprintf (stderr, "Read image from file '%s'.\n", filename);
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

    fprintf (stderr, "Write image to file '%s'.\n", filename);
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
    fprintf (stderr, "Read configuration from file '%s'.\n", filename);
}

int bcd_to_int (uint32_t bcd)
{
    return ((bcd >> 28) & 15) * 10000000 +
           ((bcd >> 24) & 15) * 1000000 +
           ((bcd >> 20) & 15) * 100000 +
           ((bcd >> 16) & 15) * 10000 +
           ((bcd >> 12) & 15) * 1000 +
           ((bcd >> 8)  & 15) * 100 +
           ((bcd >> 4)  & 15) * 10 +
           (bcd         & 15);
}

void decode_squelch (uint16_t index, int *ctcs, int *dcs)
{
    if (index == 0 || index == 0xffff) {
        // Squelch disabled.
        return;
    }
    if (index >= 0x0258) {
        // CTCSS value is Hz multiplied by 10.
        *ctcs = index;
    }
    // DCS mode.
    if (index < 0x6A)
        *dcs = DCS_CODES[index - 1];
    else
        *dcs = - DCS_CODES[index - 0x6A];
}

typedef struct {
    uint32_t    rxfreq; // binary coded decimal, 8 digits
    uint32_t    txfreq; // binary coded decimal, 8 digits
    uint16_t    rxtone;
    uint16_t    txtone;
    uint8_t     scode    : 4,
                _u1      : 4;
    uint8_t     _u2;
    uint8_t     lowpower : 1,
                _u3      : 7;
    uint8_t     pttidbot : 1,
                pttideot : 1,
                scan     : 1,
                bcl      : 1,
                _u5      : 2,
                wide     : 1,
                _u4      : 1;
} memory_channel_t;

void decode_channel (int i, char *name, int *rx_hz, int *tx_hz,
    int *rx_ctcs, int *tx_ctcs, int *rx_dcs, int *tx_dcs,
    int *lowpower, int *wide, int *scan, int *pttid, int *scode)
{
    memory_channel_t *ch = i + (memory_channel_t*) mem;

    *rx_hz = *tx_hz = *rx_ctcs = *tx_ctcs = *rx_dcs = *tx_dcs = 0;
    *name = 0;
    if (ch->rxfreq == 0 || ch->rxfreq == 0xffffffff)
        return;

    // Extract channel name; strip trailing FF's.
    char *p;
    strncpy (name, (char*) &mem[0x1000 + i*16], 7);
    name[7] = 0;
    for (p=name+6; p>=name && *p=='\xff'; p--)
        *p = 0;

    // Decode channel frequencies.
    *rx_hz = bcd_to_int (ch->rxfreq) * 10;
    *tx_hz = bcd_to_int (ch->txfreq) * 10;

    // Decode squelch modes.
    decode_squelch (ch->rxtone, rx_ctcs, rx_dcs);
    decode_squelch (ch->txtone, tx_ctcs, tx_dcs);

    // Other parameters.
    *lowpower = ch->lowpower;
    *wide = ch->wide;
    *scan = ch->scan;
    *scode = ch->scode;
    *pttid = ch->pttidbot | (ch->pttideot << 1);
}

typedef struct {
    uint8_t     enable;
    uint8_t     lower_msb; // binary coded decimal, 4 digits
    uint8_t     lower_lsb;
    uint8_t     upper_msb; // binary coded decimal, 4 digits
    uint8_t     upper_lsb;
} limits_t;

//
// Looks like limits are not implemented on old firmware
// (prior to version 291).
//
void decode_limits (char band, int *enable, int *lower, int *upper)
{
    int offset = (band == 'V') ? 0x1EC0+0x100 : 0x1EC0+0x105;

    if (is_original) {
        *enable = *lower = *upper = 0;
        return;
    }

    limits_t *limits = (limits_t*) (mem + offset);
    *enable = limits->enable;
    *lower = ((limits->lower_msb >> 4) & 15) * 1000 +
             (limits->lower_msb        & 15) * 100 +
             ((limits->lower_lsb >> 4) & 15) * 10 +
             (limits->lower_lsb        & 15);
    *upper = ((limits->upper_msb >> 4) & 15) * 1000 +
             (limits->upper_msb        & 15) * 100 +
             ((limits->upper_lsb >> 4) & 15) * 10 +
             (limits->upper_lsb        & 15);
}

void fetch_ani (char *ani)
{
    int i;

    for (i=0; i<5; i++)
        ani[i] = "0123456789ABCDEF" [mem[0x0CAA+i] & 0x0f];
}

void get_current_channel (int index, int *chan_num)
{
    unsigned char *ptr = mem + 0x0E76;
    *chan_num = ptr[index] % 128;
}

typedef struct {
    uint8_t     freq[8];    // binary coded decimal, 8 digits
    uint8_t     _u1;
    uint8_t     offset[4];  // binary coded decimal, 8 digits
    uint8_t     _u2;
    uint16_t    rxtone;
    uint16_t    txtone;
    uint8_t     band     : 1,
                _u3      : 7;
    uint8_t     _u4;
    uint8_t     scode    : 4,
                _u5      : 4;
    uint8_t     _u6;
    uint8_t     _u7      : 4,
                step     : 3,
                _u8      : 1;
    uint8_t     _u9      : 6,
                narrow   : 1,
                lowpower : 1;
} vfo_t;

void decode_vfo (int index, int *band, int *hz, int *offset,
    int *rx_ctcs, int *tx_ctcs, int *rx_dcs, int *tx_dcs,
    int *lowpower, int *wide, int *step, int *scode)
{
    vfo_t *vfo = (vfo_t*) &mem[index ? 0x0F28 : 0x0F08];

    *band = *hz = *offset = *rx_ctcs = *tx_ctcs = *rx_dcs = *tx_dcs = 0;
    *lowpower = *wide = *step = *scode = 0;

    *band = vfo->band;
    *hz = (vfo->freq[0] & 15) * 100000000 +
          (vfo->freq[1] & 15) * 10000000 +
          (vfo->freq[2] & 15) * 1000000 +
          (vfo->freq[3] & 15) * 100000 +
          (vfo->freq[4] & 15) * 10000 +
          (vfo->freq[5] & 15) * 1000 +
          (vfo->freq[6] & 15) * 100 +
          (vfo->freq[7] & 15);
    *offset = (vfo->offset[0] & 15) * 100000000 +
              (vfo->offset[1] & 15) * 10000000 +
              (vfo->offset[2] & 15) * 1000000 +
              (vfo->offset[3] & 15) * 100000;
    decode_squelch (vfo->rxtone, rx_ctcs, rx_dcs);
    decode_squelch (vfo->txtone, tx_ctcs, tx_dcs);
    *lowpower = vfo->lowpower;
    *wide = ! vfo->narrow;
    *step = vfo->step;
    *scode = vfo->scode;
}

void print_offset (FILE *out, int delta)
{
    if (delta == 0) {
        fprintf (out, "0       ");
    } else {
        if (delta > 0) {
            fprintf (out, "+");;
        } else {
            fprintf (out, "-");;
            delta = - delta;
        }
        if (delta % 1000000 == 0)
            fprintf (out, "%-7u", delta / 1000000);
        else
            fprintf (out, "%-7.3f", delta / 1000000.0);
    }
}

void print_squelch (FILE *out, int ctcs, int dcs)
{
    if      (ctcs)    fprintf (out, "%5.1f", ctcs / 10.0);
    else if (dcs > 0) fprintf (out, "D%03dN", dcs);
    else if (dcs < 0) fprintf (out, "D%03dI", -dcs);
    else              fprintf (out, "   - ");
}

void print_vfo (FILE *out, char name, int band, int hz, int offset,
    int rx_ctcs, int tx_ctcs, int rx_dcs, int tx_dcs,
    int lowpower, int wide, int step, int scode)
{
    fprintf (out, " %c  %3s  %8.4f ", name, band ? "UHF" : "VHF", hz / 1000000.0);
    print_offset (out, offset);
    fprintf (out, " ");
    print_squelch (out, rx_ctcs, rx_dcs);
    fprintf (out, "   ");
    print_squelch (out, tx_ctcs, tx_dcs);

    char sgroup [8];
    if (scode == 0)
        strcpy (sgroup, "-");
    else
        sprintf (sgroup, "%u", scode);

    fprintf (out, "   %-4s %-4s  %-6s %-3s\n", STEP_NAME[step],
        lowpower ? "Low" : "High", wide ? "Wide" : "Narrow", sgroup);
}

//
// Generic settings.
//
typedef struct {
    uint8_t squelch;    // Carrier Squelch Level
    uint8_t step;
    uint8_t _u1;
    uint8_t save;       // Battery Saver
    uint8_t vox;        // VOX Sensitivity
    uint8_t _u2;
    uint8_t abr;        // Backlight Timeout
    uint8_t tdr;        // Dual Watch
    uint8_t beep;       // Beep
    uint8_t timeout;    // Timeout Timer
    uint8_t _u3 [4];
    uint8_t voice;      // Voice
    uint8_t _u4;
    uint8_t dtmfst;     // DTMF Sidetone
    uint8_t _u5;
    uint8_t screv;      // Scan Resume
    uint8_t pttid;
    uint8_t pttlt;
    uint8_t mdfa;       // Display Mode (A)
    uint8_t mdfb;       // Display Mode (B)
    uint8_t bcl;        // Busy Channel Lockout
    uint8_t autolk;     // Automatic Key Lock
    uint8_t sftd;
    uint8_t _u6 [3];
    uint8_t wtled;      // Standby LED Color
    uint8_t rxled;      // RX LED Color
    uint8_t txled;      // TX LED Color
    uint8_t almod;      // Alarm Mode
    uint8_t band;
    uint8_t tdrab;      // Dual Watch Priority
    uint8_t ste;        // Squelch Tail Eliminate (HT to HT)
    uint8_t rpste;      // Squelch Tail Eliminate (repeater)
    uint8_t rptrl;      // STE Repeater Delay
    uint8_t ponmsg;     // Power-On Message
    uint8_t roger;      // Roger Beep
} settings_t;

//
// Transient modes.
//
typedef struct {
    uint8_t displayab  : 1,     // Display
            _u1        : 2,
            fmradio    : 1,     // Broadcast FM Radio
            alarm      : 1,
            _u2        : 1,
            reset      : 1,     // RESET Menu
            menu       : 1;     // All Menus
    uint8_t _u3;
    uint8_t workmode;           // VFO/MR Mode
    uint8_t keylock;            // Keypad Lock
} extra_settings_t;

void print_config (FILE *out)
{
    int i;

    // Print memory channels.
    fprintf (out, "\n");
    fprintf (out, "Channel Name    Receive  TxOffset R-Squel T-Squel Power FM     Scan ID[6] PTTID\n");
    for (i=0; i<128; i++) {
        int rx_hz, tx_hz, rx_ctcs, tx_ctcs, rx_dcs, tx_dcs;
        int lowpower, wide, scan, pttid, scode;
        char name[17];

        decode_channel (i, name, &rx_hz, &tx_hz, &rx_ctcs, &tx_ctcs,
            &rx_dcs, &tx_dcs, &lowpower, &wide, &scan, &pttid, &scode);
        if (rx_hz == 0) {
            // Channel is disabled
            continue;
        }

        fprintf (out, "%5d   %-7s %8.4f ", i, name, rx_hz / 1000000.0);
        print_offset (out, tx_hz - rx_hz);
        fprintf (out, " ");
        print_squelch (out, rx_ctcs, rx_dcs);
        fprintf (out, "   ");
        print_squelch (out, tx_ctcs, tx_dcs);

        char sgroup [8];
        if (scode == 0)
            strcpy (sgroup, "-");
        else
            sprintf (sgroup, "%u", scode);

        fprintf (out, "   %-4s  %-6s %-4s %-5s %-4s\n", lowpower ? "Low" : "High",
            wide ? "Wide" : "Narrow", scan ? "+" : "-", sgroup, PTTID_NAME[pttid]);
    }

    // Print frequency mode VFO settings.
    int band, hz, offset, rx_ctcs, tx_ctcs, rx_dcs, tx_dcs;
    int lowpower, wide, step, scode;
    fprintf (out, "\n");
    decode_vfo (0, &band, &hz, &offset, &rx_ctcs, &tx_ctcs,
        &rx_dcs, &tx_dcs, &lowpower, &wide, &step, &scode);
    fprintf (out, "VFO Band Receive  TxOffset R-Squel T-Squel Step Power FM     ID[6]\n");
    print_vfo (out, 'A', band, hz, offset, rx_ctcs, tx_ctcs,
        rx_dcs, tx_dcs, lowpower, wide, step, scode);
    decode_vfo (1, &band, &hz, &offset, &rx_ctcs, &tx_ctcs,
        &rx_dcs, &tx_dcs, &lowpower, &wide, &step, &scode);
    print_vfo (out, 'B', band, hz, offset, rx_ctcs, tx_ctcs,
        rx_dcs, tx_dcs, lowpower, wide, step, scode);

    if (! is_original) {
        // Print band limits.
        int vhf_enable, vhf_lower, vhf_upper, uhf_enable, uhf_lower, uhf_upper;
        decode_limits ('V', &vhf_enable, &vhf_lower, &vhf_upper);
        decode_limits ('U', &uhf_enable, &uhf_lower, &uhf_upper);
        fprintf (out, "\n");
        fprintf (out, "Limit Lower Upper Enable\n");
        fprintf (out, " VHF  %4d  %4d  %s\n", vhf_lower, vhf_upper,
            vhf_enable ? "+" : "-");
        fprintf (out, " UHF  %4d  %4d  %s\n", uhf_lower, uhf_upper,
            uhf_enable ? "+" : "-");
    }

    // Print channel mode settings.
    int chan_a, chan_b;
    get_current_channel (0, &chan_a);
    get_current_channel (1, &chan_b);
    fprintf (out, "\n");
    fprintf (out, "Channel A: %d\n", chan_a);
    fprintf (out, "Channel B: %d\n", chan_b);

    // Get atomatic number identifier.
    char ani[5];
    fetch_ani (ani);

    // Print other settings.
    settings_t *mode = (settings_t*) &mem[0x0E20];
    fprintf (out, "Carrier Squelch Level: %u\n", mode->squelch);
    fprintf (out, "Battery Saver: %s\n", SAVER_NAME[mode->save & 7]);
    fprintf (out, "VOX Sensitivity: %s\n", VOX_NAME[mode->vox & 15]);
    fprintf (out, "Backlight Timeout: %s\n", ABR_NAME[mode->abr & 7]);
    fprintf (out, "Dual Watch: %s\n", mode->tdr ? "On" : "Off");
    fprintf (out, "Keypad Beep: %s\n", mode->beep ? "On" : "Off");
    fprintf (out, "Transmittion Timer: %u\n", (mode->timeout + 1) * 15);
    fprintf (out, "Voice Prompt: %s\n", mode->voice ? "On" : "Off");
    fprintf (out, "Automatic ID[1-5]: %c%c%c%c%c\n", ani[0], ani[1], ani[2], ani[3], ani[4]);
    fprintf (out, "DTMF Sidetone: %s\n", DTMF_SIDETONE_NAME[mode->dtmfst & 3]);
    fprintf (out, "Scan Resume Method: %s\n", SCAN_RESUME_NAME[mode->screv & 3]);
    fprintf (out, "Display Mode A: %s\n", DISPLAY_MODE_NAME[mode->mdfa & 3]);
    fprintf (out, "Display Mode B: %s\n", DISPLAY_MODE_NAME[mode->mdfb & 3]);
    fprintf (out, "Busy Channel Lockout: %s\n", mode->bcl ? "On" : "Off");
    fprintf (out, "Auto Key Lock: %s\n", mode->autolk ? "On" : "Off");
    fprintf (out, "Standby LED Color: %s\n", COLOR_NAME[mode->wtled & 3]);
    fprintf (out, "RX LED Color: %s\n", COLOR_NAME[mode->rxled & 3]);
    fprintf (out, "TX LED Color: %s\n", COLOR_NAME[mode->txled & 3]);
    fprintf (out, "Alarm Mode: %s\n", ALARM_NAME[mode->almod & 3]);
    fprintf (out, "Squelch Tail Eliminate: %s\n", mode->ste ? "On" : "Off");
    fprintf (out, "Squelch Tail Eliminate for Repeater: %s\n", RPSTE_NAME[mode->rpste & 15]);
    fprintf (out, "Squelch Tail Repeater Delay: %s\n", RPSTE_NAME[mode->rptrl & 15]);
    fprintf (out, "Power-On Message: %s\n", mode->ponmsg ? "On" : "Off");
    fprintf (out, "Roger Beep: %s\n", mode->roger ? "On" : "Off");

#if 0
    // Transient modes: usually there is no interest to display or touch these.
    extra_settings_t *extra = (extra_settings_t*) &mem[0x0E4A];
    extra->displayab;
    extra->fmradio;
    extra->alarm;
    extra->reset;
    extra->menu;
    extra->workmode;
    extra->keylock;
#endif
}

int main (int argc, char **argv)
{
    int write_flag = 0, config_flag = 0;

    for (;;) {
        switch (getopt (argc, argv, "vcw")) {
        case 'v': ++verbose;     continue;
        case 'w': ++write_flag;  continue;
        case 'c': ++config_flag; continue;
        default:
            usage ();
        case EOF:
            break;
        }
        break;
    }
    argc -= optind;
    argv += optind;
    if (write_flag + config_flag > 1) {
        fprintf (stderr, "Only one of -w or -c options is allowed.\n");
        usage();
    }

    if (write_flag) {
        // Restore image file to device.
        if (argc != 2)
            usage();

        int fd = open_port (argv[0]);
        identify (fd);
        load_image (argv[1]);
        print_firmware_version(stdout);
        write_device (fd);
        close_port (fd);

    } else if (config_flag) {
        // Update device from text config file.
        if (argc != 2)
            usage();

        int fd = open_port (argv[0]);
        identify (fd);
        read_device (fd);
        print_firmware_version(stdout);
        save_image ("save.img");
        read_config (argv[1]);
        write_device (fd);
        close_port (fd);

    } else {
        if (argc != 1)
            usage();

        if (is_file (argv[0])) {
            // Print configuration from image file.
            // Load image from file.
            load_image (argv[0]);
            print_firmware_version(stdout);
            memcpy (ident, image_ident, sizeof(ident));
            print_config (stdout);
        } else {
            // Dump device to image file.
            int fd = open_port (argv[0]);
            identify (fd);
            read_device (fd);
            print_firmware_version(stdout);
            close_port (fd);
            save_image ("device.img");

            // Print configuration to file.
            const char *filename = "device.cfg";
            FILE *cfg = fopen (filename, "w");
            if (! cfg) {
                perror (filename);
                exit (-1);
            }
            print_firmware_version(cfg);
            print_config (cfg);
            fclose (cfg);
            printf ("Print configuration to file '%s'.\n", filename);
        }
    }
    return (0);
}
