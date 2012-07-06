/*
 * Interface to PIC32 microcontroller via JTAG or ICSP port.
 * Codelets borrowed from OpenOCD project.
 *
 * Copyright (C) 2012 Serge Vakulenko
 *
 * This file is part of PIC32PROXY project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "target.h"
#include "adapter.h"
#include "mips.h"
#include "pic32.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

struct _target_t {
    adapter_t   *adapter;
    const char  *cpu_name;
    unsigned    cpuid;
    unsigned    is_running;
    unsigned    flash_kbytes;
    unsigned    boot_kbytes;

    unsigned    reg [38];
#define REG_STATUS      32
#define REG_LO          33
#define REG_HI          34
#define REG_BADVADDR    35
#define REG_CAUSE       36
#define REG_DEPC        37
};

static const struct {
    unsigned devid;
    const char *name;
    unsigned flash_kbytes;
    unsigned boot_kbytes;
} pic32mx_dev[] = {
    {0x4A07053, "MX110F016B",  16,  3},
    {0x4A09053, "MX110F016C",  16,  3},
    {0x4A0B053, "MX110F016D",  16,  3},
    {0x4A06053, "MX120F032B",  32,  3},
    {0x4A08053, "MX120F032C",  32,  3},
    {0x4A0A053, "MX120F032D",  32,  3},
    {0x4D07053, "MX130F064B",  64,  3},
    {0x4D09053, "MX130F064C",  64,  3},
    {0x4D0B053, "MX130F064D",  64,  3},
    {0x4D06053, "MX150F128B", 128,  3},
    {0x4D08053, "MX150F128C", 128,  3},
    {0x4D0A053, "MX150F128D", 128,  3},
    {0x4A01053, "MX210F016B",  16,  3},
    {0x4A03053, "MX210F016C",  16,  3},
    {0x4A05053, "MX210F016D",  16,  3},
    {0x4A00053, "MX220F032B",  32,  3},
    {0x4A02053, "MX220F032C",  32,  3},
    {0x4A04053, "MX220F032D",  32,  3},
    {0x4D01053, "MX230F064B",  64,  3},
    {0x4D03053, "MX230F064C",  64,  3},
    {0x4D05053, "MX230F064D",  64,  3},
    {0x4D00053, "MX250F128B", 128,  3},
    {0x4D02053, "MX250F128C", 128,  3},
    {0x4D04053, "MX250F128D", 128,  3},
    {0x0938053, "MX360F512L", 512, 12},
    {0x0934053, "MX360F256L", 256, 12},
    {0x092D053, "MX340F128L", 128, 12},
    {0x092A053, "MX320F128L", 128, 12},
    {0x0916053, "MX340F512H", 512, 12},
    {0x0912053, "MX340F256H", 256, 12},
    {0x090D053, "MX340F128H", 128, 12},
    {0x090A053, "MX320F128H", 128, 12},
    {0x0906053, "MX320F064H",  64, 12},
    {0x0902053, "MX320F032H",  32, 12},
    {0x0978053, "MX460F512L", 512, 12},
    {0x0974053, "MX460F256L", 256, 12},
    {0x096D053, "MX440F128L", 128, 12},
    {0x0952053, "MX440F256H", 256, 12},
    {0x0956053, "MX440F512H", 512, 12},
    {0x094D053, "MX440F128H", 128, 12},
    {0x0942053, "MX420F032H",  32, 12},
    {0x4307053, "MX795F512L", 512, 12},
    {0x430E053, "MX795F512H", 512, 12},
    {0x4306053, "MX775F512L", 512, 12},
    {0x430D053, "MX775F512H", 512, 12},
    {0x4312053, "MX775F256L", 256, 12},
    {0x4303053, "MX775F256H", 256, 12},
    {0x4417053, "MX764F128L", 128, 12},
    {0x440B053, "MX764F128H", 128, 12},
    {0x4341053, "MX695F512L", 512, 12},
    {0x4325053, "MX695F512H", 512, 12},
    {0x4311053, "MX675F512L", 512, 12},
    {0x430C053, "MX675F512H", 512, 12},
    {0x4305053, "MX675F256L", 256, 12},
    {0x430B053, "MX675F256H", 256, 12},
    {0x4413053, "MX664F128L", 128, 12},
    {0x4407053, "MX664F128H", 128, 12},
    {0x4411053, "MX664F064L",  64, 12},
    {0x4405053, "MX664F064H",  64, 12},
    {0x430F053, "MX575F512L", 512, 12},
    {0x4309053, "MX575F512H", 512, 12},
    {0x4333053, "MX575F256L", 256, 12},
    {0x4317053, "MX575F256H", 256, 12},
    {0x440F053, "MX564F128L", 128, 12},
    {0x4403053, "MX564F128H", 128, 12},
    {0x440D053, "MX564F064L",  64, 12},
    {0x4401053, "MX564F064H",  64, 12},
    {0x4400053, "MX534F064H",  64, 12},
    {0x440C053, "MX534F064L",  64, 12},
    {0}
};

#if defined (__CYGWIN32__) || defined (MINGW32)
/*
 * Delay in milliseconds: Windows.
 */
#include <windows.h>

void mdelay (unsigned msec)
{
    Sleep (msec);
}
#else
/*
 * Delay in milliseconds: Unix.
 */
void mdelay (unsigned msec)
{
    usleep (msec * 1000);
}
#endif

/*
 * Connect to JTAG adapter.
 */
target_t *target_open ()
{
    target_t *t;

    t = calloc (1, sizeof (target_t));
    if (! t) {
        fprintf (stderr, "Out of memory\n");
        exit (-1);
    }
    t->cpu_name = "Unknown";

    /* Find adapter. */
    t->adapter = adapter_open_pickit ();
#ifdef USE_MPSSE
    if (! t->adapter)
        t->adapter = adapter_open_mpsse ();
#endif
    if (! t->adapter) {
        fprintf (stderr, "No target found.\n");
        exit (-1);
    }

    /* Check CPU identifier. */
    t->cpuid = t->adapter->get_idcode (t->adapter);
    if (t->cpuid == 0 || t->cpuid == ~0) {
        /* Device not detected. */
        fprintf (stderr, "Bad CPUID=%08x.\n", t->cpuid);
        t->adapter->close (t->adapter, 0);
        exit (1);
    }
    unsigned i;
    for (i=0; (t->cpuid ^ pic32mx_dev[i].devid) & 0x0fffffff; i++) {
        if (pic32mx_dev[i].devid == 0) {
            /* Device not detected. */
            fprintf (stderr, "Unknown CPUID=%08x.\n", t->cpuid);
            t->adapter->close (t->adapter, 0);
            exit (1);
        }
    }
    t->cpu_name = pic32mx_dev[i].name;
    t->flash_kbytes = pic32mx_dev[i].flash_kbytes;
    t->boot_kbytes = pic32mx_dev[i].boot_kbytes;
    t->is_running = 1;

    printf ("Processor: %s\n", t->cpu_name);
    return t;
}

/*
 * Close the device.
 */
void target_close (target_t *t, int power_on)
{
    t->adapter->close (t->adapter, power_on);
}

const char *target_cpu_name (target_t *t)
{
    return t->cpu_name;
}

unsigned target_idcode (target_t *t)
{
    return t->cpuid;
}

/*
 * Translate virtual to physical address.
 */
static unsigned virt_to_phys (unsigned addr)
{
    if (addr >= 0x80000000 && addr < 0xA0000000)
        return addr - 0x80000000;
    if (addr >= 0xA0000000 && addr < 0xC0000000)
        return addr - 0xA0000000;
    return addr;
}

/*
 * Save the state of CPU.
 */
static void target_save_state (target_t *t)
{
    static const unsigned code[] = {                    /* start: */
        MIPS32_MTC0 (2, 31, 0),                         /* move $2 to COP0 DeSave */
        MIPS32_LUI (2, UPPER16(MIPS32_PRACC_PARAM_OUT)),/* $2 = MIPS32_PRACC_PARAM_OUT */
        MIPS32_ORI (2, 2, LOWER16(MIPS32_PRACC_PARAM_OUT)),
        MIPS32_SW (0, 0*4, 2),                          /* sw $0,0*4($2) */
        MIPS32_SW (1, 1*4, 2),                          /* sw $1,1*4($2) */
        MIPS32_SW (15, 15*4, 2),                        /* sw $15,15*4($2) */
        MIPS32_MFC0 (2, 31, 0),                         /* move COP0 DeSave to $2 */
        MIPS32_MTC0 (15, 31, 0),                        /* move $15 to COP0 DeSave */
        MIPS32_LUI (15, UPPER16(MIPS32_PRACC_STACK)),   /* $15 = MIPS32_PRACC_STACK */
        MIPS32_ORI (15, 15, LOWER16(MIPS32_PRACC_STACK)),
        MIPS32_SW (1, 0, 15),                           /* sw $1,($15) */
        MIPS32_SW (2, 0, 15),                           /* sw $2,($15) */
        MIPS32_LUI (1, UPPER16(MIPS32_PRACC_PARAM_OUT)),/* $1 = MIPS32_PRACC_PARAM_OUT */
        MIPS32_ORI (1, 1, LOWER16(MIPS32_PRACC_PARAM_OUT)),
        MIPS32_SW (2, 2*4, 1),                          /* sw $2,2*4($1) */
        MIPS32_SW (3, 3*4, 1),                          /* sw $3,3*4($1) */
        MIPS32_SW (4, 4*4, 1),                          /* sw $4,4*4($1) */
        MIPS32_SW (5, 5*4, 1),                          /* sw $5,5*4($1) */
        MIPS32_SW (6, 6*4, 1),                          /* sw $6,6*4($1) */
        MIPS32_SW (7, 7*4, 1),                          /* sw $7,7*4($1) */
        MIPS32_SW (8, 8*4, 1),                          /* sw $8,8*4($1) */
        MIPS32_SW (9, 9*4, 1),                          /* sw $9,9*4($1) */
        MIPS32_SW (10, 10*4, 1),                        /* sw $10,10*4($1) */
        MIPS32_SW (11, 11*4, 1),                        /* sw $11,11*4($1) */
        MIPS32_SW (12, 12*4, 1),                        /* sw $12,12*4($1) */
        MIPS32_SW (13, 13*4, 1),                        /* sw $13,13*4($1) */
        MIPS32_SW (14, 14*4, 1),                        /* sw $14,14*4($1) */
        MIPS32_SW (16, 16*4, 1),                        /* sw $16,16*4($1) */
        MIPS32_SW (17, 17*4, 1),                        /* sw $17,17*4($1) */
        MIPS32_SW (18, 18*4, 1),                        /* sw $18,18*4($1) */
        MIPS32_SW (19, 19*4, 1),                        /* sw $19,19*4($1) */
        MIPS32_SW (20, 20*4, 1),                        /* sw $20,20*4($1) */
        MIPS32_SW (21, 21*4, 1),                        /* sw $21,21*4($1) */
        MIPS32_SW (22, 22*4, 1),                        /* sw $22,22*4($1) */
        MIPS32_SW (23, 23*4, 1),                        /* sw $23,23*4($1) */
        MIPS32_SW (24, 24*4, 1),                        /* sw $24,24*4($1) */
        MIPS32_SW (25, 25*4, 1),                        /* sw $25,25*4($1) */
        MIPS32_SW (26, 26*4, 1),                        /* sw $26,26*4($1) */
        MIPS32_SW (27, 27*4, 1),                        /* sw $27,27*4($1) */
        MIPS32_SW (28, 28*4, 1),                        /* sw $28,28*4($1) */
        MIPS32_SW (29, 29*4, 1),                        /* sw $29,29*4($1) */
        MIPS32_SW (30, 30*4, 1),                        /* sw $30,30*4($1) */
        MIPS32_SW (31, 31*4, 1),                        /* sw $31,31*4($1) */

        MIPS32_MFC0 (2, 12, 0),                         /* move status to $2 */
        MIPS32_SW (2, 32*4, 1),                         /* sw $2,32*4($1) */
        MIPS32_MFLO (2),                                /* move lo to $2 */
        MIPS32_SW (2, 33*4, 1),                         /* sw $2,33*4($1) */
        MIPS32_MFHI (2),                                /* move hi to $2 */
        MIPS32_SW (2, 34*4, 1),                         /* sw $2,34*4($1) */
        MIPS32_MFC0 (2, 8, 0),                          /* move badvaddr to $2 */
        MIPS32_SW (2, 35*4, 1),                         /* sw $2,35*4($1) */
        MIPS32_MFC0 (2, 13, 0),                         /* move cause to $2 */
        MIPS32_SW (2, 36*4, 1),                         /* sw $2,36*4($1) */
        MIPS32_MFC0 (2, 24, 0),                         /* move depc (pc) to $2 */
        MIPS32_SW (2, 37*4, 1),                         /* sw $2,37*4($1) */

        MIPS32_LW (2, 0, 15),                           /* lw $2,($15) */
        MIPS32_LW (1, 0, 15),                           /* lw $1,($15) */
        MIPS32_B (NEG16(58)),                           /* b start */
        MIPS32_MFC0 (15, 31, 0),                        /* move COP0 DeSave to $15 */
    };

fprintf (stderr, "save_state()\n");
    t->adapter->exec (t->adapter, ARRAY_SIZE(code), code,
        0, 0, ARRAY_SIZE(t->reg), t->reg, 1);
}

/*
 * Restore the state of CPU.
 */
static void target_restore_state (target_t *t)
{
    /* Nothing to do. */
}

void target_stop (target_t *t)
{
fprintf (stderr, "target_stop()\n");
    if (! t->is_running)
        return;
    t->adapter->stop_cpu (t->adapter);
    t->is_running = 0;
    target_save_state (t);
}

void target_step (target_t *t)
{
    // TODO
    fprintf (stderr, "TODO: target_step()\n");
}

void target_resume (target_t *t)
{
    static const unsigned code[] = {
        MIPS32_DRET,                            /* return from debug mode */
    };

fprintf (stderr, "target_resume()\n");
    if (t->is_running)
        return;
    target_restore_state (t);
    t->is_running = 1;
    t->adapter->exec (t->adapter, ARRAY_SIZE(code), code,
        0, 0, 0, 0, 0);
}

void target_run (target_t *t, unsigned addr)
{
    // TODO
    fprintf (stderr, "TODO: target_run (addr = %08x)\n", addr);
}

void target_restart (target_t *t)
{
    // TODO
    fprintf (stderr, "TODO: target_restart()\n");
}

int target_is_stopped (target_t *t, int *is_aborted)
{
fprintf (stderr, "target_is_stopped()\n");
    *is_aborted = 0;

    /* Tiny delay. */
    mdelay (100);
    if (! t->adapter->cpu_stopped (t->adapter))
        return 0;

    /* Stopped. */
    if (t->is_running) {
        //t->adapter->stop_cpu (t->adapter);
        t->is_running = 0;
        target_save_state (t);
    }
#if 0
    /* BREAKD instruction detected. */
    if (t->adapter->oscr & OSCR_SWO)
        *is_aborted = 1;
#endif
    return 1;
}

/*
 * Read a register:
 *      0-31  - GPRs
 *      32    - CP0 Status
 *      33    - LO
 *      34    - HI
 *      35    - CP0 BadVAddr
 *      36    - CP0 Cause
 *      37    - PC
 *      38-69 - FPRs
 *      70    - FCSR
 *      71    - FIR
 */
unsigned target_read_register (target_t *t, unsigned regno)
{
    //fprintf (stderr, "target_read_register (regno = %u)\n", regno);
    switch (regno) {
    case 0 ... 31:              /* general purpose registers */
        return t->reg [regno];
    case 32:                    /* CP0 Status */
        return t->reg [REG_STATUS];
    case 33:                    /* LO */
        return t->reg [REG_LO];
    case 34:                    /* HI */
        return t->reg [REG_HI];
    case 35:                    /* CP0 BadVAddr */
        return t->reg [REG_BADVADDR];
    case 36:                    /* CP0 Cause */
        return t->reg [REG_CAUSE];
    case 37:                    /* PC */
        return t->reg [REG_DEPC];
    }
    return 0;
}

void target_write_register (target_t *t, unsigned regno, unsigned val)
{
    // TODO
    fprintf (stderr, "TODO: target_write_register (regno = %u, val = %08x)\n", regno, val);
}

void target_add_break (target_t *t, unsigned addr, int type)
{
    // TODO
    fprintf (stderr, "TODO: target_add_break (addr = %u, type = %d)\n", addr, type);
}

void target_remove_break (target_t *t, unsigned addr)
{
    // TODO
    fprintf (stderr, "TODO: target_remove_break (addr = %08x)\n", addr);
}

/*
 * Read data from memory.
 */
void target_read_block (target_t *t, unsigned addr,
    unsigned nwords, unsigned *data)
{
#if 0
    if (! t->adapter->read_data) {
        printf ("\nData reading not supported by the adapter.\n");
        exit (1);
    }
#endif
    addr = virt_to_phys (addr);
fprintf (stderr, "TODO: target_read_block (addr = %x, nwords = %d)\n", addr, nwords);
#if 0
    while (nwords > 0) {
        unsigned n = nwords;
        if (n > 256)
            n = 256;
        t->adapter->read_data (t->adapter, addr, 256, data);
        addr += n<<2;
        data += n;
        nwords -= n;
    }
    //fprintf (stderr, "    done (addr = %x)\n", addr);
#endif
}

unsigned target_read_word (target_t *t, unsigned addr)
{
    unsigned word;

    //word = t->adapter->read_word (t->adapter, addr);
    word = ~0;
fprintf (stderr, "TODO: target_read_word (addr = %08x) -> %08x\n", addr, word);
    return word;
}

void target_write_word (target_t *t, unsigned addr, unsigned word)
{
    // TODO
    fprintf (stderr, "TODO: target_write_word (addr = %08x, word = %08x)\n", addr, word);
}

void target_write_block (target_t *t, unsigned addr,
    unsigned nwords, unsigned *data)
{
    // TODO
    fprintf (stderr, "TODO: target_write_block (addr = %08x, nwords = %u)\n", addr, nwords);
}
