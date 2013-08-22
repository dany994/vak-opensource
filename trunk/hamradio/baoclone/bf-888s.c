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
#include <termios.h>
#include "radio.h"
#include "util.h"

static const int DCS_CODES[] = {
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

static const char MAGIC[] = "PROGRAM";

static const char *PTTID_NAME[] = { "-", "Beg", "End", "Both" };

static const char *STEP_NAME[] = { "2.5 ", "5.0 ", "6.25", "10.0",
                            "12.5", "25.0", "????", "????" };

static const char *SAVER_NAME[] = { "Off", "1", "2", "3", "4", "?5?", "?6?", "?7?" };

static const char *VOX_NAME[] = { "Off", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                           "10", "?11?", "?12?", "?13?", "?14?", "?15?" };

static const char *ABR_NAME[] = { "Off", "1", "2", "3", "4", "5", "?6?", "?7?" };

static const char *DTMF_SIDETONE_NAME[] = { "Off", "DTMF Only", "ANI Only", "DTMF+ANI" };

static const char *SCAN_RESUME_NAME[] = { "After Timeout", "When Carrier Off", "Stop On Active", "??" };

static const char *DISPLAY_MODE_NAME[] = { "Channel", "Name", "Frequency", "??" };

static const char *COLOR_NAME[] = { "Off", "Blue", "Orange", "Purple" };

static const char *ALARM_NAME[] = { "Site", "Tone", "Code", "??" };

static const char *RPSTE_NAME[] = { "Off", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                             "10", "?11?", "?12?", "?13?", "?14?", "?15?" };

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

static void decode_squelch (uint16_t index, int *ctcs, int *dcs)
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

static void decode_channel (int i, char *name, int *rx_hz, int *tx_hz,
    int *rx_ctcs, int *tx_ctcs, int *rx_dcs, int *tx_dcs,
    int *lowpower, int *wide, int *scan, int *pttid, int *scode)
{
    memory_channel_t *ch = i + (memory_channel_t*) radio_mem;

    *rx_hz = *tx_hz = *rx_ctcs = *tx_ctcs = *rx_dcs = *tx_dcs = 0;
    *name = 0;
    if (ch->rxfreq == 0 || ch->rxfreq == 0xffffffff)
        return;

    // Extract channel name; strip trailing FF's.
    char *p;
    strncpy (name, (char*) &radio_mem[0x1000 + i*16], 7);
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
static void decode_limits (char band, int *enable, int *lower, int *upper)
{
    int offset = (band == 'V') ? 0x1EC0+0x100 : 0x1EC0+0x105;

    limits_t *limits = (limits_t*) (radio_mem + offset);
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

static void fetch_ani (char *ani)
{
    int i;

    for (i=0; i<5; i++)
        ani[i] = "0123456789ABCDEF" [radio_mem[0x0CAA+i] & 0x0f];
}

static void get_current_channel (int index, int *chan_num)
{
    unsigned char *ptr = radio_mem + 0x0E76;
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

static void decode_vfo (int index, int *band, int *hz, int *offset,
    int *rx_ctcs, int *tx_ctcs, int *rx_dcs, int *tx_dcs,
    int *lowpower, int *wide, int *step, int *scode)
{
    vfo_t *vfo = (vfo_t*) &radio_mem[index ? 0x0F28 : 0x0F08];

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

static void print_offset (FILE *out, int delta)
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

static void print_squelch (FILE *out, int ctcs, int dcs)
{
    if      (ctcs)    fprintf (out, "%5.1f", ctcs / 10.0);
    else if (dcs > 0) fprintf (out, "D%03dN", dcs);
    else if (dcs < 0) fprintf (out, "D%03dI", -dcs);
    else              fprintf (out, "   - ");
}

static void print_vfo (FILE *out, char name, int band, int hz, int offset,
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
// Print full information about the device configuration.
//
static void bf888s_print_config (FILE *out)
{
    fprintf (stderr, "TODO: Print configuration of BF-888S.\n");
    return;

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
    settings_t *mode = (settings_t*) &radio_mem[0x0E20];
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
};
