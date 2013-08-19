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
#include <stdint.h>
#include <sys/stat.h>

const char version[] = "1.0";
const char copyright[] = "Copyright (C) 2013 Serge Vakulenko";

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

    printf ("Device: %.16s\n", &mem[0x1EE0]);
    printf ("Serial number: %.16s\n", &mem[0x1ED0]);
    printf ("Firmware: '%s'\n", version);
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
#if 0
    _ident_radio(radio)

    image_version = _firmware_version_from_image(radio)
    radio_version = _get_radio_firmware_version(radio)
    print "Image is %s" % repr(image_version)
    print "Radio is %s" % repr(radio_version)

    if "BFB" not in radio_version:
        raise errors.RadioError("Unsupported firmware version: `%s'" %
                                radio_version)

    # Main block
    for i in range(0x08, 0x1808, 0x10):
        _send_block(radio, i - 0x08, radio.get_mmap()[i:i+0x10])
        _do_status(radio, i)

    if len(radio.get_mmap().get_packed()) == 0x1808:
        print "Old image, not writing aux block"
        return # Old image, no aux block

    if image_version != radio_version:
        raise errors.RadioError("Upload finished, but the 'Other Settings' "
                                "could not be sent because the firmware "
                                "version of the image does not match that "
                                "of the radio")

    # Auxiliary block at radio address 0x1EC0, our offset 0x1808
    for i in range(0x1EC0, 0x2000, 0x10):
        addr = 0x1808 + (i - 0x1EC0)
        _send_block(radio, i, radio.get_mmap()[addr:addr+0x10])
#endif
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

int bcd_to_hz (uint32_t bcd)
{
    return ((bcd >> 28) & 15) * 100000000 +
           ((bcd >> 24) & 15) * 10000000 +
           ((bcd >> 20) & 15) * 1000000 +
           ((bcd >> 16) & 15) * 100000 +
           ((bcd >> 12) & 15) * 10000 +
           ((bcd >> 8)  & 15) * 1000 +
           ((bcd >> 4)  & 15) * 100 +
           (bcd         & 15) * 10;
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
    int *lowpower, int *wide, int *bcl, int *scan, int *pttid, int *scode)
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
    *rx_hz = bcd_to_hz (ch->rxfreq);
    *tx_hz = bcd_to_hz (ch->txfreq);

    // Decode squelch modes.
    decode_squelch (ch->rxtone, rx_ctcs, rx_dcs);
    decode_squelch (ch->txtone, tx_ctcs, tx_dcs);

    // Other parameters.
    *lowpower = ch->lowpower;
    *wide = ch->wide;
    *bcl = ch->bcl;
    *scan = ch->scan;
    *scode = ch->scode;
    *pttid = ch->pttidbot | (ch->pttideot << 1);
}

void print_config ()
{
    int i;

    // Print memory channels.
    printf ("Chan  Name    Receive  TxOffset R-Squel T-Squel Power FM     Scan Scode BCL PTTID\n");
    for (i=0; i<128; i++) {
        int rx_hz, tx_hz, rx_ctcs, tx_ctcs, rx_dcs, tx_dcs;
        int lowpower, wide, busy_channel_lockout, scan, pttid, scode;
        char name[17];

        decode_channel (i, name, &rx_hz, &tx_hz, &rx_ctcs, &tx_ctcs,
            &rx_dcs, &tx_dcs, &lowpower, &wide, &busy_channel_lockout,
            &scan, &pttid, &scode);
        if (rx_hz == 0) {
            // Channel is disabled
            return;
        }
        char offset[16];
        if (tx_hz == rx_hz) {
            strcpy (offset, "0");
        } else {
            int delta = tx_hz - rx_hz;
            offset[0] = '+';
            if (delta < 0) {
                delta = - delta;
                offset[0] = '-';
            }
            if (delta % 1000000 == 0)
                sprintf (offset+1, "%u", delta / 1000000);
            else
                sprintf (offset+1, "%.3f", delta / 1000000.0);
        }

        printf ("%4d  %-7s %8.4f %-8s", i, name,
            rx_hz / 1000000.0, offset);

        if      (rx_ctcs)    printf (" %5.1f", rx_ctcs / 10.0);
        else if (rx_dcs > 0) printf (" D%03dN", rx_dcs);
        else if (rx_dcs < 0) printf (" D%03dI", -rx_dcs);
        else                 printf ("    - ");

        if      (tx_ctcs)    printf ("   %5.1f", tx_ctcs / 10.0);
        else if (tx_dcs > 0) printf ("   D%03dN", tx_dcs);
        else if (tx_dcs < 0) printf ("   D%03dI", -tx_dcs);
        else                 printf ("      - ");

        char sgroup [8];
        if (scode == 0)
            strcpy (sgroup, "-");
        else
            sprintf (sgroup, "%u", scode);

        printf ("   %-4s  %-6s %-4s %-5s %-3s %-4s\n", lowpower ? "Low" : "High",
            wide ? "Wide" : "Narrow", scan ? "+" : "-", sgroup,
            busy_channel_lockout ? "+" : "-", PTTID_NAME[pttid]);
    }

    // TODO: ani, settings, extra, wmchannel, vfoa, vfob,
    // poweron_msg, sixpoweron_msg, limits
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
