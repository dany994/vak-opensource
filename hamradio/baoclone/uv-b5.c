/*
 * Interface to Baofeng UV-B5 and compatibles.
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

#define NCHAN 99

static const int CTCSS_TONES[50] = {
     670,  693,  719,  744,  770,  797,  825,  854,  885,  915,
     948,  974, 1000, 1035, 1072, 1109, 1148, 1188, 1230, 1273,
    1318, 1365, 1413, 1462, 1514, 1567, 1598, 1622, 1655, 1679,
    1713, 1738, 1773, 1799, 1835, 1862, 1899, 1928, 1966, 1995,
    2035, 2065, 2107, 2181, 2257, 2291, 2336, 2418, 2503, 2541,
};

static const int DCS_CODES[104] = {
     23,  25,  26,  31,  32,  36,  43,  47,  51,  53,
     54,  65,  71,  72,  73,  74, 114, 115, 116, 122,
    125, 131, 132, 134, 143, 145, 152, 155, 156, 162,
    165, 172, 174, 205, 212, 223, 225, 226, 243, 244,
    245, 246, 251, 252, 255, 261, 263, 265, 266, 271,
    274, 306, 311, 315, 325, 331, 332, 343, 346, 351,
    356, 364, 365, 371, 411, 412, 413, 423, 431, 432,
    445, 446, 452, 454, 455, 462, 464, 465, 466, 503,
    506, 516, 523, 526, 532, 546, 565, 606, 612, 624,
    627, 631, 632, 654, 662, 664, 703, 712, 723, 731,
    732, 734, 743, 754,
};

static const char CHARSET[] = "0123456789- ABCDEFGHIJKLMNOPQRSTUVWXYZ/_+*";

static const char *PTTID_NAME[] = { "Off", "Begin", "End", "Both" };

static const char *STEP_NAME[] = { "5.0",  "6.25", "10.0", "12.5",
                                   "20.0", "25.0", "????", "????" };

static const char *VOX_NAME[] = { "Off", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                           "10", "?11?", "?12?", "?13?", "?14?", "?15?" };

static const char *TIMER_NAME[] = { "Off", "1", "2", "3", "4", "5", "6", "7" };

static const char *SCAN_NAME[] = { "Time", "Carrier", "Seek", "??" };

static const char *TXTDR_NAME[] = { "Current Frequency", "Frequency A", "Frequency B", "??" };

static const char *DISPLAY_MODE_NAME[] = { "Frequency", "Name", "Channel", "??" };

//
// Print a generic information about the device.
//
static void uvb5_print_version (FILE *out)
{
}

//
// Read block of data, up to 16 bytes.
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
    len = read_with_timeout (fd, data, 0x10);
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
    if (reply[0] != 0x74 && reply[0] != 0x78 && reply[0] != 0x1f) {
        fprintf (stderr, "Bad acknowledge after block 0x%04x: %02x\n", start, reply[0]);
        exit(-1);
    }
    if (verbose) {
        printf ("# Read 0x%04x: ", start);
        print_hex (data, nbytes);
        printf ("\n");
    } else {
        ++radio_progress;
        if (radio_progress % 8 == 0) {
            fprintf (stderr, "#");
            fflush (stderr);
        }
    }
}

//
// Write block of data, up to 16 bytes.
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
        if (radio_progress % 8 == 0) {
            fprintf (stderr, "#");
            fflush (stderr);
        }
    }
}

//
// Read memory image from the device.
//
static void uvb5_download()
{
    int addr;

    for (addr=0; addr<0x1000; addr+=0x10)
        read_block (radio_port, addr, &radio_mem[addr], 0x10);
}

//
// Write memory image to the device.
//
static void uvb5_upload()
{
    int addr;

    for (addr=0; addr<0x1000; addr+=0x10)
        write_block (radio_port, addr, &radio_mem[addr], 0x10);
}

static void decode_squelch (uint8_t index, int pol, int *ctcs, int *dcs)
{
    if (index == 0) {
        // Squelch disabled.
        return;
    }
    if (index <= 50) {
        // CTCSS value is Hz multiplied by 10.
        *ctcs = CTCSS_TONES[index - 1];
        *dcs = 0;
        return;
    }
    // DCS mode.
    *dcs = DCS_CODES[index - 51];
    if (pol)
        *dcs = - *dcs;
    *ctcs = 0;
}

typedef struct {
    uint32_t    rxfreq;     // binary coded decimal, 8 digits
    uint32_t    txoff;      // binary coded decimal, 8 digits
    uint8_t     step      : 3,
                compander : 1,
                rxpol     : 1,
                txpol     : 1,
                _u2       : 2;
    uint8_t     rxtone;
    uint8_t     txtone;
    uint8_t     duplex    : 2, // unused?
                revfreq   : 1,
                highpower : 1,
                bcl       : 1,
                isnarrow  : 1,
                scanadd   : 1,
                pttid     : 1;
    uint8_t     _u3 [4];
} memory_channel_t;

static void decode_channel (int i, char *name, int *rx_hz, int *txoff_hz,
    int *rx_ctcs, int *tx_ctcs, int *rx_dcs, int *tx_dcs, int *step,
    int *lowpower, int *wide, int *scan, int *pttid, int *bcl,
    int *compander, int *revfreq)
{
    memory_channel_t *ch = i + (memory_channel_t*) radio_mem;

    *rx_hz = *txoff_hz = *rx_ctcs = *tx_ctcs = *rx_dcs = *tx_dcs = 0;
    *lowpower = *wide = *scan = *pttid = *bcl = *compander = 0;
    *step = *revfreq = 0;
    if (name)
        *name = 0;
    if (ch->rxfreq == 0 || ch->rxfreq == 0xffffffff)
        return;

    // Extract channel name; strip trailing FF's.
    if (name && i >= 1 && i <= NCHAN) {
        unsigned char *p = (unsigned char*) &radio_mem[0x0A00 + (i-1)*5];
        int n;
        for (n=0; n<5; n++) {
            name[n] = (*p < 42) ? CHARSET[*p++]: 0;
        }
        name[5] = 0;
    }

    // Decode channel frequencies.
    *rx_hz = bcd_to_int (ch->rxfreq) * 10;
    *txoff_hz = bcd_to_int (ch->txoff) * 10;

    // Decode squelch modes.
    decode_squelch (ch->rxtone, ch->rxpol, rx_ctcs, rx_dcs);
    decode_squelch (ch->txtone, ch->txpol, tx_ctcs, tx_dcs);

    // Other parameters.
    *step = ch->step;
    *lowpower = ! ch->highpower;
    *wide = ! ch->isnarrow;
    *scan = ch->scanadd;
    *pttid = ch->pttid;
    *bcl = ch->bcl;
    *compander = ch->compander;
    *revfreq = ch->revfreq;
}

typedef struct {
    uint8_t     lower_lsb;  // binary coded decimal, 4 digits
    uint8_t     lower_msb;
    uint8_t     upper_lsb;  // binary coded decimal, 4 digits
    uint8_t     upper_msb;
} limits_t;

//
// Looks like limits are not implemented on old firmware
// (prior to version 291).
//
static void decode_limits (char band, int *lower, int *upper)
{
    int offset = (band == 'V') ? 0xF00 : 0xF04;

    limits_t *limits = (limits_t*) (radio_mem + offset);
    *lower = ((limits->lower_msb >> 4) & 15) * 1000 +
             (limits->lower_msb        & 15) * 100 +
             ((limits->lower_lsb >> 4) & 15) * 10 +
             (limits->lower_lsb        & 15);
    *upper = ((limits->upper_msb >> 4) & 15) * 1000 +
             (limits->upper_msb        & 15) * 100 +
             ((limits->upper_lsb >> 4) & 15) * 10 +
             (limits->upper_lsb        & 15);
}

static void fetch_ani (char *ani)
{
    int i;

    for (i=0; i<6; i++)
        ani[i] = "0123456789ABCDEF" [radio_mem[0x0D20+i] & 0x0f];
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

static void print_vfo (FILE *out, char name, int hz, int offset,
    int rx_ctcs, int tx_ctcs, int rx_dcs, int tx_dcs, int step,
    int lowpower, int wide, int pttid, int bcl, int revfreq, int compander)
{
    fprintf (out, " %c  %8.4f ", name, hz / 1000000.0);
    print_offset (out, offset);
    fprintf (out, " ");
    print_squelch (out, rx_ctcs, rx_dcs);
    fprintf (out, " ");
    print_squelch (out, tx_ctcs, tx_dcs);

    fprintf (out, " %-4s %-4s  %-6s %-4s %-3s %-4s %s\n",
        STEP_NAME[step & 7], lowpower ? "Low" : "High",
        wide ? "Wide" : "Narr", pttid ? "+" : "-", bcl ? "+" : "-",
        revfreq ? "+" : "-", compander ? "+" : "-");
}

typedef struct {
    uint8_t     msb;
    uint8_t     lsb;
} fm_t;

//
// Generic settings.
//
typedef struct {
    uint8_t squelch;            // Carrier Squelch Level
    uint8_t scantype    : 2,    // Scan Resume
            tdr         : 1,    // Dual Watch
            roger       : 1,    // Roger Beep
            nobeep      : 1,    // Keypad Beep Disable
            backlight   : 1,    // Backlight
            save_funct  : 1,    // Battery Saver
            freqmode_ab : 1;    // Frequency mode
    uint8_t pttid       : 2,    // PTTID mode
            fm          : 1,    // FM Radio
            voice_prompt: 1,    // Voice Enable
            workmode_fm : 1,
            workmode_a  : 1,    // Freq/channel Mode A
            workmode_b  : 1,    // Freq/channel Mode B
            language    : 1;    // Voice Language
    uint8_t timeout;            // Timeout Timer
    uint8_t txtdr       : 2,
            _u1         : 2,
            mdf_a       : 2,    // Display Mode A
            mdf_b       : 2;    // Display Mode B
    uint8_t sidetone    : 1,    // DTMF Sidetone
            _u2         : 2,
            sqtail      : 1,    // Squelch tail enable
            _u3         : 4;
    uint8_t vox;                // VOX Level
} settings_t;

//
// Print full information about the device configuration.
//
static void uvb5_print_config (FILE *out)
{
    int i;

    // Print memory channels.
    fprintf (out, "\n");
    fprintf (out, "Channel Name  Receive  TxOffset Rx-Sq Tx-Sq Power FM   Scan PTTID BCL Rev Compand\n");
    for (i=1; i<=NCHAN; i++) {
        int rx_hz, txoff_hz, rx_ctcs, tx_ctcs, rx_dcs, tx_dcs;
        int step, lowpower, wide, scan, pttid;
        int bcl, compander, revfreq;
        char name[17];

        decode_channel (i, name, &rx_hz, &txoff_hz, &rx_ctcs, &tx_ctcs,
            &rx_dcs, &tx_dcs, &step, &lowpower, &wide, &scan, &pttid,
            &bcl, &compander, &revfreq);

        if (rx_hz == 0) {
            // Channel is disabled
            continue;
        }

        fprintf (out, "%5d   %-5s %8.4f ", i, name, rx_hz / 1000000.0);
        print_offset (out, txoff_hz);
        fprintf (out, " ");
        print_squelch (out, rx_ctcs, rx_dcs);
        fprintf (out, " ");
        print_squelch (out, tx_ctcs, tx_dcs);

        fprintf (out, " %-4s  %-6s %-4s %-4s %-3s %-4s %s\n",
            lowpower ? "Low" : "High", wide ? "Wide" : "Narr",
            scan ? "+" : "-", pttid ? "+" : "-",
            bcl ? "+" : "-", revfreq ? "+" : "-", compander ? "+" : "-");
    }

    // Print frequency mode VFO settings.
    int hz, offset, rx_ctcs, tx_ctcs, rx_dcs, tx_dcs;
    int step, lowpower, wide, scan, pttid;
    int bcl, compander, revfreq;;
    fprintf (out, "\n");

    decode_channel (0, 0, &hz, &offset, &rx_ctcs, &tx_ctcs,
        &rx_dcs, &tx_dcs, &step, &lowpower, &wide, &scan, &pttid,
        &bcl, &compander, &revfreq);
    fprintf (out, "VFO Receive  TxOffset Rx-Sq Tx-Sq Step Power FM   PTTID BCL Rev Compand\n");
    print_vfo (out, 'A', hz, offset, rx_ctcs, tx_ctcs, rx_dcs, tx_dcs,
        step, lowpower, wide, pttid, bcl, revfreq, compander);
    decode_channel (130, 0, &hz, &offset, &rx_ctcs, &tx_ctcs,
        &rx_dcs, &tx_dcs, &step, &lowpower, &wide, &scan, &pttid,
        &bcl, &compander, &revfreq);
    print_vfo (out, 'B', hz, offset, rx_ctcs, tx_ctcs, rx_dcs, tx_dcs,
        step, lowpower, wide, pttid, bcl, revfreq, compander);

    // Print band limits.
    int vhf_lower, vhf_upper, uhf_lower, uhf_upper;
    decode_limits ('V', &vhf_lower, &vhf_upper);
    decode_limits ('U', &uhf_lower, &uhf_upper);
    fprintf (out, "\n");
    fprintf (out, "Limit Lower  Upper \n");
    fprintf (out, " VHF  %5.1f  %5.1f\n", vhf_lower/10.0, vhf_upper/10.0);
    fprintf (out, " UHF  %5.1f  %5.1f\n", uhf_lower/10.0, uhf_upper/10.0);

    // Broadcast FM.
    fm_t *fm = (fm_t*) &radio_mem[0x09A0];
    fprintf (out, "\n");
    fprintf (out, "FM   Frequency\n");
    for (i=0; i<16; i++) {
        int freq = (fm[i].msb << 8) + fm[i].lsb + 650;
        if (freq <= 1080)
            fprintf (out, " %-2d  %5.1f\n", i+1, freq / 10.0);
    }

    // Get atomatic number identifier.
    char ani[6];
    fetch_ani (ani);

    // Print other settings.
    settings_t *mode = (settings_t*) &radio_mem[0x0D00];
    fprintf (out, "\n");
    fprintf (out, "Squelch Level: %u\n", mode->squelch);
    fprintf (out, "Battery Saver: %s\n", mode->save_funct ? "On" : "Off");
    fprintf (out, "Roger Beep: %s\n", mode->roger ? "On" : "Off");
    fprintf (out, "Transmittion Timer: %s\n", TIMER_NAME[mode->timeout & 7]);
    fprintf (out, "VOX Level: %s\n", VOX_NAME[mode->vox & 15]);
    fprintf (out, "Keypad Beep: %s\n", !mode->nobeep ? "On" : "Off");
    fprintf (out, "Voice Prompt: %s\n", mode->voice_prompt ? "On" : "Off");
    fprintf (out, "Dual Watch: %s\n", mode->tdr ? "On" : "Off");
    fprintf (out, "Backlight: %s\n", mode->backlight ? "On" : "Off");
    fprintf (out, "PTT ID Transmit: %s\n", PTTID_NAME[mode->pttid & 3]);
    fprintf (out, "ANI Code: %c%c%c%c%c%c\n", ani[0], ani[1], ani[2], ani[3], ani[4], ani[5]);
    fprintf (out, "DTMF Sidetone: %s\n", mode->sidetone ? "On" : "Off");
    fprintf (out, "Display A Mode: %s\n", DISPLAY_MODE_NAME[mode->mdf_a & 3]);
    fprintf (out, "Display B Mode: %s\n", DISPLAY_MODE_NAME[mode->mdf_b & 3]);
    fprintf (out, "Scan Resume: %s\n", SCAN_NAME[mode->scantype & 3]);
    fprintf (out, "Frequency mode: %s\n", mode->freqmode_ab ? "B" : "A");
    fprintf (out, "TX in Dual Watch: %s\n", TXTDR_NAME[mode->txtdr & 3]);
    fprintf (out, "Squelch Tail Eliminate: %s\n", !mode->sqtail ? "On" : "Off");
    fprintf (out, "Voice Language: %s\n", mode->language ? "Chinese" : "English");

    // Transient modes: no need to backup or configure.
    //fprintf (out, "Radio A Mode: %s\n", mode->workmode_a ? "Channel" : "Frequency");
    //fprintf (out, "Radio B Mode: %s\n", mode->workmode_b ? "Channel" : "Frequency");
    //fprintf (out, "FM Radio: %s\n", mode->fm ? "On" : "Off");
    //fprintf (out, "FM Radio Mode: %s\n", mode->workmode_fm ? "Channel" : "Frequency");
}

//
// Read memory image from the binary file.
//
static void uvb5_read_image (FILE *img, unsigned char *ident)
{
    char buf[40];

    if (fread (ident, 1, 8, img) != 8) {
        fprintf (stderr, "Error reading image header.\n");
        exit (-1);
    }
    // Ignore next 40 bytes.
    if (fread (buf, 1, 40, img) != 40) {
        fprintf (stderr, "Error reading header.\n");
        exit (-1);
    }
    if (fread (&radio_mem[0], 1, 0x1000, img) != 0x1000) {
        fprintf (stderr, "Error reading image data.\n");
        exit (-1);
    }
}

//
// Save memory image to the binary file.
// Try to be compatible with Chirp.
//
static void uvb5_save_image (FILE *img)
{
    fwrite (radio_ident, 1, 8, img);
    fwrite ("Radio Program data v1.08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 1, 40, img);
    fwrite (&radio_mem[0], 1, 0x1000, img);
}

static void uvb5_parse_parameter (char *param, char *value)
{
    fprintf (stderr, "TODO: Parse parameter for UV-B5.\n");
    exit(-1);
}

static int uvb5_parse_header (char *line)
{
    fprintf (stderr, "TODO: Parse table header for UV-B5.\n");
    exit(-1);
}

static int uvb5_parse_row (int table_id, int first_row, char *line)
{
    fprintf (stderr, "TODO: Parse table row for UV-B5.\n");
    exit(-1);
}

//
// Baofeng UV-B5, UV-B6
//
radio_device_t radio_uvb5 = {
    "Baofeng UV-B5",
    uvb5_download,
    uvb5_upload,
    uvb5_read_image,
    uvb5_save_image,
    uvb5_print_version,
    uvb5_print_config,
    uvb5_parse_parameter,
    uvb5_parse_header,
    uvb5_parse_row,
};
