/*
 * KMD: floppy disk controller for DVK
 *
 * Copyright (c) 2011, Serge Vakulenko
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING" for more details.
 */
#include "pdp11_defs.h"

extern uint16 *M;

#define KMD_SIZE        (800*1024)  /* disk size, bytes */

/*
 * CR register
 */
#define CR_GO           0000001     /* run command (write only) */
#define CR_CMD_MASK     0000036     /* command mask */
#define CR_CMD_RD       0000000     /* read */
#define CR_CMD_WR       0000002     /* write */
#define CR_CMD_RDM      0000004     /* read with mark */
#define CR_CMD_WRM      0000006     /* write with mark */
#define CR_CMD_RDTR     0000010     /* read track */
#define CR_CMD_RDID     0000012     /* read identifier */
#define CR_CMD_FORMAT   0000014     /* format track */
#define CR_CMD_SEEK     0000016     /* select track */
#define CR_CMD_SET      0000020     /* set parameters */
#define CR_CMD_RDERR    0000022     /* read error status */
#define CR_CMD_LOAD     0000036     /* boot */
#define CR_DONE         0000040     /* command finished (read only) */
#define CR_IE           0000100     /* interrupt enable */
#define CR_TR           0000200     /* transaction ready (read only) */
#define CR_ADDR(a)      ((a) >> 8 & 037400) /* address extension */
#define CR_INIT         0040000     /* initialization (write only) */
#define CR_ERR          0100000     /* command error (read only) */

/*
 * Hardware registers.
 */
int kmd_cr;             /* Control register */
int kmd_dr;             /* Data register */

t_stat kmd_event (UNIT *u);
t_stat kmd_rd (int32 *data, int32 PA, int32 access);
t_stat kmd_wr (int32 data, int32 PA, int32 access);
int32 kmd_inta (void);
t_stat kmd_reset (DEVICE *dptr);
t_stat kmd_boot (int32 unitno, DEVICE *dptr);
t_stat kmd_attach (UNIT *uptr, char *cptr);
t_stat kmd_detach (UNIT *uptr);

/*
 * KMD data structures
 *
 * kmd_unit     unit descriptors
 * kmd_reg      register list
 * kmd_dev      device descriptor
 */
UNIT kmd_unit [] = {
    { UDATA (kmd_event, UNIT_FIX+UNIT_ATTABLE+UNIT_ROABLE, KMD_SIZE) },
    { UDATA (kmd_event, UNIT_FIX+UNIT_ATTABLE+UNIT_ROABLE, KMD_SIZE) },
    { UDATA (kmd_event, UNIT_FIX+UNIT_ATTABLE+UNIT_ROABLE, KMD_SIZE) },
    { UDATA (kmd_event, UNIT_FIX+UNIT_ATTABLE+UNIT_ROABLE, KMD_SIZE) },
};

REG kmd_reg[] = {
    { "CR", &kmd_cr, DEV_RDX, 16, 0, 1 },
    { "DR", &kmd_dr, DEV_RDX, 16, 0, 1 },
    { 0 }
};

MTAB kmd_mod[] = {
    { 0 }
};

DIB kmd_dib = {
    IOBA_KMD, IOLN_KMD, &kmd_rd, &kmd_wr,
    1, IVCL (KMD), 0, { &kmd_inta }
};

DEVICE kmd_dev = {
    "KMD", kmd_unit, kmd_reg, kmd_mod,
    4,          /* #units */
    DEV_RDX,    /* address radix */
    T_ADDR_W,   /* address width */
    2,          /* addr increment */
    DEV_RDX,    /* data radix */
    16,         /* data width */
    NULL, NULL, &kmd_reset, &kmd_boot, &kmd_attach, &kmd_detach,
    &kmd_dib, DEV_DISABLE | DEV_UBUS | DEV_QBUS | DEV_DEBUG
};

/*
 * Output to console and log file: when enabled "cpu debug".
 * Appends newline.
 */
void kmd_debug (const char *fmt, ...)
{
    va_list args;
    extern FILE *sim_deb;

    va_start (args, fmt);
    vprintf (fmt, args);
    printf ("\r\n");
    va_end (args);
    if (sim_deb && sim_deb != stdout) {
        va_start (args, fmt);
        vfprintf (sim_deb, fmt, args);
        fprintf (sim_deb, "\n");
        fflush (sim_deb);
        va_end (args);
    }
}

/*
 * Reset routine
 */
t_stat kmd_reset (DEVICE *dptr)
{
    if (kmd_dev.dctrl)
        kmd_debug ("### KMD reset");
    kmd_cr = CR_DONE;
    kmd_dr = 0;
    sim_cancel (&kmd_unit[0]);
    sim_cancel (&kmd_unit[1]);
    sim_cancel (&kmd_unit[2]);
    sim_cancel (&kmd_unit[3]);
    return SCPE_OK;
}

t_stat kmd_attach (UNIT *u, char *cptr)
{
    t_stat s;

    s = attach_unit (u, cptr);
    if (s != SCPE_OK)
        return s;
    // TODO
    return SCPE_OK;
}

t_stat kmd_detach (UNIT *u)
{
    // TODO
    return detach_unit (u);
}

t_stat kmd_boot (int32 unitno, DEVICE *dptr)
{
#if 0
    int32 i;
    extern int32 saved_PC;
    extern uint16 *M;

    DIB *dibp = (DIB*) dptr->ctxt;

    for (i = 0; i < BOOT_LEN; i++)
        M [(BOOT_START >> 1) + i] = boot_rom[i];
    M [BOOT_UNIT >> 1] = unitno & 3;
    M [BOOT_CSR >> 1] = dibp->ba & DMASK;
    saved_PC = BOOT_ENTRY;
#endif
    return SCPE_OK;
}

void kmd_io ()
{
    static const char *opname[16] = {
        "read", "write", "read mark", "write mark",
        "read track", "read id", "format track", "seek",
        "set parameters", "read error status", "op24", "op26",
        "op30", "op32", "op34", "boot" };

    uint16 *param = M + (kmd_dr >> 1) + ((kmd_cr & 037400) << 7);
    int32 addr = param[1] | (param[0] & 0xff00) << 8;
    int diskno = param[0] & 3;
    int head = param[0] >> 2 & 1;
    int sector = param[2] & 0xff;
    int cyl = param[2] >> 8 & 0xff;
    int nbytes = param[3] << 1;
    UNIT *u = &kmd_unit [diskno];

    if (kmd_dev.dctrl) {
        //kmd_debug ("### KMD %s, CR %06o, DR %06o, params %06o %06o %06o %06o",
        //    opname [kmd_cr >> 1 & 15], kmd_cr, kmd_dr,
        //    param[0], param[1], param[2], param[3]);
        kmd_debug ("### KMD%d %s chs=%d/%d/%d, addr %06o, %d bytes",
            diskno, opname [kmd_cr >> 1 & 15],
            cyl, head, sector, addr, nbytes);
    }

    unsigned long seek = ((cyl * 2 + head) * 10 + sector - 1) * 512L;
    switch (kmd_cr & CR_CMD_MASK) {
    case CR_CMD_RD:             /* read */
        fseek (u->fileref, seek, SEEK_SET);
        if (sim_fread (&M[addr>>1], 1, nbytes, u->fileref) != nbytes) {
            /* Reading uninitialized media. */
            kmd_cr |= CR_ERR;
            return;
        }
        break;
    case CR_CMD_WR:             /* write */
        fseek (u->fileref, seek, SEEK_SET);
        sim_fwrite (&M[addr>>1], 1, nbytes, u->fileref);
        break;
    case CR_CMD_RDM:            /* read with mark */
    case CR_CMD_WRM:            /* write with mark */
    case CR_CMD_RDTR:           /* read track */
    case CR_CMD_RDID:           /* read identifier */
    case CR_CMD_FORMAT:         /* format track */
    case CR_CMD_SEEK:           /* select track */
    case CR_CMD_SET:            /* set parameters */
    case CR_CMD_RDERR:          /* read error status */
    case CR_CMD_LOAD:           /* boot */
        kmd_debug ("### KMD%d %s: operation not implemented",
            diskno, opname [kmd_cr >> 1 & 15]);
        return;
    }
    if (ferror (u->fileref))
        kmd_debug ("### KMD%d %s: i/o error",
            diskno, opname [kmd_cr >> 1 & 15]);
}

/*
 * I/O dispatch routines, I/O addresses 172140 - 172142
 *
 *  base + 0     CR      read/write
 *  base + 2     DR      read/write
 */
t_stat kmd_rd (int32 *data, int32 PA, int32 access)
{
    if (PA & 2) {
        /* Data register. */
        *data = kmd_dr;
        //if (kmd_dev.dctrl)
        //    kmd_debug ("### KMD DR -> %06o", kmd_dr);
    } else {
        /* Control register. */
        *data = kmd_cr;
        //if (kmd_dev.dctrl)
        //    kmd_debug ("### KMD CR -> %06o", kmd_cr);
    }
    return SCPE_OK;
}

t_stat kmd_wr (int32 data, int32 PA, int32 access)
{
    if (PA & 2) {
        /* Data register. */
        //if (kmd_dev.dctrl)
        //    kmd_debug ("### KMD DR := %06o", data);
        kmd_dr = data;
        if (kmd_cr & CR_TR) {
            /* Do real read/write. */
            kmd_cr &= ~CR_TR;
            kmd_io ();
            kmd_cr |= CR_DONE;
        }
    } else {
        /* Control register. */
        //if (kmd_dev.dctrl)
        //    kmd_debug ("### KMD CR := %06o", data);
        kmd_cr = (kmd_cr & (CR_DONE | CR_TR | CR_ERR)) |
            data & ~(CR_GO | CR_DONE | CR_TR | CR_INIT | CR_ERR);

        if (data & CR_INIT) {
            /* Reset comntroller. */
            kmd_reset (&kmd_dev);

        } else if (data & CR_GO) {
            /* Start new transaction. */
            kmd_cr &= ~(CR_DONE | CR_ERR);
            kmd_cr |= CR_TR;
        }
    }
    return SCPE_OK;
}

/*
 * Return interrupt vector
 */
int32 kmd_inta (void)
{
#if 0
    int32 i;
    MSC *ncp;
    DEVICE *dptr;
    DIB *dibp;

    for (i = 0; i < KMD_NUMCT; i++) {                        /* loop thru ctrl */
        ncp = kmd_ctxmap[i];                                /* get context */
        if (ncp->irq) {                                     /* ctrl int set? */
            dptr = kmd_devmap[i];                           /* get device */
            dibp = (DIB *) dptr->ctxt;                      /* get DIB */
            kmd_clrint (ncp);                               /* clear int req */
            return dibp->vec;                               /* return vector */
            }
        }
#endif
    return 0;                                               /* no intr req */
}

/*
 * Событие: закончен обмен с диском.
 * Устанавливаем флаг прерывания.
 */
t_stat kmd_event (UNIT *u)
{
    // TODO
    return SCPE_OK;
}
