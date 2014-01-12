/*
 * Interface to MK-54 calculator via USB HID interface.
 *
 * Copyright (C) 2014 Serge Vakulenko
 *
 * This file is part of PIC32PROG project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "hidapi.h"

/*
 * MK-54 commands.
 */
#define CMD_QUERY_DEVICE        0xc1
#define CMD_PROGRAM_DEVICE      0xc2
#define CMD_PROGRAM_COMPLETE    0xc3
#define CMD_GET_DATA            0xc4

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

    if (cmd != CMD_QUERY_DEVICE && cmd != CMD_GET_DATA) {
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
 * Read the calculator's program memory.
 */
void device_read (device_t *d, unsigned char *data)
{
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
}

/*
 * Write the calculator's program memory.
 */
void device_program (device_t *d, unsigned char *data)
{
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
        d->reply[1] != 56 ||                /* HID packet data size */
        d->reply[2] != 3 ||                 /* PIC32 device family */
        d->reply[3] != 1)                   /* program memory type */
            return 0;

    return d;
}
