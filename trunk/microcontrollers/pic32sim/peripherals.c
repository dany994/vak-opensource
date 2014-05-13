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

#define STORAGE(name) case name: *namep = #name; 
#define READONLY(name) case name: *namep = #name; goto readonly
#define WRITEOP(name) case name: *namep = #name; goto op_##name;\
                      case name+4: *namep = #name"CLR"; goto op_##name;\
                      case name+8: *namep = #name"SET"; goto op_##name;\
                      case name+12: *namep = #name"INV"; op_##name: \
                      *bufp = write_op (*bufp, data, address)
#define WRITEOPX(name,label) \
		      case name: *namep = #name; goto op_##label;\
                      case name+4: *namep = #name"CLR"; goto op_##label;\
                      case name+8: *namep = #name"SET"; goto op_##label;\
                      case name+12: *namep = #name"INV"; goto op_##label

#define VALUE(name) (name & 0x80000 ? iomem2 : iomem) [(name & 0xffff) >> 2]

static unsigned *iomem;         // image of I/O area
static unsigned *iomem2;        // image of second I/O area

static unsigned syskey_unlock;	// syskey state

void update_irq_flag()
{
    //TODO
#if 0
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

    mips_update_irq_flag();
#endif
}

static void soft_reset()
{
    //TODO
#if 0
    mips_reset();
    cpu->pc = pic32->start_address;

    /* reset all devices */
    reset_all();
    sdcard_reset();
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
    STORAGE (BMXCON); break;    // Bus Mmatrix Control
    STORAGE (BMXDKPBA); break;  // Data RAM kernel program base address
    STORAGE (BMXDUDBA); break;  // Data RAM user data base address
    STORAGE (BMXDUPBA); break;  // Data RAM user program base address
    STORAGE (BMXPUPBA); break;  // Program Flash user program base address
    STORAGE (BMXDRMSZ); break;  // Data RAM memory size
    STORAGE (BMXPFMSZ); break;  // Program Flash memory size
    STORAGE (BMXBOOTSZ); break; // Boot Flash size

    /*
     * Interrupt controller registers.
     */
    STORAGE (INTCON); break;	// Interrupt Control
    STORAGE (INTSTAT); break;   // Interrupt Status
    STORAGE (IFS0); break;	// IFS(0..2) - Interrupt Flag Status
    STORAGE (IFS1); break;
    STORAGE (IFS2); break;
    STORAGE (IEC0); break;	// IEC(0..2) - Interrupt Enable Control
    STORAGE (IEC1); break;
    STORAGE (IEC2); break;
    STORAGE (IPC0); break;	// IPC(0..11) - Interrupt Priority Control
    STORAGE (IPC1); break;
    STORAGE (IPC2); break;
    STORAGE (IPC3); break;
    STORAGE (IPC4); break;
    STORAGE (IPC5); break;
    STORAGE (IPC6); break;
    STORAGE (IPC7); break;
    STORAGE (IPC8); break;
    STORAGE (IPC9); break;
    STORAGE (IPC10); break;
    STORAGE (IPC11); break;
    STORAGE (IPC12); break;

    /*
     * Prefetch controller.
     */
    STORAGE (CHECON); break;	// Prefetch Control

    /*
     * System controller.
     */
    STORAGE (OSCCON); break;	// Oscillator Control
    STORAGE (OSCTUN); break;	// Oscillator Tuning
    STORAGE (DDPCON); break;	// Debug Data Port Control
    STORAGE (DEVID); break;	// Device Identifier
    STORAGE (SYSKEY); break;	// System Key
    STORAGE (RCON); break;	// Reset Control
    STORAGE (RSWRST); break;	// Software Reset 

    /*
     * Analog to digital converter.
     */
    STORAGE (AD1CON1); break;	// Control register 1 
    STORAGE (AD1CON2); break;	// Control register 2
    STORAGE (AD1CON3); break;	// Control register 3 
    STORAGE (AD1CHS); break;    // Channel select 
    STORAGE (AD1CSSL); break;   // Input scan selection 
    STORAGE (AD1PCFG); break;   // Port configuration 
    STORAGE (ADC1BUF0); break;  // Result words
    STORAGE (ADC1BUF1); break; 
    STORAGE (ADC1BUF2); break;  
    STORAGE (ADC1BUF3); break; 
    STORAGE (ADC1BUF4); break;  
    STORAGE (ADC1BUF5); break; 
    STORAGE (ADC1BUF6); break;  
    STORAGE (ADC1BUF7); break; 
    STORAGE (ADC1BUF8); break;  
    STORAGE (ADC1BUF9); break; 
    STORAGE (ADC1BUFA); break;  
    STORAGE (ADC1BUFB); break; 
    STORAGE (ADC1BUFC); break;  
    STORAGE (ADC1BUFD); break; 
    STORAGE (ADC1BUFE); break;  
    STORAGE (ADC1BUFF); break; 

    /*
     * General purpose IO signals.
     */
    STORAGE (TRISA); break;     // Port A: mask of inputs 
    STORAGE (PORTA); break;     // Port A: read inputs
    STORAGE (LATA); break;      // Port A: read outputs 
    STORAGE (ODCA); break;      // Port A: open drain configuration 
    STORAGE (TRISB); break;     // Port B: mask of inputs 
    STORAGE (PORTB); break;     // Port B: read inputs
    STORAGE (LATB); break;      // Port B: read outputs 
    STORAGE (ODCB); break;      // Port B: open drain configuration 
    STORAGE (TRISC); break;     // Port C: mask of inputs 
    STORAGE (PORTC); break;     // Port C: read inputs
    STORAGE (LATC); break;      // Port C: read outputs 
    STORAGE (ODCC); break;      // Port C: open drain configuration 
    STORAGE (TRISD); break;     // Port D: mask of inputs 
    STORAGE (PORTD);		// Port D: read inputs
#ifdef MAXIMITE
#if 0
	/* Poll PS2 keyboard */
	if (keyboard_clock())
	    d->port_d &= ~MASKD_PS2C;
	else
	    d->port_d |= MASKD_PS2C;
	if (keyboard_data())
	    d->port_d &= ~MASKD_PS2D;
	else
	    d->port_d |= MASKD_PS2D;
#endif
#endif
	break;
    STORAGE (LATD); break;      // Port D: read outputs 
    STORAGE (ODCD); break;      // Port D: open drain configuration 
    STORAGE (TRISE); break;     // Port E: mask of inputs 
    STORAGE (PORTE);		// Port E: read inputs
#ifdef UBW32
	/* Swap disk: DATA */
	d->port_e &= ~MASKE_DATA;
	d->port_e |= swap_io (0, 0xff);
#endif
	break;
    STORAGE (LATE); break;      // Port E: read outputs 
    STORAGE (ODCE); break;      // Port E: open drain configuration 
    STORAGE (TRISF); break;     // Port F: mask of inputs 
    STORAGE (PORTF); break;     // Port F: read inputs
    STORAGE (LATF); break;      // Port F: read outputs 
    STORAGE (ODCF); break;      // Port F: open drain configuration 
    STORAGE (TRISG); break;     // Port G: mask of inputs 
    STORAGE (PORTG); break;     // Port G: read inputs
    STORAGE (LATG); break;      // Port G: read outputs 
    STORAGE (ODCG); break;      // Port G: open drain configuration 
    STORAGE (CNCON); break;     // Interrupt-on-change control 
    STORAGE (CNEN); break;      // Input change interrupt enable 
    STORAGE (CNPUE); break;     // Input pin pull-up enable 

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
    WRITEOP (BMXCON); return;   // Bus Matrix Control
    STORAGE (BMXDKPBA); break;  // Data RAM kernel program base address
    STORAGE (BMXDUDBA); break;  // Data RAM user data base address
    STORAGE (BMXDUPBA); break;  // Data RAM user program base address
    STORAGE (BMXPUPBA); break;  // Program Flash user program base address
    READONLY(BMXDRMSZ);         // Data RAM memory size
    READONLY(BMXPFMSZ);         // Program Flash memory size
    READONLY(BMXBOOTSZ);        // Boot Flash size

    /*
     * Interrupt controller registers.
     */
    WRITEOP (INTCON); return;   // Interrupt Control
    READONLY(INTSTAT);          // Interrupt Status
    WRITEOP (IPTMR);  return;   // Temporal Proximity Timer
    WRITEOP (IFS0); goto irq;	// IFS(0..2) - Interrupt Flag Status
    WRITEOP (IFS1); goto irq;
    WRITEOP (IFS2); goto irq;
    WRITEOP (IEC0); goto irq;	// IEC(0..2) - Interrupt Enable Control
    WRITEOP (IEC1); goto irq;
    WRITEOP (IEC2); goto irq;
    WRITEOP (IPC0); goto irq;	// IPC(0..11) - Interrupt Priority Control
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

    /*
     * Prefetch controller.
     */
    WRITEOP (CHECON); return;   // Prefetch Control

    /*
     * System controller.
     */
    STORAGE (OSCCON); break;	// Oscillator Control
    STORAGE (OSCTUN); break;	// Oscillator Tuning
    STORAGE (DDPCON); break;	// Debug Data Port Control
    READONLY(DEVID);		// Device Identifier
    STORAGE (SYSKEY);		// System Key
	/* Unlock state machine. */
	if (syskey_unlock == 0 && VALUE(SYSKEY) == 0xaa996655)
	    syskey_unlock = 1;
	if (syskey_unlock == 1 && VALUE(SYSKEY) == 0x556699aa)
	    syskey_unlock = 2;
	else
	    syskey_unlock = 0;
	break;
    STORAGE (RCON); break;	// Reset Control
    STORAGE (RSWRST);		// Software Reset 
	if (syskey_unlock == 2 && (VALUE(RSWRST) & 1))
	    soft_reset();
	break;
	
    /*
     * Analog to digital converter.
     */
    WRITEOP (AD1CON1); return;	// Control register 1 
    WRITEOP (AD1CON2); return;	// Control register 2
    WRITEOP (AD1CON3); return;	// Control register 3 
    WRITEOP (AD1CHS); return;   // Channel select 
    WRITEOP (AD1CSSL); return;  // Input scan selection 
    WRITEOP (AD1PCFG); return;  // Port configuration 
    READONLY(ADC1BUF0);         // Result words
    READONLY(ADC1BUF1); 
    READONLY(ADC1BUF2); 
    READONLY(ADC1BUF3); 
    READONLY(ADC1BUF4); 
    READONLY(ADC1BUF5); 
    READONLY(ADC1BUF6); 
    READONLY(ADC1BUF7); 
    READONLY(ADC1BUF8); 
    READONLY(ADC1BUF9); 
    READONLY(ADC1BUFA); 
    READONLY(ADC1BUFB); 
    READONLY(ADC1BUFC); 
    READONLY(ADC1BUFD); 
    READONLY(ADC1BUFE); 
    READONLY(ADC1BUFF); 

    /*
     * General purpose IO signals.
     */
    WRITEOP (TRISA); return;	    // Port A: mask of inputs 
    WRITEOPX(PORTA, LATA);          // Port A: write outputs 
    WRITEOP (LATA);		    // Port A: write outputs 
#ifdef UBW32
	/* Control SD card 0 */
	if (VALUE(LATA) & MASKA_CS0)
	    sdcard_select (0, 0);
	else
	    sdcard_select (0, 1);

	/* Control SD card 1 */
	if (VALUE(LATA) & MASKA_CS1)
	    sdcard_select (1, 0);
	else
	    sdcard_select (1, 1);
#endif
	return;
    WRITEOP (ODCA); return;	    // Port A: open drain configuration 
    WRITEOP (TRISB); return;	    // Port B: mask of inputs 
    WRITEOPX(PORTB, LATB);          // Port B: write outputs 
    WRITEOP (LATB);		    // Port B: write outputs 
#ifdef EXPLORER16
	/* Control SD card 0 */
	if (d->lat_b & MASKB_CS0)
	    sdcard_select (0, 0);
	else
	    sdcard_select (0, 1);

	/* Control SD card 1 */
	if (d->lat_b & MASKB_CS1)
	    sdcard_select (1, 0);
	else
	    sdcard_select (1, 1);
#endif
	return;
    WRITEOP (ODCB); return;	    // Port B: open drain configuration 
    WRITEOP (TRISC); return;	    // Port C: mask of inputs 
    WRITEOPX(PORTC, LATC);          // Port C: write outputs 
    WRITEOP (LATC);		    // Port C: write outputs 
#ifdef UBW32
	if (d->lat_c & MASKC_LDADDR)  /* Swap disk: LDADDR */
	    swap_ldaddr (0);
	else
	    swap_ldaddr (1);
#endif
	return;
    WRITEOP (ODCC); return;	    // Port C: open drain configuration 
    WRITEOP (TRISD); return;	    // Port D: mask of inputs 
    WRITEOPX(PORTD, LATD);          // Port D: write outputs 
    WRITEOP (LATD);		    // Port D: write outputs 
#ifdef MAX32
	/* Control SD card 0 */
	if (d->lat_d & MASKD_CS0)
	    sdcard_select (0, 0);
	else
	    sdcard_select (0, 1);

	/* Control SD card 1 */
	if (d->lat_d & MASKD_CS1)
	    sdcard_select (1, 0);
	else
	    sdcard_select (1, 1);
#endif
	return;
    WRITEOP (ODCD); return;	    // Port D: open drain configuration 
    WRITEOP (TRISE); return;	    // Port E: mask of inputs 
    WRITEOPX(PORTE, LATE);          // Port E: write outputs 
    WRITEOP (LATE);		    // Port E: write outputs 
#ifdef MAXIMITE
	/* Control SD card 0 */
	if (d->lat_e & MASKE_CS0)
	    sdcard_select (0, 0);
	else
	    sdcard_select (0, 1);
#if 0
	/* Control SD card 1 */
	if (d->lat_e & MASKE_CS1)
	    sdcard_select (1, 0);
	else
	    sdcard_select (1, 1);
#endif
#endif
#ifdef UBW32
	if (d->lat_e & MASKE_RD)        /* Swap disk: RD */
	    swap_rd (0);
	else
	    swap_rd (1);

	if (d->lat_e & MASKE_WR)        /* Swap disk: WR */
	    swap_wr (0);
	else
	    swap_wr (1);

	/* Swap disk: DATA */
	swap_io (d->lat_e >> SHIFTE_DATA,
	    d->tris_e >> SHIFTE_DATA);
#endif
	return;
    WRITEOP (ODCE); return;	    // Port E: open drain configuration 
    WRITEOP (TRISF); return;	    // Port F: mask of inputs 
    WRITEOPX(PORTF, LATF);          // Port F: write outputs 
    WRITEOP (LATF);		    // Port F: write outputs 
	return;
    WRITEOP (ODCF); return;	    // Port F: open drain configuration 
    WRITEOP (TRISG); return;	    // Port G: mask of inputs 
    WRITEOPX(PORTG, LATG);          // Port G: write outputs 
    WRITEOP (LATG);		    // Port G: write outputs 
	return;
    WRITEOP (ODCG); return;	    // Port G: open drain configuration 
    WRITEOP (CNCON); return;	    // Interrupt-on-change control 
    WRITEOP (CNEN); return;	    // Input change interrupt enable 
    WRITEOP (CNPUE); return;	    // Input pin pull-up enable 

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

    /*
     * Prefetch controller.
     */
    VALUE (CHECON) = 0x00000007;

    /*
     * System controller.
     */
    VALUE (OSCCON) = 0x01453320;	// from ubw32 board 
    VALUE (OSCTUN) = 0;
    VALUE (DDPCON) = 0;
    VALUE (DEVID)  = 0x04307053;	// 795F512L 
    VALUE (SYSKEY) = 0;
    VALUE (RCON)   = 0;
    VALUE (RSWRST) = 0;
    syskey_unlock  = 0;

    /*
     * Analog to digital converter.
     */
    VALUE (AD1CON1) = 0;		// Control register 1 
    VALUE (AD1CON2) = 0;		// Control register 2
    VALUE (AD1CON3) = 0;		// Control register 3 
    VALUE (AD1CHS)  = 0;		// Channel select 
    VALUE (AD1CSSL) = 0;		// Input scan selection 
    VALUE (AD1PCFG) = 0;		// Port configuration 

    /*
     * General purpose IO signals.
     * All pins are inputs, high, open drains and pullups disabled.
     * No interrupts on change.
     */
    VALUE (TRISA) = 0xFFFF;		// Port A: mask of inputs 
    VALUE (PORTA) = 0xFFFF;		// Port A: read inputs, write outputs 
    VALUE (LATA)  = 0xFFFF;		// Port A: read/write outputs 
    VALUE (ODCA)  = 0;			// Port A: open drain configuration 
    VALUE (TRISB) = 0xFFFF;		// Port B: mask of inputs 
    VALUE (PORTB) = 0xFFFF;		// Port B: read inputs, write outputs 
    VALUE (LATB)  = 0xFFFF;		// Port B: read/write outputs 
    VALUE (ODCB)  = 0;			// Port B: open drain configuration 
    VALUE (TRISC) = 0xFFFF;		// Port C: mask of inputs 
    VALUE (PORTC) = 0xFFFF;		// Port C: read inputs, write outputs 
    VALUE (LATC)  = 0xFFFF;		// Port C: read/write outputs 
    VALUE (ODCC)  = 0;			// Port C: open drain configuration 
    VALUE (TRISD) = 0xFFFF;		// Port D: mask of inputs 
    VALUE (PORTD) = 0xFFFF;		// Port D: read inputs, write outputs 
    VALUE (LATD)  = 0xFFFF;		// Port D: read/write outputs 
    VALUE (ODCD)  = 0;			// Port D: open drain configuration 
    VALUE (TRISE) = 0xFFFF;		// Port E: mask of inputs 
    VALUE (PORTE) = 0xFFFF;		// Port D: read inputs, write outputs 
    VALUE (LATE)  = 0xFFFF;		// Port E: read/write outputs 
    VALUE (ODCE)  = 0;			// Port E: open drain configuration 
    VALUE (TRISF) = 0xFFFF;		// Port F: mask of inputs 
    VALUE (PORTF) = 0xFFFF;		// Port F: read inputs, write outputs 
    VALUE (LATF)  = 0xFFFF;		// Port F: read/write outputs 
    VALUE (ODCF)  = 0;			// Port F: open drain configuration 
    VALUE (TRISG) = 0xFFFF;		// Port G: mask of inputs 
    VALUE (PORTG) = 0xFFFF;		// Port G: read inputs, write outputs 
    VALUE (LATG)  = 0xFFFF;		// Port G: read/write outputs 
    VALUE (ODCG)  = 0;			// Port G: open drain configuration 
    VALUE (CNCON) = 0;			// Interrupt-on-change control 
    VALUE (CNEN)  = 0;			// Input change interrupt enable 
    VALUE (CNPUE) = 0;			// Input pin pull-up enable 
}

void io_init (void *datap, void *data2p)
{
    iomem = datap;
    iomem2 = data2p;
    io_reset();
}
