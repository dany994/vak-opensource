/*
 * DyIO control utility.
 *
 * Copyright (C) 2015 Serge Vakulenko
 *
 * This file is distributed under the terms of the Apache License, Version 2.0.
 * See http://opensource.org/licenses/Apache-2.0 for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include "serial.h"

const char version[] = "1.0";
const char copyright[] = "Copyright (C) 2015 Serge Vakulenko";

char *progname;
int verbose;
int trace;
int dyio_debug;
uint8_t dyio_mac[6] = { 0 };
uint8_t dyio_reply[256];
int dyio_replylen;

#define PROTOCOL_VERSION 3

#define TYPE_STATUS     0x00    /* Synchronous, high priority, non state changing */
#define TYPE_GET        0x10    /* Synchronous, query for information, non state changing */
#define TYPE_POST       0x20    /* Synchronous, device state changing */
#define TYPE_CRITICAL   0x30    /* Synchronous, high priority, state changing */
#define TYPE_ASYNC      0x40    /* Asynchronous, high priority, state changing */

struct dyio_header {
    uint8_t proto;              /* Protocol revision */
    uint8_t mac[6];             /* MAC address of the device */
    uint8_t type;               /* Packet type */
    uint8_t namespace;          /* Low bit is direction flag */
    uint8_t datalen;            /* The length of data including the RPC */
    uint8_t hsum;               /* Header checksum */
    uint8_t rpc[4];             /* RPC call identifier */
};

void usage()
{
    fprintf(stderr, "DyIO usility, Version %s, %s\n", version, copyright);
    fprintf(stderr, "Usage:\n\t%s [-vtd] [-r count] file...\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t-v\tverbose mode\n");
    fprintf(stderr, "\t-t\ttrace mode\n");
    fprintf(stderr, "\t-d\tdebug\n");
    exit(-1);
}

/*
 * Send the command sequence and get back a response.
 */
static int send_receive(int type, int namespace, char *rpc,
    uint8_t *data, int datalen)
{
    struct dyio_header hdr;
    uint8_t *p;
    int len, i, got, sum, retry = 0;
again:
    /*
     * Prepare header and checksum.
     */
    hdr.proto     = PROTOCOL_VERSION;
    hdr.type      = type;
    hdr.namespace = namespace;
    hdr.datalen   = datalen + sizeof(hdr.rpc);
    memcpy(hdr.mac, dyio_mac, sizeof(hdr.mac));
    memcpy(hdr.rpc, rpc, sizeof(hdr.rpc));
    hdr.hsum = hdr.proto + hdr.mac[0] + hdr.mac[1] + hdr.mac[2] +
               hdr.mac[3] + hdr.mac[4] + hdr.mac[5] + hdr.type +
               hdr.namespace + hdr.datalen;

    /*
     * Send command.
     */
    if (dyio_debug > 1) {
        printf("send %x-%x-%x-%x-%x-%x-%x-%x-%x-[%u]-%x-'%c%c%c%c'",
            hdr.proto, hdr.mac[0], hdr.mac[1], hdr.mac[2],
            hdr.mac[3], hdr.mac[4], hdr.mac[5], hdr.type,
            hdr.namespace, hdr.datalen, hdr.hsum,
            hdr.rpc[0], hdr.rpc[1], hdr.rpc[2], hdr.rpc[3]);
        for (i=0; i<datalen; ++i)
            printf("-%x", data[i]);
        printf("\n");
    }
    if (serial_write((uint8_t*)&hdr, sizeof(hdr)) < 0) {
        fprintf(stderr, "dyio-send: header write error\n");
        exit(-1);
    }
    if (datalen > 0 && serial_write(data, datalen) < 0) {
        fprintf(stderr, "dyio-send: data write error\n");
        exit(-1);
    }

    /*
     * Get header.
     */
next:
    p = (uint8_t*) &hdr;
    len = 0;
    while (len < sizeof(hdr)) {
        got = serial_read(p, sizeof(hdr) - len);
        if (! got)
            return 0;

        p += got;
        len += got;
    }
    if (hdr.proto != PROTOCOL_VERSION) {
        /* Skip all incoming data. */
        unsigned char buf [300];

        if (retry)
            printf("got invalid header: %x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x\n",
                hdr.proto, hdr.mac[0], hdr.mac[1], hdr.mac[2],
                hdr.mac[3], hdr.mac[4], hdr.mac[5], hdr.type,
                hdr.namespace, hdr.datalen, hdr.hsum,
                hdr.rpc[0], hdr.rpc[1], hdr.rpc[2], hdr.rpc[3]);
//flush_input:
        serial_read(buf, sizeof(buf));
        if (! retry) {
            retry = 1;
            goto again;
        }
        return 0;
    }

    /*
     * Get response.
     */
    dyio_replylen = hdr.datalen - sizeof(hdr.rpc) + 1;
    p = dyio_reply;
    len = 0;
    while (len < dyio_replylen) {
        got = serial_read(p, dyio_replylen - len);
        if (! got)
            return 0;

        p += got;
        len += got;
    }
    if (dyio_debug > 1) {
        printf(" got %x-%x-%x-%x-%x-%x-%x-%x-%x-[%u]-%x-'%c%c%c%c'",
            hdr.proto, hdr.mac[0], hdr.mac[1], hdr.mac[2],
            hdr.mac[3], hdr.mac[4], hdr.mac[5], hdr.type,
            hdr.namespace, hdr.datalen, hdr.hsum,
            hdr.rpc[0], hdr.rpc[1], hdr.rpc[2], hdr.rpc[3]);
        for (i=0; i<dyio_replylen; ++i)
            printf("-%x", dyio_reply[i]);
        printf("\n");
    }

    /* Check sum. */
    sum = hdr.proto + hdr.mac[0] + hdr.mac[1] + hdr.mac[2] +
          hdr.mac[3] + hdr.mac[4] + hdr.mac[5] + hdr.type +
          hdr.namespace + hdr.datalen;
    if (sum != hdr.hsum) {
        printf("invalid reply checksum\n");
        //goto flush_input;
    }
    if (hdr.type == TYPE_ASYNC)
        goto next;
    return 1;
}

void dyio_connect(const char *devname)
{
    int retry_count;
    unsigned char response [12];

    /* Open serial port */
    if (serial_open(devname, 115200) < 0) {
        /* failed to open serial port */
        perror(devname);
        exit(-1);
    }

    /* Synchronize */
    retry_count = 0;
    for (;;) {
        if (send_receive(TYPE_GET, 0, "_png", 0, 0)) {
            if (memcmp(response, "xxxx", 4) == 0) {
                if (dyio_debug > 1)
                    printf("dyio-connect: OK\n");
                break;
            }
        }
        ++retry_count;
        if (dyio_debug > 1)
            printf("dyio-connect: error %d\n", retry_count);
        if (retry_count >= 3) {
            /* Bad reply or no device connected */
            return;
        }
    }
}

int main(int argc, char **argv)
{
    progname = *argv;
    for (;;) {
        switch (getopt(argc, argv, "vtd")) {
        case EOF:
            break;
        case 'v':
            ++verbose;
            continue;
        case 't':
            ++trace;
            continue;
        case 'd':
            ++dyio_debug;
            continue;
#if 0
        case 'r':
            count = strtol(optarg, 0, 0);
            continue;
#endif
        default:
            usage();
        }
        break;
    }
    argc -= optind;
    argv += optind;

    if (argc < 0)
        usage();

    dyio_connect("/dev/ttyACM0");

    return 0;
}
