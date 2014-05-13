/*
 * Simulate the peripherals of PIC32 microcontroller.
 *
 * Copyright (C) 2014 Serge Vakulenko, <serge@vak.ru>
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
#include "globals.h"
#include "pic32mx.h"

#define STORAGE(name) case name: *namep = #name; break
#define READONLY(name) case name: *namep = #name; goto readonly
#define WRITEOP(name) case name: *namep = #name; goto op_##name;\
                      case name+4: *namep = #name"CLR"; goto op_##name;\
                      case name+8: *namep = #name"SET"; goto op_##name;\
                      case name+12: *namep = #name"INV"; op_##name: \
                      *bufp = write_op (*bufp, data, address)

#define VALUE(name) (name & 0x80000 ? iomem2 : iomem) [(name & 0xffff) >> 2]

static unsigned *iomem;         // image of I/O area
static unsigned *iomem2;        // image of second I/O area

void update_irq_flag()
{
#if 0
    cpu_mips_t *cpu = pic32->vm->boot_cpu;
    int vector, level, irq, n, v;

    /* Assume no interrupts pending. */
    cpu->irq_cause = 0;
    cpu->irq_pending = 0;
    pic32->intstat = 0;

    if ((pic32->ifs[0] & pic32->iec[0]) ||
        (pic32->ifs[1] & pic32->iec[1]) ||
        (pic32->ifs[2] & pic32->iec[2]))
    {
        /* Find the most prioritive pending interrupt,
         * it's vector and level. */
        vector = 0;
        level = 0;
        for (irq=0; irq<sizeof(irq_to_vector)/sizeof(int); irq++) {
            n = irq >> 5;
            if ((pic32->ifs[n] & pic32->iec[n]) >> (irq & 31) & 1) {
                /* Interrupt is pending. */
                v = irq_to_vector [irq];
                if (v < 0)
                    continue;
                if (pic32->ivprio[v] > level) {
                    vector = v;
                    level = pic32->ivprio[v];
                }
            }
        }
        pic32->intstat = vector | (level << 8);

        cpu->irq_cause = level << 10;
/*printf ("-- vector = %d, level = %d\n", vector, level);*/
    }
/*else printf ("-- no irq pending\n");*/

    mips_update_irq_flag (cpu);
#endif
}

/*
 * Perform an assign/clear/set/invert operation.
 */
static inline unsigned write_op (a, b, op)
{
    switch (op & 0xc) {
    case 0x0: a = b;   break;   // Assign
    case 0x4: a &= ~b; break;   // Clear
    case 0x8: a |= b;  break;   // Set
    case 0xc: a ^= b;  break;   // Invert
    }
    return a;
}

unsigned io_read32 (unsigned address, unsigned *bufp, const char **namep)
{
    switch (address) {
    /*
     * Bus matrix control registers.
     */
    STORAGE (BMXCON);           // Bus Mmatrix Control
    STORAGE (BMXDKPBA);         // Data RAM kernel program base address
    STORAGE (BMXDUDBA);         // Data RAM user data base address
    STORAGE (BMXDUPBA);         // Data RAM user program base address
    STORAGE (BMXPUPBA);         // Program Flash user program base address
    STORAGE (BMXDRMSZ);         // Data RAM memory size
    STORAGE (BMXPFMSZ);         // Program Flash memory size
    STORAGE (BMXBOOTSZ);        // Boot Flash size

    /*
     * Interrupt controller registers.
     */
    STORAGE (INTCON);		// Interrupt Control
    STORAGE (INTSTAT);          // Interrupt Status
    STORAGE (IFS0);           // IFS(0..2) - Interrupt Flag Status
    STORAGE (IFS1);
    STORAGE (IFS2);
    STORAGE (IEC0);           // IEC(0..2) - Interrupt Enable Control
    STORAGE (IEC1);
    STORAGE (IEC2);
    STORAGE (IPC0);           // IPC(0..11) - Interrupt Priority Control
    STORAGE (IPC1);
    STORAGE (IPC2);
    STORAGE (IPC3);
    STORAGE (IPC4);
    STORAGE (IPC5);
    STORAGE (IPC6);
    STORAGE (IPC7);
    STORAGE (IPC8);
    STORAGE (IPC9);
    STORAGE (IPC10);
    STORAGE (IPC11);
    STORAGE (IPC12);

    default:
        fprintf (stderr, "--- Read %08x: peripheral register not supported\n",
            address);
        exit (1);
    }
    return *bufp;
}

void io_write32 (unsigned address, unsigned *bufp, unsigned data, const char **namep)
{
    switch (address) {
    /*
     * Bus matrix control registers.
     */
    STORAGE (BMXCON);           // Bus Matrix Control
    STORAGE (BMXDKPBA);         // Data RAM kernel program base address
    STORAGE (BMXDUDBA);         // Data RAM user data base address
    STORAGE (BMXDUPBA);         // Data RAM user program base address
    STORAGE (BMXPUPBA);         // Program Flash user program base address
    READONLY(BMXDRMSZ);         // Data RAM memory size
    READONLY(BMXPFMSZ);         // Program Flash memory size
    READONLY(BMXBOOTSZ);        // Boot Flash size

    /*
     * Interrupt controller registers.
     */
    WRITEOP (INTCON); return;   // Interrupt Control
    READONLY(INTSTAT);          // Interrupt Status
    WRITEOP (IPTMR);  return;   // Temporal Proximity Timer
    WRITEOP (IFS0); goto irq; // IFS(0..2) - Interrupt Flag Status
    WRITEOP (IFS1); goto irq;
    WRITEOP (IFS2); goto irq;
    WRITEOP (IEC0); goto irq; // IEC(0..2) - Interrupt Enable Control
    WRITEOP (IEC1); goto irq;
    WRITEOP (IEC2); goto irq;
    WRITEOP (IPC0); goto irq; // IPC(0..11) - Interrupt Priority Control
    WRITEOP (IPC1); goto irq;
    WRITEOP (IPC2); goto irq;
    WRITEOP (IPC3); goto irq;
    WRITEOP (IPC4); goto irq;
    WRITEOP (IPC5); goto irq;
    WRITEOP (IPC6); goto irq;
    WRITEOP (IPC7); goto irq;
    WRITEOP (IPC8); goto irq;
    WRITEOP (IPC9); goto irq;
    WRITEOP (IPC10); goto irq;
    WRITEOP (IPC11); goto irq;
    WRITEOP (IPC12);
irq:    update_irq_flag();
        return;

    default:
        fprintf (stderr, "--- Write %08x to %08x: peripheral register not supported\n",
            data, address);
        exit (1);
readonly:
        fprintf (stderr, "--- Write %08x to %s: readonly register\n",
            data, *namep);
        *namep = 0;
        return;
    }
    *bufp = data;
}

void io_reset()
{
    /*
     * Bus matrix control registers.
     */
    VALUE (BMXCON)    = 0x001f0041;     // Bus Matrix Control
    VALUE (BMXDKPBA)  = 0;              // Data RAM kernel program base address
    VALUE (BMXDUDBA)  = 0;              // Data RAM user data base address
    VALUE (BMXDUPBA)  = 0;              // Data RAM user program base address
    VALUE (BMXPUPBA)  = 0;              // Program Flash user program base address
    VALUE (BMXDRMSZ)  = 128 * 1024;     // Data RAM memory size
    VALUE (BMXPFMSZ)  = 512 * 1024;     // Program Flash memory size
    VALUE (BMXBOOTSZ) = 12 * 1024;      // Boot Flash size
}

void io_init (void *datap, void *data2p)
{
    iomem = datap;
    iomem2 = data2p;
    io_reset();
}
