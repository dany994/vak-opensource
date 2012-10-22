/*
 * Open USB connection to FT232R-based Morse paddle interface and
 * display key events.
 *
 * Usage:
 *      dump-paddle /dev/tty.usbserial-A1016UNH
 *
 * Copyright (C) 2012 Serge Vakulenko, <serge@vak.ru>
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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <usb.h>

/*
 * Identifiers of USB adapter.
 */
#define FT232R_VID      0x0403
#define FT232R_PID      0x6001  /* homemade RS232R adapter */

/*
 * Bit 7 (0x80): unused.
 * Bit 6 (0x40): unused.
 * Bit 5 (0x20): unused.
 * Bit 4 (0x10): unused.
 * Bit 3 (0x08): TMS output.
 * Bit 2 (0x04): TDO input.
 * Bit 1 (0x02): TDI output.
 * Bit 0 (0x01): TCK output.
 *
 * Sync bit bang mode is implemented, as described in FTDI Application
 * Note AN232R-01: "Bit Bang Modes for the FT232R and FT245R".
 */
#define TCK             (1 << 0)
#define TDI             (1 << 1)
#define READ_TDO        (1 << 2)
#define TMS             (1 << 3)

/*
 * USB endpoints.
 */
#define IN_EP           0x02
#define OUT_EP          0x81

/* Requests */
#define SIO_RESET               0 /* Reset the port */
#define SIO_MODEM_CTRL          1 /* Set the modem control register */
#define SIO_SET_FLOW_CTRL       2 /* Set flow control register */
#define SIO_SET_BAUD_RATE       3 /* Set baud rate */
#define SIO_SET_DATA            4 /* Set the data characteristics of the port */
#define SIO_POLL_MODEM_STATUS   5
#define SIO_SET_EVENT_CHAR      6
#define SIO_SET_ERROR_CHAR      7
#define SIO_SET_LATENCY_TIMER   9
#define SIO_GET_LATENCY_TIMER   10
#define SIO_SET_BITMODE         11
#define SIO_READ_PINS           12
#define SIO_READ_EEPROM         0x90
#define SIO_WRITE_EEPROM        0x91
#define SIO_ERASE_EEPROM        0x92

char *progname;
int verbose;
int trace;
int debug;

extern char *optarg;

/* Доступ к устройству через libusb. */
usb_dev_handle *usbdev;

/* Буфер для вывода-ввода в режиме sync bitbang. */
unsigned char output [128];
int output_len;

/*
 * Add one sample to send buffer.
 */
static void adapter_write (unsigned out_value)
{
    if (output_len >= (int) sizeof (output)) {
        fprintf (stderr, "adapter_write: buffer overflow\n");
        exit (-1);
    }
    output [output_len++] = out_value;
}

/*
 * Extract input data from bitbang buffer.
 */
static void adapter_read (unsigned offset, unsigned nbits, unsigned char *data)
{
    unsigned n;

    for (n=0; n<nbits; n++) {
        if (output [offset + n*2 + 1] & READ_TDO)
            data [n/8] |= 1 << (n & 7);
        else
            data [n/8] &= ~(1 << (n & 7));
    }
}

/*
 * Perform sync bitbang output/input transaction.
 * Befor call, an array output[] should be filled with data to send.
 * Counter output_len contains a number of bytes to send.
 * On return, received data are put back to array output[].
 */
static void adapter_send_recv (void)
{
    int bytes_to_write, bytes_written, n, txdone, rxdone;
    int empty_rxfifo, bytes_to_read, bytes_read;
    unsigned char reply [64];

    adapter_write (0);

    /* First two receive bytes contain modem and line status. */
    empty_rxfifo = sizeof(reply) - 2;

    /* Indexes in data buffer. */
    txdone = 0;
    rxdone = 0;
    while (rxdone < output_len) {
        /* Try to send as much as possible,
         * but avoid overflow of receive buffer.
         * Unfortunately, transfer sizes bigger that
         * 64 bytes cause hang ups. */
        bytes_to_write = 64;
        if (bytes_to_write > output_len - txdone)
            bytes_to_write = output_len - txdone;
        if (bytes_to_write > empty_rxfifo)
            bytes_to_write = empty_rxfifo;

        /* Write data. */
        bytes_written = 0;
        while (bytes_written < bytes_to_write) {
            if (debug)
                fprintf (stderr, "usb bulk write %d bytes\n",
                    bytes_to_write - bytes_written);

            n = usb_bulk_write (usbdev, IN_EP,
                (char*) output + txdone + bytes_written,
                bytes_to_write - bytes_written, 1000);
            if (n < 0) {
                fprintf (stderr, "usb bulk write failed, error %d\n", n);
                exit (-1);
            }
            /*if (n != bytes_to_write)
                fprintf (stderr, "usb bulk written %d bytes of %d",
                    n, bytes_to_write - bytes_written);*/
            bytes_written += n;
        }
        txdone += bytes_written;
        empty_rxfifo -= bytes_written;

        if (empty_rxfifo == 0 || txdone == output_len) {
            /* Get reply. */
            bytes_to_read = sizeof(reply) - empty_rxfifo - 2;
            bytes_read = 0;
            while (bytes_read < bytes_to_read) {
                n = usb_bulk_read (usbdev, OUT_EP,
                    (char*) reply,
                    bytes_to_read - bytes_read + 2, 2000);
                if (n < 0) {
                    fprintf (stderr, "usb bulk read failed\n");
                    exit (-1);
                }
                if (n != bytes_to_read + 2)
                    fprintf (stderr, "usb bulk read %d bytes of %d\n",
                        n, bytes_to_read - bytes_read + 2);
                else if (debug)
                    fprintf (stderr, "usb bulk read %d bytes\n", n);
                if (n > 2) {
                    /* Copy data. */
                    memcpy (output + rxdone, reply + 2, n - 2);
                    bytes_read += n;
                    rxdone += n - 2;
                }
            }
            empty_rxfifo = sizeof(reply) - 2;
        }
    }
    output_len = 0;
}

static void adapter_close (void)
{
    usb_release_interface (usbdev, 0);
    usb_close (usbdev);
}

/*
 * Инициализация адаптера FT232R.
 */
void adapter_open (void)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    usb_init();
    usb_find_busses();
    usb_find_devices();
    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == FT232R_VID &&
                dev->descriptor.idProduct == FT232R_PID)
                goto found;
        }
    }
    fprintf (stderr, "USB adapter not found: vid=%04x, pid=%04x\n",
        FT232R_VID, FT232R_PID);
    exit (1);
found:
    usbdev = usb_open (dev);
    if (! usbdev) {
        fprintf (stderr, "usb_open() failed\n");
        exit (1);
    }
#if ! defined (__CYGWIN32__) && ! defined (MINGW32)
    char driver_name [100];
    if (usb_get_driver_np (usbdev, 0, driver_name, sizeof(driver_name)) == 0) {
	if (usb_detach_kernel_driver_np (usbdev, 0) < 0) {
            printf("Failed to detach the %s kernel driver.\n", driver_name);
            usb_close (usbdev);
            exit (1);
	}
    }
#endif
    usb_claim_interface (usbdev, 0);

    /* Reset the ftdi device. */
    if (usb_control_msg (usbdev,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
        SIO_RESET, 0, 0, 0, 0, 1000) != 0) {
        if (errno == EPERM)
            fprintf (stderr, "Superuser privileges needed.\n");
        else
            fprintf (stderr, "FTDI reset failed\n");
failed: usb_release_interface (usbdev, 0);
        usb_close (usbdev);
        exit (1);
    }

    /* Sync bit bang mode. */
    if (usb_control_msg (usbdev,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
        SIO_SET_BITMODE, TCK | TDI | TMS | 0x400,
        0, 0, 0, 1000) != 0) {
        fprintf (stderr, "Can't set sync bitbang mode\n");
        goto failed;
    }

    /* Ровно 500 нсек между выдачами. */
    unsigned divisor = 0;
    unsigned char latency_timer = 1;
    int baud = (divisor == 0) ? 3000000 :
        (divisor == 1) ? 2000000 : 3000000 / divisor;
    fprintf (stderr, "Speed %d samples/sec\n", baud);

    /* Frequency divisor is 14-bit non-zero value. */
    if (usb_control_msg (usbdev,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
        SIO_SET_BAUD_RATE, divisor,
        0, 0, 0, 1000) != 0) {
        fprintf (stderr, "Can't set baud rate\n");
        goto failed;
    }

    if (usb_control_msg (usbdev,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
        SIO_SET_LATENCY_TIMER, latency_timer, 0, 0, 0, 1000) != 0) {
        fprintf (stderr, "unable to set latency timer\n");
        goto failed;
    }
    if (usb_control_msg (usbdev,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
        SIO_GET_LATENCY_TIMER, 0, 0, (char*) &latency_timer, 1, 1000) != 1) {
        fprintf (stderr, "unable to get latency timer\n");
        goto failed;
    }
    fprintf (stderr, "Latency timer: %u usec\n", latency_timer);
}

extern int optind;

void usage ()
{
    fprintf (stderr, "Dump Morse paddle state.\n");
    fprintf (stderr, "Usage:\n\t%s [-vtd] file...\n", progname);
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "\t-v\tverbose mode\n");
    fprintf (stderr, "\t-t\ttrace mode\n");
    fprintf (stderr, "\t-d\tdebug\n");
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

    if (argc != 0)
        usage ();

    adapter_open();

    // TODO
    for (;;) {
        unsigned char rx;

        adapter_write (0);
        adapter_write (0);
        adapter_write (0);
        adapter_send_recv();
        adapter_read (0, 1, &rx);
        printf ("%02x \r", rx);
        fflush (stdout);
    }
    adapter_close();
    return (0);
}
