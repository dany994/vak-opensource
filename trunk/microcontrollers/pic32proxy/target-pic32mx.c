/*
 * Interface to PIC32 microcontroller via JTAG or ICSP port.
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
#include "pic32.h"

struct _target_t {
    adapter_t   *adapter;
    const char  *cpu_name;
    unsigned    cpuid;
    unsigned    flash_kbytes;
    unsigned    boot_kbytes;
    unsigned    pe;
};

static const struct {
    unsigned devid;
    const char *name;
    unsigned flash_kbytes;
    unsigned boot_kbytes;
    unsigned pe;
} pic32mx_dev[] = {
    {0x4A07053, "MX110F016B",  16,  3, 1},
    {0x4A09053, "MX110F016C",  16,  3, 1},
    {0x4A0B053, "MX110F016D",  16,  3, 1},
    {0x4A06053, "MX120F032B",  32,  3, 1},
    {0x4A08053, "MX120F032C",  32,  3, 1},
    {0x4A0A053, "MX120F032D",  32,  3, 1},
    {0x4D07053, "MX130F064B",  64,  3, 1},
    {0x4D09053, "MX130F064C",  64,  3, 1},
    {0x4D0B053, "MX130F064D",  64,  3, 1},
    {0x4D06053, "MX150F128B", 128,  3, 1},
    {0x4D08053, "MX150F128C", 128,  3, 1},
    {0x4D0A053, "MX150F128D", 128,  3, 1},
    {0x4A01053, "MX210F016B",  16,  3, 1},
    {0x4A03053, "MX210F016C",  16,  3, 1},
    {0x4A05053, "MX210F016D",  16,  3, 1},
    {0x4A00053, "MX220F032B",  32,  3, 1},
    {0x4A02053, "MX220F032C",  32,  3, 1},
    {0x4A04053, "MX220F032D",  32,  3, 1},
    {0x4D01053, "MX230F064B",  64,  3, 1},
    {0x4D03053, "MX230F064C",  64,  3, 1},
    {0x4D05053, "MX230F064D",  64,  3, 1},
    {0x4D00053, "MX250F128B", 128,  3, 1},
    {0x4D02053, "MX250F128C", 128,  3, 1},
    {0x4D04053, "MX250F128D", 128,  3, 1},
    {0x0938053, "MX360F512L", 512, 12, 0},
    {0x0934053, "MX360F256L", 256, 12, 0},
    {0x092D053, "MX340F128L", 128, 12, 0},
    {0x092A053, "MX320F128L", 128, 12, 0},
    {0x0916053, "MX340F512H", 512, 12, 0},
    {0x0912053, "MX340F256H", 256, 12, 0},
    {0x090D053, "MX340F128H", 128, 12, 0},
    {0x090A053, "MX320F128H", 128, 12, 0},
    {0x0906053, "MX320F064H",  64, 12, 0},
    {0x0902053, "MX320F032H",  32, 12, 0},
    {0x0978053, "MX460F512L", 512, 12, 0},
    {0x0974053, "MX460F256L", 256, 12, 0},
    {0x096D053, "MX440F128L", 128, 12, 0},
    {0x0952053, "MX440F256H", 256, 12, 0},
    {0x0956053, "MX440F512H", 512, 12, 0},
    {0x094D053, "MX440F128H", 128, 12, 0},
    {0x0942053, "MX420F032H",  32, 12, 0},
    {0x4307053, "MX795F512L", 512, 12, 0},
    {0x430E053, "MX795F512H", 512, 12, 0},
    {0x4306053, "MX775F512L", 512, 12, 0},
    {0x430D053, "MX775F512H", 512, 12, 0},
    {0x4312053, "MX775F256L", 256, 12, 0},
    {0x4303053, "MX775F256H", 256, 12, 0},
    {0x4417053, "MX764F128L", 128, 12, 0},
    {0x440B053, "MX764F128H", 128, 12, 0},
    {0x4341053, "MX695F512L", 512, 12, 0},
    {0x4325053, "MX695F512H", 512, 12, 0},
    {0x4311053, "MX675F512L", 512, 12, 0},
    {0x430C053, "MX675F512H", 512, 12, 0},
    {0x4305053, "MX675F256L", 256, 12, 0},
    {0x430B053, "MX675F256H", 256, 12, 0},
    {0x4413053, "MX664F128L", 128, 12, 0},
    {0x4407053, "MX664F128H", 128, 12, 0},
    {0x4411053, "MX664F064L",  64, 12, 0},
    {0x4405053, "MX664F064H",  64, 12, 0},
    {0x430F053, "MX575F512L", 512, 12, 0},
    {0x4309053, "MX575F512H", 512, 12, 0},
    {0x4333053, "MX575F256L", 256, 12, 0},
    {0x4317053, "MX575F256H", 256, 12, 0},
    {0x440F053, "MX564F128L", 128, 12, 0},
    {0x4403053, "MX564F128H", 128, 12, 0},
    {0x440D053, "MX564F064L",  64, 12, 0},
    {0x4401053, "MX564F064H",  64, 12, 0},
    {0x4400053, "MX534F064H",  64, 12, 0},
    {0x440C053, "MX534F064L",  64, 12, 0},
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
    t->pe = pic32mx_dev[i].pe;

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
 * Read data from memory.
 */
void target_read_block (target_t *t, unsigned addr,
    unsigned nwords, unsigned *data)
{
    if (! t->adapter->read_data) {
        printf ("\nData reading not supported by the adapter.\n");
        exit (1);
    }

    addr = virt_to_phys (addr);
    //fprintf (stderr, "target_read_block (addr = %x, nwords = %d)\n", addr, nwords);
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
}

unsigned target_read_word (target_t *t, unsigned addr)
{
    // TODO
    fprintf (stderr, "TODO: target_read_word (addr = %08x)\n", addr);
    return ~0;
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

void target_stop (target_t *t)
{
    // TODO
    fprintf (stderr, "TODO: target_stop()\n");
}

void target_step (target_t *t)
{
    // TODO
    fprintf (stderr, "TODO: target_step()\n");
}

void target_resume (target_t *t)
{
    // TODO
    fprintf (stderr, "TODO: target_resume()\n");
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
    // TODO
    fprintf (stderr, "TODO: target_is_stopped()\n");
    return 1;
}

unsigned target_read_register (target_t *t, unsigned regno)
{
    // TODO
    fprintf (stderr, "TODO: target_read_register (regno = %u)\n", regno);
    return ~0;
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
