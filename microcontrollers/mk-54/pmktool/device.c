/*
 * Interface to MK-54 calculator via USB HID interface.
 *
 * Copyright (C) 2014 Serge Vakulenko
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "hidapi.h"

/*
 * MK-54 commands.
 */
#define CMD_QUERY_DEVICE    0xc1    // Get generic status
#define CMD_READ_STACK      0xc2    // Read X, Y, Z, T, X1 values
#define CMD_READ_REG_LOW    0xc3    // Read registers 0-7
#define CMD_READ_REG_HIGH   0xc4    // Read registers 8-E
#define CMD_READ_PROG_LOW   0xc5    // Read program 0-59
#define CMD_READ_PROG_HIGH  0xc6    // Read program 60-105
#define CMD_WRITE_PROG_LOW  0xc7    // Write program 0-59
#define CMD_WRITE_PROG_HIGH 0xc8    // Write program 60-105

struct _device_t {
    hid_device *hiddev;                 // handle for hidapi
    unsigned char request [64];         // request to send
    unsigned char reply [64];           // reply received
};

/*
 * Identifiers of USB device.
 */
#define CALCULATOR_VID          0xca1c  /* Bogus vendor id */
#define MK54_PID                0x0054  /* MK-54 calculator */

static int debug_level;

/*
 * Send a request to the device.
 * Store the reply into the d->reply[] array.
 */
static void send_recv (device_t *d, unsigned char cmd, unsigned nbytes)
{
    unsigned k;
    int reply_len;

    d->request[0] = cmd;
    memset (d->request + nbytes + 1, 0, 64 - nbytes - 1);

    if (debug_level > 0) {
        fprintf (stderr, "---Send");
        for (k=0; k<=nbytes; ++k) {
            if (k != 0 && (k & 15) == 0)
                fprintf (stderr, "\n       ");
            fprintf (stderr, " %02x", d->request[k]);
        }
        fprintf (stderr, "\n");
    }
    hid_write (d->hiddev, d->request, 64);

    if (cmd == CMD_WRITE_PROG_LOW || cmd == CMD_WRITE_PROG_HIGH) {
        /* No reply expected. */
        return;
    }

    memset (d->reply, 0, sizeof(d->reply));
    reply_len = hid_read_timeout (d->hiddev, d->reply, 64, 4000);
    if (reply_len == 0) {
        fprintf (stderr, "Timed out.\n");
        exit (-1);
    }
    if (reply_len != 64) {
        fprintf (stderr, "hid device: error %d receiving packet\n", reply_len);
        exit (-1);
    }
    if (debug_level > 0) {
        fprintf (stderr, "---Recv");
        for (k=0; k<reply_len; ++k) {
            if (k != 0 && (k & 15) == 0)
                fprintf (stderr, "\n       ");
            fprintf (stderr, " %02x", d->reply[k]);
        }
        fprintf (stderr, "\n");
    }
}

void device_close (device_t *d)
{
    free (d);
}

/*
 * Read the calculator's stack: X, Y, Z, T and X1 value.
 */
void device_read_stack (device_t *d, unsigned char stack[5][6])
{
    send_recv (d, CMD_READ_STACK, 0);
    if (d->reply[0] != CMD_READ_STACK ||
        d->reply[1] != 2 + 5*6) {       /* Reply data size */
        fprintf (stderr, "hid device: bad reply for READ_STACK command\n");
        exit (-1);
    }
    memcpy (stack, &d->reply[2], 5*6);
}

/*
 * Read the calculator's registers 0-14.
 */
void device_read_regs (device_t *d, unsigned char regs[14][6])
{
    send_recv (d, CMD_READ_REG_LOW, 0);
    if (d->reply[0] != CMD_READ_REG_LOW ||
        d->reply[1] != 2 + 8*6) {       /* Reply data size */
        fprintf (stderr, "hid device: bad reply for READ_REG_LOW command\n");
        exit (-1);
    }
    memcpy (&regs[0], &d->reply[2], 8*6);

    send_recv (d, CMD_READ_REG_HIGH, 0);
    if (d->reply[0] != CMD_READ_REG_HIGH ||
        d->reply[1] != 2 + 6*6) {       /* Reply data size */
        fprintf (stderr, "hid device: bad reply for READ_REG_HIGH command\n");
        exit (-1);
    }
    memcpy (&regs[8], &d->reply[2], 6*6);
}

/*
 * Read the calculator's program memory.
 */
void device_read_program (device_t *d, unsigned char *data)
{
    // TODO
#if 0
    unsigned nbytes, n;
    unsigned addr = 0;

    for (nbytes=98; ; nbytes-=50) {
        n = nbytes>50 ? 50 : nbytes;
        d->request[1] = addr;
        d->request[2] = n;

        send_recv (d, CMD_GET_DATA, 2);

        memcpy (data, d->reply, n);

        if (nbytes <= 50)
            break;
        data += 50;
        addr += 50;
    }
#endif
}

/*
 * Write the calculator's program memory.
 */
void device_write_program (device_t *d, unsigned char *data)
{
    // TODO
#if 0
    unsigned nbytes, n;
    unsigned addr = 0;

    for (nbytes=98; nbytes>0; nbytes-=50) {
        n = nbytes>50 ? 50 : nbytes;

        //fprintf (stderr, "hid device: program %d bytes at %08x: %08x-%08x-...-%08x\n",
        //    nbytes, addr, data[0], data[1], data[nbytes-1]);

        d->request[1] = addr;
        d->request[2] = n;
        memcpy (d->request + 3, data, n);

        send_recv (d, CMD_PROGRAM_DEVICE, n + 2);

        data += 50;
        addr += 50;
    }
    send_recv (d, CMD_PROGRAM_COMPLETE, 0);
#endif
}

/*
 * Connect to device via USB port.
 * Return a pointer to a data structure, allocated dynamically.
 * When device not found, return 0.
 */
device_t *device_open (int debug)
{
    device_t *d;
    hid_device *hiddev;

    debug_level = debug;
    hiddev = hid_open (CALCULATOR_VID, MK54_PID, 0);
    if (! hiddev) {
        /*fprintf (stderr, "HID device not found\n");*/
        return 0;
    }
    d = calloc (1, sizeof (*d));
    if (! d) {
        fprintf (stderr, "Out of memory\n");
        return 0;
    }
    d->hiddev = hiddev;

    /* Read version of device. */
    send_recv (d, CMD_QUERY_DEVICE, 0);
    if (d->reply[0] != CMD_QUERY_DEVICE ||
        d->reply[1] != 2)                   /* Reply data size */
            return 0;

    return d;
}
