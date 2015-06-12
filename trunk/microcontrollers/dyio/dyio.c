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
uint8_t dyio_reply_mac[6];
uint8_t dyio_reply[256];
int dyio_replylen;

struct dyio_header {
    uint8_t proto;              /* Protocol revision */
#define PROTO_VERSION   3

    uint8_t mac[6];             /* MAC address of the device */

    uint8_t type;               /* Packet type */
#define PKT_STATUS      0x00    /* Synchronous, high priority, non state changing */
#define PKT_GET         0x10    /* Synchronous, query for information, non state changing */
#define PKT_POST        0x20    /* Synchronous, device state changing */
#define PKT_CRITICAL    0x30    /* Synchronous, high priority, state changing */
#define PKT_ASYNC       0x40    /* Asynchronous, high priority, state changing */

    uint8_t id;                 /* Namespace index; high bit is response flag */
#define ID_BCS_CORE     0       /* _png, _nms */
#define ID_BCS_RPC      1       /* _rpc, args */
#define ID_BCS_IO       2       /* asyn, cchn, gacm, gacv, gchc, gchm,
                                 * gchv, gcml, sacv, schv, strm */
#define ID_BCS_SETMODE  3       /* schm, sacm */
#define ID_DYIO         4       /* _mac, _pwr, _rev */
#define ID_BCS_PID      5       /* acal, apid, cpdv, cpid, gpdc,
                                 * kpid, _pid, rpid, _vpd */
#define ID_BCS_DYPID    6       /* dpid */
#define ID_BCS_SAFE     7       /* safe */
#define ID_RESPONSE     0x80

    uint8_t datalen;            /* The length of data including the RPC */
    uint8_t hsum;               /* Sum of previous bytes */
    uint8_t rpc[4];             /* RPC call identifier */
};

/*
 * Types of method parameters.
 */
#define TYPE_I08            8   /* 8 bit integer */
#define TYPE_I16            16  /* 16 bit integer */
#define TYPE_I32            32  /* 32 bit integer */
#define TYPE_STR            37  /* first byte is number of values, next is byte values */
#define TYPE_I32STR         38  /* first byte is number of values, next is 32-bit values */
#define TYPE_ASCII          39  /* ASCII string, null terminated */
#define TYPE_FIXED100       41  /* float */
#define TYPE_FIXED1K        42  /* float */
#define TYPE_BOOL           43  /* a boolean value */
#define TYPE_FIXED1K_STR    44  /* first byte is number of values, next is floats */

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
    memcpy(dyio_reply_mac, hdr.mac, sizeof(hdr.mac));

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

    if (hdr.type == PKT_ASYNC) {
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
        exit(-1);
    }

    /* Ping the device. */
    dyio_call(PKT_GET, ID_BCS_CORE, "_png", 0, 0);
    if (dyio_debug > 1)
        printf("dyio-connect: OK\n");

    printf("DyIO device address: %02x-%02x-%02x-%02x-%02x-%02x\n",
        dyio_reply_mac[0], dyio_reply_mac[1], dyio_reply_mac[2],
        dyio_reply_mac[3], dyio_reply_mac[4], dyio_reply_mac[5]);
}

static const char *pkt_name(int type)
{
    switch (type) {
    case PKT_STATUS:    return "STATUS";
    case PKT_GET:       return "GET";
    case PKT_POST:      return "POST";
    case PKT_CRITICAL:  return "CRITICAL";
    case PKT_ASYNC:     return "ASYNC";
    default:            return "UNKNOWN";
    }
}

static void print_args(int nargs, uint8_t *arg)
{
    int i;

    for (i=0; i<nargs; i++) {
        if (i)
            printf(", ");

        switch (arg[i]) {
        case TYPE_I08:          printf("byte");     break;
        case TYPE_I16:          printf("int16");    break;
        case TYPE_I32:          printf("int");      break;
        case TYPE_STR:          printf("byte[]");   break;
        case TYPE_I32STR:       printf("int[]");    break;
        case TYPE_ASCII:        printf("asciiz");   break;
        case TYPE_FIXED100:     printf("f100");     break;
        case TYPE_FIXED1K:      printf("fixed");    break;
        case TYPE_BOOL:         printf("bool");     break;
        case TYPE_FIXED1K_STR:  printf("fixed[]");  break;
        default:                printf("unknown");  break;
        }
    }
}

/*
 * Query and display information about the DyIO device.
 */
void dyio_info()
{
    int num_spaces, ns, num_methods, m, num_args, num_resp;
    int query_type, resp_type;
    uint8_t query[2], *args, *resp;
    char rpc[5];

    /* Print firmware revision. */
    dyio_call(PKT_GET, ID_DYIO, "_rev", 0, 0);
    if (dyio_replylen < 6) {
        printf("dyio-info: incorrect _rev reply: length %u bytes\n", dyio_replylen);
        exit(-1);
    }
    printf("Firmware Revision %u.%u.%u\n",
        dyio_reply[0], dyio_reply[1], dyio_reply[2]);

    /* Query the number of namespaces.
     * TODO: The reply length must be 1 byte, but it's 3 for some reason. */
    dyio_call(PKT_GET, ID_BCS_CORE, "_nms", 0, 0);
    if (dyio_replylen < 1) {
        printf("dyio-info: incorrect _nms reply: length %u bytes\n", dyio_replylen);
        exit(-1);
    }
    num_spaces = dyio_reply[0];

    /* Print info about every namespace. */
    for (ns=0; ns<num_spaces; ns++) {
        query[0] = ns;
        dyio_call(PKT_GET, ID_BCS_CORE, "_nms", query, 1);
        if (dyio_replylen < 1) {
            printf("dyio-info: incorrect _nms[%u] reply\n", ns);
            exit(-1);
        }
        printf("Namespace %u: %s\n", ns, dyio_reply);

        /* Print available methods. */
        num_methods = 1;
        for (m=0; m<num_methods; m++) {
            /* Get method name (RPC). */
            query[0] = ns;
            query[1] = m;
            dyio_call(PKT_GET, ID_BCS_RPC, "_rpc", query, 2);
            if (dyio_replylen < 7) {
                printf("dyio-info: incorrect _rpc[%u] reply\n", ns);
                exit(-1);
            }
            num_methods = dyio_reply[2];
            rpc[0] = dyio_reply[3];
            rpc[1] = dyio_reply[4];
            rpc[2] = dyio_reply[5];
            rpc[3] = dyio_reply[6];
            rpc[4] = 0;

            /* Get method args. */
            query[0] = ns;
            query[1] = m;
            dyio_call(PKT_GET, ID_BCS_RPC, "args", query, 2);
            if (dyio_replylen < 6) {
                printf("dyio-info: incorrect args[%u] reply\n", ns);
                exit(-1);
            }
            query_type = dyio_reply[2];
            num_args = dyio_reply[3];
            args = &dyio_reply[4];
            resp_type = dyio_reply[4 + num_args];
            num_resp = dyio_reply[5 + num_args];
            resp = &dyio_reply[6 + num_args];

            printf("    %s %s(", rpc, pkt_name(query_type));
            print_args(num_args, args);
            printf(") -> %s(", pkt_name(resp_type));
            print_args(num_resp, resp);
            printf(")\n");
        }
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
