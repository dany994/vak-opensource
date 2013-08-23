/*
 * Interface to Baofeng BF-888S and compatibles.
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
#include <stdint.h>
#include "radio.h"
#include "util.h"

#define NCHAN 16

static const char *SIDEKEY_NAME[] = { "Off", "Monitor", "TX Power", "Alarm" };

//
// Print a generic information about the device.
//
static void bf888s_print_version (FILE *out)
{
}

//
// Read block of data, up to 8 bytes.
// Halt the program on any error.
//
static void read_block (int fd, int start, unsigned char *data, int nbytes)
{
    unsigned char cmd[4], reply[4];
    int addr, len;

    // Send command.
    cmd[0] = 'R';
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
    if (reply[0] != 'W' || addr != start || reply[3] != nbytes) {
        fprintf (stderr, "Bad reply for block 0x%04x of %d bytes: %02x-%02x-%02x-%02x\n",
            start, nbytes, reply[0], reply[1], reply[2], reply[3]);
        exit(-1);
    }

    // Read data.
    len = read_with_timeout (fd, data, 8);
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
        ++radio_progress;
        if (radio_progress % 4 == 0) {
            fprintf (stderr, "#");
            fflush (stderr);
        }
    }
}

//
// Write block of data, up to 8 bytes.
// Halt the program on any error.
//
static void write_block (int fd, int start, const unsigned char *data, int nbytes)
{
    unsigned char cmd[4], reply;

    // Send command.
    cmd[0] = 'W';
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
        ++radio_progress;
        if (radio_progress % 4 == 0) {
            fprintf (stderr, "#");
            fflush (stderr);
        }
    }
}

//
// Read firmware image from the device.
//
static void bf888s_download()
{
    int addr;

    memset (radio_mem, 0xff, 0x400);
    for (addr=0x10; addr<0x110; addr+=8)
        read_block (radio_port, addr, &radio_mem[addr], 8);
    for (addr=0x2b0; addr<0x2c0; addr+=8)
        read_block (radio_port, addr, &radio_mem[addr], 8);
    for (addr=0x3c0; addr<0x3e0; addr+=8)
        read_block (radio_port, addr, &radio_mem[addr], 8);
}

//
// Write firmware image to the device.
//
static void bf888s_upload()
{
    int addr;

    for (addr=0x10; addr<0x110; addr+=8)
        write_block (radio_port, addr, &radio_mem[addr], 8);
    for (addr=0x2b0; addr<0x2c0; addr+=8)
        write_block (radio_port, addr, &radio_mem[addr], 8);
    for (addr=0x3c0; addr<0x3e0; addr+=8)
        write_block (radio_port, addr, &radio_mem[addr], 8);
}

static int bcd_to_int (uint32_t bcd)
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

static int int_to_bcd (uint32_t val)
{
    return ((val / 10000000) % 10) << 28 |
           ((val / 1000000)  % 10) << 24 |
           ((val / 100000)   % 10) << 20 |
           ((val / 10000)    % 10) << 16 |
           ((val / 1000)     % 10) << 12 |
           ((val / 100)      % 10) << 8 |
           ((val / 10)       % 10) << 4 |
           (val              % 10);
}

static void decode_squelch (uint16_t bcd, int *ctcs, int *dcs)
{
    if (bcd == 0 || bcd == 0xffff) {
        // Squelch disabled.
        return;
    }
    int index = ((bcd >> 12) & 15) * 1000 +
                ((bcd >> 8)  & 15) * 100 +
                ((bcd >> 4)  & 15) * 10 +
                (bcd         & 15);

    if (index < 8000) {
        // CTCSS value is Hz multiplied by 10.
        *ctcs = index;
    }
    // DCS mode.
    if (index < 12000)
        *dcs = index - 8000;
    else
        *dcs = - (index - 12000);
}

typedef struct {
    uint32_t    rxfreq;     // binary coded decimal, 8 digits
    uint32_t    txfreq;     // binary coded decimal, 8 digits
    uint16_t    rxtone;
    uint16_t    txtone;
    uint8_t     nobcl     : 1,
                noscr     : 1,
                narrow    : 1,
                highpower : 1,
                noscan    : 1,
                _u1       : 3;
    uint8_t     _u3[3];
} memory_channel_t;

static void decode_channel (int i, int *rx_hz, int *tx_hz,
    int *rx_ctcs, int *tx_ctcs, int *rx_dcs, int *tx_dcs,
    int *lowpower, int *wide, int *scan, int *bcl, int *scramble)
{
    memory_channel_t *ch = i + (memory_channel_t*) &radio_mem[0x10];

    *rx_hz = *tx_hz = *rx_ctcs = *tx_ctcs = *rx_dcs = *tx_dcs = 0;
    if (ch->rxfreq == 0 || ch->rxfreq == 0xffffffff)
        return;

    // Decode channel frequencies.
    *rx_hz = bcd_to_int (ch->rxfreq) * 10;
    *tx_hz = bcd_to_int (ch->txfreq) * 10;

    // Decode squelch modes.
    decode_squelch (ch->rxtone, rx_ctcs, rx_dcs);
    decode_squelch (ch->txtone, tx_ctcs, tx_dcs);

    // Other parameters.
    *lowpower = ! ch->highpower;
    *wide = ! ch->narrow;
    *scan = ! ch->noscan;
    *bcl = ! ch->nobcl;
    *scramble = ! ch->noscr;
}

static void setup_channel (int i, double rx_mhz, double tx_mhz,
    int rq, int tq, int highpower, int wide, int scan, int bcl, int scramble)
{
    memory_channel_t *ch = i + (memory_channel_t*) &radio_mem[0x10];

    ch->rxfreq = int_to_bcd ((int) (rx_mhz * 100000.0));
    ch->txfreq = int_to_bcd ((int) (tx_mhz * 100000.0));
    ch->rxtone = rq;
    ch->txtone = tq;
    ch->highpower = highpower;
    ch->narrow = ! wide;
    ch->noscan = ! scan;
    ch->nobcl = ! bcl;
    ch->noscr = ! scramble;
}

static void print_offset (FILE *out, int delta)
{
    if (delta == 0) {
        fprintf (out, " 0      ");
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

static void print_squelch (FILE *out, int ctcs, int dcs)
{
    if      (ctcs)    fprintf (out, "%5.1f", ctcs / 10.0);
    else if (dcs > 0) fprintf (out, "D%03dN", dcs);
    else if (dcs < 0) fprintf (out, "D%03dI", -dcs);
    else              fprintf (out, "   - ");
}

//
// Generic settings at 0x2b0.
//
typedef struct {
    uint8_t voice;      // Voice Prompt
    uint8_t chinese;    // Voice Language
    uint8_t scan;       // Scan
    uint8_t vox;        // VOX Function
    uint8_t voxgain;    // VOX Sensitivity (0='1' ... 4='5')
    uint8_t voxinhrx;   // VOX Inhibit On Receive
    uint8_t lowinhtx;   // Low Vol Inhibit Tx
    uint8_t highinhtx;  // High Vol Inhibit Tx
    uint8_t alarm;      // Alarm
    uint8_t fm;         // FM
} settings_t;

//
// Extra settings at 0x3c0.
//
typedef struct {
    uint8_t beep  : 1,  // Beep
            saver : 1,  // Battery Saver
            _u1   : 6;
    uint8_t squelch;    // Carrier Squelch Level (0-9)
    uint8_t sidekey;    // Side Key (0=Off, 1=Monitor, 2=TX Power, 3=Alarm)
    uint8_t timeout;    // TX Timer (0-10 multiply 30 sec)
} extra_settings_t;

//
// Print full information about the device configuration.
//
static void bf888s_print_config (FILE *out)
{
    int i;

    // Print memory channels.
    fprintf (out, "\n");
    fprintf (out, "Channel Receive  TxOffset R-Squel T-Squel Power FM     Scan BCL Scramble\n");
    for (i=0; i<NCHAN; i++) {
        int rx_hz, tx_hz, rx_ctcs, tx_ctcs, rx_dcs, tx_dcs;
        int lowpower, wide, scan, bcl, scramble;

        decode_channel (i, &rx_hz, &tx_hz, &rx_ctcs, &tx_ctcs,
            &rx_dcs, &tx_dcs, &lowpower, &wide, &scan, &bcl, &scramble);
        if (rx_hz == 0) {
            // Channel is disabled
            continue;
        }

        fprintf (out, "%5d   %8.4f ", i+1, rx_hz / 1000000.0);
        print_offset (out, tx_hz - rx_hz);
        fprintf (out, " ");
        print_squelch (out, rx_ctcs, rx_dcs);
        fprintf (out, "   ");
        print_squelch (out, tx_ctcs, tx_dcs);

        fprintf (out, "   %-4s  %-6s %-4s %-3s %s\n", lowpower ? "Low" : "High",
            wide ? "Wide" : "Narrow", scan ? "+" : "-",
            bcl ? "+" : "-", scramble ? "+" : "-");
    }

    // Print other settings.
    settings_t *mode = (settings_t*) &radio_mem[0x2b0];
    extra_settings_t *extra = (extra_settings_t*) &radio_mem[0x3c0];
    fprintf (out, "\n");
    fprintf (out, "Squelch Level: %u\n", extra->squelch);
    fprintf (out, "Side Key: %s\n", SIDEKEY_NAME[extra->sidekey & 3]);
    fprintf (out, "TX Timer: ");
    if (extra->timeout == 0) fprintf (out, "Off\n");
    else                     fprintf (out, "%u\n", extra->timeout * 30);
    fprintf (out, "Scan Function: %s\n", mode->scan ? "On" : "Off");
    fprintf (out, "Voice Prompt: %s\n", mode->voice ? "On" : "Off");
    fprintf (out, "Voice Language: %s\n", mode->chinese ? "Chinese" : "English");
    fprintf (out, "Alarm: %s\n", mode->alarm ? "On" : "Off");
    fprintf (out, "FM: %s\n", mode->fm ? "On" : "Off");
    fprintf (out, "VOX Function: %s\n", mode->vox ? "On" : "Off");
    fprintf (out, "VOX Sensitivity: %u\n", mode->voxgain);
    fprintf (out, "VOX Inhibit On Receive: %s\n", mode->voxinhrx ? "On" : "Off");
    fprintf (out, "Battery Saver: %s\n", extra->saver ? "On" : "Off");
    fprintf (out, "Beep: %s\n", extra->beep ? "On" : "Off");
    fprintf (out, "High Vol Inhibit TX: %s\n", mode->highinhtx ? "On" : "Off");
    fprintf (out, "Low Vol Inhibit TX: %s\n", mode->lowinhtx ? "On" : "Off");
}

//
// Read firmware image from the binary file.
// Try to be compatible with Baofeng BF-480 software.
//
static void bf888s_read_image (FILE *img, unsigned char *ident)
{
    char buf[8];

    if (fread (ident, 1, 8, img) != 8) {
        fprintf (stderr, "Error reading image header.\n");
        exit (-1);
    }
    // Ignore next 8 bytes.
    if (fread (buf, 1, 8, img) != 8) {
        fprintf (stderr, "Error reading header.\n");
        exit (-1);
    }
    if (fread (&radio_mem[0x10], 1, 0x3d0, img) != 0x3d0) {
        fprintf (stderr, "Error reading image data.\n");
        exit (-1);
    }

    // Move 16 bytes from 0x370 to 0x2b0.
    memcpy (radio_mem+0x2b0, radio_mem+0x370, 0x10);
    memset (radio_mem+0x370, 0xff, 0x10);
}

//
// Save firmware image to the binary file.
// Try to be compatible with Baofeng BF-480 software.
//
static void bf888s_save_image (FILE *img)
{
    fwrite (radio_ident, 1, 8, img);
    fwrite ("\xff\xff\xff\xff\xff\xff\xff\xff", 1, 8, img);
    fwrite (&radio_mem[0x10], 1, 0x2b0-0x10, img);
    fwrite ("\xff\xff\xff\xff\xff\xff\xff\xff", 1, 8, img);
    fwrite ("\xff\xff\xff\xff\xff\xff\xff\xff", 1, 8, img);
    fwrite (&radio_mem[0x2c0], 1, 0x370-0x2c0, img);
    fwrite (&radio_mem[0x2b0], 1, 0x10, img);
    fwrite (&radio_mem[0x380], 1, 0x3e0-0x380, img);
}

static int on_off (char *param, char *value)
{
    if (strcasecmp ("On", value) == 0)
        return 1;
    if (strcasecmp ("Off", value) == 0)
        return 0;
    fprintf (stderr, "Bad value for %s: %s\n", param, value);
    exit(-1);
}

static void parse_parameter (char *param, char *value)
{
    settings_t *mode = (settings_t*) &radio_mem[0x2b0];
    extra_settings_t *extra = (extra_settings_t*) &radio_mem[0x3c0];
    int i;

    if (strcasecmp ("Radio", param) == 0) {
        if (strcasecmp ("Baofeng BF-888S", value) != 0) {
bad:        fprintf (stderr, "Bad value for %s: %s\n", param, value);
            exit(-1);
        }
        return;
    }
    if (strcasecmp ("Squelch Level", param) == 0) {
        extra->squelch = atoi (value);
        return;
    }
    if (strcasecmp ("Side Key", param) == 0) {
        for (i=0; i<4; i++) {
            if (strcasecmp (SIDEKEY_NAME[i], value) == 0) {
                extra->sidekey = i;
                return;
            }
        }
        goto bad;
    }
    if (strcasecmp ("TX Timer", param) == 0) {
        if (strcasecmp ("Off", value) == 0) {
            extra->timeout = 0;
        } else {
            extra->timeout = atoi (value) / 30;
        }
        return;
    }
    if (strcasecmp ("Scan Function", param) == 0) {
        mode->scan = on_off (param, value);
        return;
    }
    if (strcasecmp ("Voice Prompt", param) == 0) {
        mode->voice = on_off (param, value);
        return;
    }
    if (strcasecmp ("Voice Language", param) == 0) {
        if (strcasecmp ("English", value) == 0) {
            mode->chinese = 0;
            return;
        }
        if (strcasecmp ("Chinese", value) == 0) {
            mode->chinese = 1;
            return;
        }
        goto bad;
    }
    if (strcasecmp ("Alarm", param) == 0) {
        mode->alarm = on_off (param, value);
        return;
    }
    if (strcasecmp ("FM", param) == 0) {
        mode->fm = on_off (param, value);
        return;
    }
    if (strcasecmp ("VOX Function", param) == 0) {
        mode->vox = on_off (param, value);
        return;
    }
    if (strcasecmp ("VOX Sensitivity", param) == 0) {
        mode->voxgain = atoi (value);
        if (mode->voxgain > 0)
             mode->voxgain -= 1;
        return;
    }
    if (strcasecmp ("VOX Inhibit On Receive", param) == 0) {
        mode->voxinhrx = on_off (param, value);
        return;
    }
    if (strcasecmp ("Battery Saver", param) == 0) {
        extra->saver = on_off (param, value);
        return;
    }
    if (strcasecmp ("Beep", param) == 0) {
        extra->beep = on_off (param, value);
        return;
    }
    if (strcasecmp ("High Vol Inhibit TX", param) == 0) {
        mode->highinhtx = on_off (param, value);
        return;
    }
    if (strcasecmp ("Low Vol Inhibit TX", param) == 0) {
        mode->lowinhtx = on_off (param, value);
        return;
    }
    fprintf (stderr, "Unknown parameter: %s = %s\n", param, value);
    exit(-1);
}

static int encode_squelch (char *str)
{
    // TODO
    return 0;
}

static void parse_channel (int first_flag, char *num_str, char *rxfreq_str,
    char *offset_str, char *rq_str, char *tq_str, char *power_str, char *wide_str,
    char *scan_str, char *bcl_str, char *scramble_str)
{
    int num, rq, tq, highpower, wide, scan, bcl, scramble;
    float rx_mhz, txoff_mhz;

    num = atoi (num_str);
    if (num < 1 || num > NCHAN) {
        fprintf (stderr, "Bad channel number");
bad:    fprintf (stderr, " in row: %s %s %s %s %s %s %s %s %s %s\n",
            num_str, rxfreq_str, offset_str, rq_str, tq_str, power_str,
            wide_str, scan_str, bcl_str, scramble_str);
        exit (-1);
    }
    if (sscanf (rxfreq_str, "%f", &rx_mhz) != 1 ||
        rx_mhz < 400 || rx_mhz >= 470)
    {
        fprintf (stderr, "Bad receive frequency");
        goto bad;
    }
    if (sscanf (offset_str, "%f", &txoff_mhz) != 1 ||
        (rx_mhz + txoff_mhz) < 400 || (rx_mhz + txoff_mhz) >= 470)
    {
        fprintf (stderr, "Bad transmit offset");
        goto bad;
    }
    rq = encode_squelch (rq_str);
    tq = encode_squelch (tq_str);

    if (strcasecmp ("High", power_str) == 0) {
        highpower = 1;
    } else if (strcasecmp ("Low", power_str) == 0) {
        highpower = 0;
    } else {
        fprintf (stderr, "Bad power level");
        goto bad;
    }

    if (strcasecmp ("Wide", wide_str) == 0) {
        wide = 1;
    } else if (strcasecmp ("Narrow", wide_str) == 0) {
        wide = 0;
    } else {
        fprintf (stderr, "Bad modulation width");
        goto bad;
    }

    if (*scan_str == '+') {
        scan = 1;
    } else if (*scan_str == '-') {
        scan = 0;
    } else {
        fprintf (stderr, "Bad scan flag");
        goto bad;
    }

    if (*bcl_str == '+') {
        bcl = 1;
    } else if (*bcl_str == '-') {
        bcl = 0;
    } else {
        fprintf (stderr, "Bad BCL flag");
        goto bad;
    }

    if (*scramble_str == '+') {
        scramble = 1;
    } else if (*scramble_str == '-') {
        scramble = 0;
    } else {
        fprintf (stderr, "Bad scramble flag");
        goto bad;
    }

    if (first_flag) {
        // On first entry, erase the channel table.
        int i;
        for (i=0; i<NCHAN; i++) {
            setup_channel (i, 0, 0, 0, 0, 1, 1, 0, 0, 0);
        }
    }
    setup_channel (num-1, rx_mhz, rx_mhz + txoff_mhz, rq, tq,
        highpower, wide, scan, bcl, scramble);
}

//
// Read the configuration from text file, and modify the firmware.
//
static void bf888s_parse_config (FILE *in)
{
    char line [256], *p, *v, table;
    int table_dirty = 0;

    fprintf (stderr, "Parse configuration for BF-888S.\n");

    table = 0;
    while (fgets (line, sizeof(line), in)) {
        // Strip trailing spaces and newline.
        line[sizeof(line)-1] = 0;
        v = line + strlen(line) - 1;
        while (v >= line && (*v=='\n' || *v=='\r' || *v==' ' || *v=='\t'))
            *v-- = 0;

        // Ignore comments and empty lines.
        p = line;
        if (*p == '#' || *p == 0)
            continue;

        if (*p != ' ') {
            // Table finished.
            table = 0;

            // Find the value.
            v = strchr (p, ':');
            if (! v) {
                // Table header.
                v = strchr (p, ' ');
                if (! v) {
badline:            fprintf (stderr, "Invalid line: '%s'\n", line);
                    exit(-1);
                }

                // Decode table type.
                if (strncasecmp (p, "Channel", 7) == 0) {
                    table = 'C';
                    table_dirty = 0;
                } else
                    goto badline;
                continue;
            }

            // Parameter.
            *v++ = 0;

            // Skip spaces.
            while (*v == ' ' || *v == '\t')
                v++;

            parse_parameter (p, v);

        } else {
            // Table row or comment.
            // Skip spaces.
            // Ignore comments and empty lines.
            while (*p == ' ' || *p == '\t')
                p++;
            if (*p == '#' || *p == 0)
                continue;
            if (! table)
                goto badline;

            char v1[256], v2[256], v3[256], v4[256], v5[256];
            char v6[256], v7[256], v8[256], v9[256], v10[256];
            if (sscanf (p, "%s %s %s %s %s %s %s %s %s %s",
                v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) != 10)
            {
                goto badline;
            }

            parse_channel (! table_dirty, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
            table_dirty = 1;
        }
    }
//exit (-1);
}

//
// Baofeng BF-888S
//
radio_device_t radio_bf888s = {
    "Baofeng BF-888S",
    bf888s_download,
    bf888s_upload,
    bf888s_read_image,
    bf888s_save_image,
    bf888s_print_version,
    bf888s_print_config,
    bf888s_parse_config,
};
