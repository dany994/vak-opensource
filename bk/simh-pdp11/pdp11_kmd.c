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

#define KMD_SIZE        (800*1024)  /* disk size, bytes */

/*
 * CR register
 */
#define CR_GO           0000001     /* run command (write only) */
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

/*
 * I/O dispatch routines, I/O addresses 172140 - 172142
 *
 *  base + 0     CR      read/write
 *  base + 2     DR      read/write
 */
t_stat kmd_rd (int32 *data, int32 PA, int32 access)
{
    //if (kmd_dev.dctrl)
        kmd_debug ("### KMD read %06o", PA);
#if 0
    int32 cidx = kmd_map_pa ((uint32) PA);
    MSC *cp = kmd_ctxmap[cidx];
    DEVICE *dptr = kmd_devmap[cidx];

    if (cidx < 0)
        return SCPE_IERR;
    switch ((PA >> 1) & 01) {                           /* decode PA<1> */
    case 0:                                             /* IP */
        *data = 0;                                      /* reads zero */
        if (cp->csta == CST_S3_PPB)                     /* waiting for poll? */
            kmd_step4 (cp);
        else if (cp->csta == CST_UP) {                  /* if up */
            if (DEBUG_PRD (dptr))
                fprintf (sim_deb, ">>KMD%c: poll started, PC=%X\n",
                         'A' + cp->cnum, OLDPC);
            cp->pip = 1;                                /* poll host */
            sim_activate (dptr->units + KMD_QUEUE, kmd_qtime);
        }
        break;
    case 1:                                             /* SA */
        *data = cp->sa;
        break;
    }
#endif
    return SCPE_OK;
}

t_stat kmd_wr (int32 data, int32 PA, int32 access)
{
    //if (kmd_dev.dctrl)
        kmd_debug ("### KMD write %06o := %06o", PA, data);
#if 0
    int32 cidx = kmd_map_pa ((uint32) PA);
    MSC *cp = kmd_ctxmap[cidx];
    DEVICE *dptr = kmd_devmap[cidx];

    if (cidx < 0)
        return SCPE_IERR;
    switch ((PA >> 1) & 01) {                           /* decode PA<1> */
    case 0:                                             /* IP */
        kmd_reset (kmd_devmap[cidx]);                   /* init device */
        if (DEBUG_PRD (dptr))
            fprintf (sim_deb, ">>KMD%c: initialization started\n",
                     'A' + cp->cnum);
        break;

    case 1:                                             /* SA */
        cp->saw = data;
        if (cp->csta < CST_S4)                          /* stages 1-3 */
            sim_activate (dptr->units + KMD_QUEUE, kmd_itime);
        else if (cp->csta == CST_S4)                    /* stage 4 (fast) */
            sim_activate (dptr->units + KMD_QUEUE, kmd_itime4);
        break;
    }
#endif
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
