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
int dyio_debug;
uint8_t dyio_mac[6] = { 0 };
uint8_t dyio_reply[256];
int dyio_replylen;

struct dyio_header {
    uint8_t proto;              /* Protocol revision */
#define PROTO_VERSION   3

    uint8_t mac[6];             /* MAC address of the device */

    uint8_t type;               /* Packet type */
#define TYPE_STATUS     0x00    /* Synchronous, high priority, non state changing */
#define TYPE_GET        0x10    /* Synchronous, query for information, non state changing */
#define TYPE_POST       0x20    /* Synchronous, device state changing */
#define TYPE_CRITICAL   0x30    /* Synchronous, high priority, state changing */
#define TYPE_ASYNC      0x40    /* Asynchronous, high priority, state changing */

    uint8_t id;                 /* Namespace index; high bit is response flag */
#define ID_BCS_CORE     0       /* _png, _nms */
#define ID_BCS_RPC      1       /* _rpc, args */
#define ID_BCS_IO       2
#define ID_BCS_SETMODE  3
#define ID_DYIO         4
#define ID_BCS_PID      5
#define ID_BCS_DYPID    6
#define ID_BCS_SAFE     7
#define ID_RESPONSE     0x80

    uint8_t datalen;            /* The length of data including the RPC */
    uint8_t hsum;               /* Sum of previous bytes */
    uint8_t rpc[4];             /* RPC call identifier */
};

/*
 * Send the command sequence and get back a response.
 */
void dyio_call(int type, int namespace, char *rpc, uint8_t *data, int datalen)
{
    struct dyio_header hdr;
    uint8_t *p, sum;
    int len, i, got, retry = 0;

    /*
     * Prepare header and checksum.
     */
again:
    hdr.proto     = PROTO_VERSION;
    hdr.type      = type;
    hdr.id        = namespace;
    hdr.datalen   = datalen + sizeof(hdr.rpc);
    memcpy(hdr.mac, dyio_mac, sizeof(hdr.mac));
    memcpy(hdr.rpc, rpc, sizeof(hdr.rpc));
    hdr.hsum = hdr.proto + hdr.mac[0] + hdr.mac[1] + hdr.mac[2] +
               hdr.mac[3] + hdr.mac[4] + hdr.mac[5] + hdr.type +
               hdr.id + hdr.datalen;
    sum = hdr.rpc[0] + hdr.rpc[1] + hdr.rpc[2] + hdr.rpc[3];
    for (i=0; i<datalen; ++i)
        sum += data[i];

    /*
     * Send command.
     */
    if (dyio_debug > 1) {
        printf("send %x-%x-%x-%x-%x-%x-%x-%x-%x-[%u]-%x-'%c%c%c%c'",
            hdr.proto, hdr.mac[0], hdr.mac[1], hdr.mac[2],
            hdr.mac[3], hdr.mac[4], hdr.mac[5], hdr.type,
            hdr.id, hdr.datalen, hdr.hsum,
            hdr.rpc[0], hdr.rpc[1], hdr.rpc[2], hdr.rpc[3]);
        for (i=0; i<datalen; ++i)
            printf("-%x", data[i]);
        printf("-%x\n", sum);
    }
    if (serial_write((uint8_t*)&hdr, sizeof(hdr)) < 0) {
        fprintf(stderr, "dyio: header write error\n");
        exit(-1);
    }
    if (datalen > 0 && serial_write(data, datalen) < 0) {
        fprintf(stderr, "dyio: data write error\n");
        exit(-1);
    }
    if (serial_write(&sum, 1) < 0) {
        fprintf(stderr, "dyio: data sum write error\n");
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
        if (! got) {
            fprintf(stderr, "dyio: connection lost\n");
            exit(-1);
        }

        p += got;
        len += got;
    }
    if (hdr.proto != PROTO_VERSION) {
        /* Skip all incoming data. */
        unsigned char buf [300];

        if (retry)
            printf("got invalid header: %x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x\n",
                hdr.proto, hdr.mac[0], hdr.mac[1], hdr.mac[2],
                hdr.mac[3], hdr.mac[4], hdr.mac[5], hdr.type,
                hdr.id, hdr.datalen, hdr.hsum,
                hdr.rpc[0], hdr.rpc[1], hdr.rpc[2], hdr.rpc[3]);
flush_input:
        serial_read(buf, sizeof(buf));
        if (! retry) {
            retry = 1;
            goto again;
        }
        fprintf(stderr, "dyio: unable to synchronize\n");
        exit(-1);
    }

    /*
     * Get response.
     */
    dyio_replylen = hdr.datalen - sizeof(hdr.rpc);
    p = dyio_reply;
    len = 0;
    while (len <= dyio_replylen) {
        got = serial_read(p, dyio_replylen + 1 - len);
        if (! got) {
            fprintf(stderr, "dyio: connection lost\n");
            exit(-1);
        }

        p += got;
        len += got;
    }
    if (dyio_debug > 1) {
        printf(">>>> %x-%x-%x-%x-%x-%x-%x-%x-%x-[%u]-%x-'%c%c%c%c'",
            hdr.proto, hdr.mac[0], hdr.mac[1], hdr.mac[2],
            hdr.mac[3], hdr.mac[4], hdr.mac[5], hdr.type,
            hdr.id, hdr.datalen, hdr.hsum,
            hdr.rpc[0], hdr.rpc[1], hdr.rpc[2], hdr.rpc[3]);
        for (i=0; i<=dyio_replylen; ++i)
            printf("-%x", dyio_reply[i]);
        printf("\n");
    }

    /* Check header sum. */
    sum = hdr.proto + hdr.mac[0] + hdr.mac[1] + hdr.mac[2] +
          hdr.mac[3] + hdr.mac[4] + hdr.mac[5] + hdr.type +
          hdr.id + hdr.datalen;
    if (sum != hdr.hsum) {
        printf("dyio: invalid reply header sum = %02x, expected %02x \n", sum, hdr.hsum);
        goto flush_input;
    }

    /* Check data sum. */
    sum = hdr.rpc[0] + hdr.rpc[1] + hdr.rpc[2] + hdr.rpc[3];
    for (i=0; i<dyio_replylen; ++i)
        sum += dyio_reply[i];
    if (sum != dyio_reply[dyio_replylen]) {
        printf("dyio: invalid reply data sum = %02x, expected %02x \n", sum, dyio_reply[dyio_replylen]);
        goto flush_input;
    }

    if (! (hdr.id & ID_RESPONSE)) {
        printf("dyio: incorrect response flag\n");
        goto next;
    }

    if (hdr.type == TYPE_ASYNC) {
        goto next;
    }
}

/*
 * Establish a connection to the DyIO device.
 */
void dyio_connect(const char *devname)
{
    /* Open serial port */
    if (serial_open(devname, 115200) < 0) {
        /* failed to open serial port */
        perror(devname);
        exit(-1);
    }

    /* Ping the device. */
    dyio_call(TYPE_GET, ID_BCS_CORE, "_png", 0, 0);
    if (dyio_debug > 1)
        printf("dyio-connect: OK\n");
}

/*
 * Query and display information about the DyIO device.
 */
void dyio_info()
{
    int num_spaces, i;
    uint8_t query[1];

    /* Query the number of namespaces.
     * TODO: The reply length must be 1 byte, but it's 3 for some reason. */
    dyio_call(TYPE_GET, ID_BCS_CORE, "_nms", 0, 0);
    if (dyio_replylen < 1) {
        printf("dyio-info: incorrect _nms reply: length %u bytes\n", dyio_replylen);
        exit(-1);
    }
    num_spaces = dyio_reply[0];
    printf("Connected to DyIO device with %u namespaces.\n", num_spaces);

    /* Print info about every namespace. */
    for (i=0; i<num_spaces; i++) {
        query[0] = i;
        dyio_call(TYPE_GET, ID_BCS_CORE, "_nms", query, 1);
        if (dyio_replylen < 1) {
            printf("dyio-info: incorrect _nms[%u] reply\n", i);
            exit(-1);
        }
        printf("Namespace %u: %s\n", i, dyio_reply);
    }
}

void usage()
{
    printf("DyIO usility, Version %s, %s\n", version, copyright);
    printf("Usage:\n\t%s [-vd] portname\n", progname);
    printf("Options:\n");
    printf("\t-v\tverbose mode\n");
    printf("\t-d\tdebug\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    char *devname;

    progname = *argv;
    for (;;) {
        switch (getopt(argc, argv, "vd")) {
        case EOF:
            break;
        case 'v':
            ++verbose;
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

    if (argc != 1)
        usage();
    devname = argv[0];
    printf("Port name: %s\n", devname);

    dyio_connect(devname);

    dyio_info();

    return 0;
}
