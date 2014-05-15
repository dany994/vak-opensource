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
#include <stdint.h>
#include <unistd.h>
#include "globals.h"

#define PIC32MX7
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
#define WRITEOPR(name,romask) \
                      case name: *namep = #name; goto op_##name;\
                      case name+4: *namep = #name"CLR"; goto op_##name;\
                      case name+8: *namep = #name"SET"; goto op_##name;\
                      case name+12: *namep = #name"INV"; op_##name: \
                      *bufp &= romask; \
                      *bufp |= write_op (*bufp, data, address) & ~(romask)

#define VALUE(name) (name & 0x80000 ? iomem2 : iomem) [(name & 0xffff) >> 2]

static uint32_t *iomem;         // image of I/O area
static uint32_t *iomem2;        // image of second I/O area
static uint32_t *bootmem;       // image of boot memory

static unsigned syskey_unlock;	// syskey state

static unsigned uart_irq[6] = { // UART interrupt numbers
    PIC32_IRQ_U1E,
    PIC32_IRQ_U2E,
    PIC32_IRQ_U3E,
    PIC32_IRQ_U4E,
    PIC32_IRQ_U5E,
    PIC32_IRQ_U6E,
};

static int uart_oactive[6];     // UART output active

int vtty_get_char (int devnum)
{
    //TODO
    return 0;
}

int vtty_is_char_avail (int devnum)
{
    //TODO
    return 0;
}

void vtty_put_char (int devnum, char ch)
{
    if (ch == '\r')             // Skip ^M
        return;

    // TODO
    if (write (1, &ch, 1) < 0)
        /*ignore*/;
}

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

void set_irq (int irq)
{
#if 0
    /* Set interrupt flag status */
    ifs [irq >> 5] |= 1 << (irq & 31);

    update_irq_flag();
#endif
}

void clear_irq (int irq)
{
#if 0
    /* Clear interrupt flag status */
    ifs [irq >> 5] &= ~(1 << (irq & 31));

    update_irq_flag();
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
    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
     * Prefetch controller.
     */
    STORAGE (CHECON); break;	// Prefetch Control

    /*-------------------------------------------------------------------------
     * System controller.
     */
    STORAGE (OSCCON); break;	// Oscillator Control
    STORAGE (OSCTUN); break;	// Oscillator Tuning
    STORAGE (DDPCON); break;	// Debug Data Port Control
    STORAGE (DEVID); break;	// Device Identifier
    STORAGE (SYSKEY); break;	// System Key
    STORAGE (RCON); break;	// Reset Control
    STORAGE (RSWRST); break;	// Software Reset

    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
     * UART 1.
     */
    STORAGE (U1RXREG);          // Receive data
        *bufp = vtty_get_char (0);              // read a byte from input queue
        VALUE(U1STA) &= ~PIC32_USTA_URXDA;
        if (vtty_is_char_avail (0)) {           // one more byte available
            VALUE(U1STA) |= PIC32_USTA_URXDA;
        } else {
            clear_irq (uart_irq[0] + 1);        // deactivate RX interrupt
        }
        break;
    STORAGE (U1BRG); break;     // Baud rate
    STORAGE (U1MODE); break;    // Mode
    STORAGE (U1STA);            // Status and control
        VALUE(U1STA) |= PIC32_USTA_RIDLE |      // Receiver is idle
                        PIC32_USTA_TRMT;        // Transmit shift register is empty
        if (vtty_is_char_avail (0))
            VALUE(U1STA) |= PIC32_USTA_URXDA;   // Receive data available
#if 0
        printf ("<%x>", VALUE(U1STA));
        fflush (stdout);
#endif
        break;
    STORAGE (U1TXREG);   *bufp = 0; break;      // Transmit
    STORAGE (U1MODECLR); *bufp = 0; break;
    STORAGE (U1MODESET); *bufp = 0; break;
    STORAGE (U1MODEINV); *bufp = 0; break;
    STORAGE (U1STACLR);  *bufp = 0; break;
    STORAGE (U1STASET);  *bufp = 0; break;
    STORAGE (U1STAINV);  *bufp = 0; break;
    STORAGE (U1BRGCLR);  *bufp = 0; break;
    STORAGE (U1BRGSET);  *bufp = 0; break;
    STORAGE (U1BRGINV);  *bufp = 0; break;

    /*-------------------------------------------------------------------------
     * UART 2.
     */
    STORAGE (U2RXREG);          // Receive data
        *bufp = vtty_get_char (1);              // read a byte from input queue
        VALUE(U2STA) &= ~PIC32_USTA_URXDA;
        if (vtty_is_char_avail (1)) {           // one more byte available
            VALUE(U2STA) |= PIC32_USTA_URXDA;
        } else {
            clear_irq (uart_irq[1] + 1);        // deactivate RX interrupt
        }
        break;
    STORAGE (U2BRG); break;     // Baud rate
    STORAGE (U2MODE); break;    // Mode
    STORAGE (U2STA);            // Status and control
        VALUE(U2STA) |= PIC32_USTA_RIDLE |      // Receiver is idle
                        PIC32_USTA_TRMT;        // Transmit shift register is empty
        if (vtty_is_char_avail (1))
            VALUE(U2STA) |= PIC32_USTA_URXDA;   // Receive data available
#if 0
        printf ("<%x>", VALUE(U2STA));
        fflush (stdout);
#endif
        break;
    STORAGE (U2TXREG);   *bufp = 0; break;      // Transmit
    STORAGE (U2MODECLR); *bufp = 0; break;
    STORAGE (U2MODESET); *bufp = 0; break;
    STORAGE (U2MODEINV); *bufp = 0; break;
    STORAGE (U2STACLR);  *bufp = 0; break;
    STORAGE (U2STASET);  *bufp = 0; break;
    STORAGE (U2STAINV);  *bufp = 0; break;
    STORAGE (U2BRGCLR);  *bufp = 0; break;
    STORAGE (U2BRGSET);  *bufp = 0; break;
    STORAGE (U2BRGINV);  *bufp = 0; break;

    /*-------------------------------------------------------------------------
     * UART 3.
     */
    STORAGE (U3RXREG);          // Receive data
        *bufp = vtty_get_char (2);              // read a byte from input queue
        VALUE(U3STA) &= ~PIC32_USTA_URXDA;
        if (vtty_is_char_avail (2)) {           // one more byte available
            VALUE(U3STA) |= PIC32_USTA_URXDA;
        } else {
            clear_irq (uart_irq[2] + 1);        // deactivate RX interrupt
        }
        break;
    STORAGE (U3BRG); break;     // Baud rate
    STORAGE (U3MODE); break;    // Mode
    STORAGE (U3STA);            // Status and control
        VALUE(U3STA) |= PIC32_USTA_RIDLE |      // Receiver is idle
                        PIC32_USTA_TRMT;        // Transmit shift register is empty
        if (vtty_is_char_avail (2))
            VALUE(U3STA) |= PIC32_USTA_URXDA;   // Receive data available
#if 0
        printf ("<%x>", VALUE(U3STA));
        fflush (stdout);
#endif
        break;
    STORAGE (U3TXREG);   *bufp = 0; break;      // Transmit
    STORAGE (U3MODECLR); *bufp = 0; break;
    STORAGE (U3MODESET); *bufp = 0; break;
    STORAGE (U3MODEINV); *bufp = 0; break;
    STORAGE (U3STACLR);  *bufp = 0; break;
    STORAGE (U3STASET);  *bufp = 0; break;
    STORAGE (U3STAINV);  *bufp = 0; break;
    STORAGE (U3BRGCLR);  *bufp = 0; break;
    STORAGE (U3BRGSET);  *bufp = 0; break;
    STORAGE (U3BRGINV);  *bufp = 0; break;

    /*-------------------------------------------------------------------------
     * UART 4.
     */
    STORAGE (U4RXREG);          // Receive data
        *bufp = vtty_get_char (3);              // read a byte from input queue
        VALUE(U4STA) &= ~PIC32_USTA_URXDA;
        if (vtty_is_char_avail (3)) {           // one more byte available
            VALUE(U4STA) |= PIC32_USTA_URXDA;
        } else {
            clear_irq (uart_irq[3] + 1);        // deactivate RX interrupt
        }
        break;
    STORAGE (U4BRG); break;     // Baud rate
    STORAGE (U4MODE); break;    // Mode
    STORAGE (U4STA);            // Status and control
        VALUE(U4STA) |= PIC32_USTA_RIDLE |      // Receiver is idle
                        PIC32_USTA_TRMT;        // Transmit shift register is empty
        if (vtty_is_char_avail (3))
            VALUE(U4STA) |= PIC32_USTA_URXDA;   // Receive data available
#if 0
        printf ("<%x>", VALUE(U4STA));
        fflush (stdout);
#endif
        break;
    STORAGE (U4TXREG);   *bufp = 0; break;      // Transmit
    STORAGE (U4MODECLR); *bufp = 0; break;
    STORAGE (U4MODESET); *bufp = 0; break;
    STORAGE (U4MODEINV); *bufp = 0; break;
    STORAGE (U4STACLR);  *bufp = 0; break;
    STORAGE (U4STASET);  *bufp = 0; break;
    STORAGE (U4STAINV);  *bufp = 0; break;
    STORAGE (U4BRGCLR);  *bufp = 0; break;
    STORAGE (U4BRGSET);  *bufp = 0; break;
    STORAGE (U4BRGINV);  *bufp = 0; break;

    /*-------------------------------------------------------------------------
     * UART 5.
     */
    STORAGE (U5RXREG);          // Receive data
        *bufp = vtty_get_char (4);              // read a byte from input queue
        VALUE(U5STA) &= ~PIC32_USTA_URXDA;
        if (vtty_is_char_avail (4)) {           // one more byte available
            VALUE(U5STA) |= PIC32_USTA_URXDA;
        } else {
            clear_irq (uart_irq[4] + 1);        // deactivate RX interrupt
        }
        break;
    STORAGE (U5BRG); break;     // Baud rate
    STORAGE (U5MODE); break;    // Mode
    STORAGE (U5STA);            // Status and control
        VALUE(U5STA) |= PIC32_USTA_RIDLE |      // Receiver is idle
                        PIC32_USTA_TRMT;        // Transmit shift register is empty
        if (vtty_is_char_avail (4))
            VALUE(U5STA) |= PIC32_USTA_URXDA;   // Receive data available
#if 0
        printf ("<%x>", VALUE(U5STA));
        fflush (stdout);
#endif
        break;
    STORAGE (U5TXREG);   *bufp = 0; break;      // Transmit
    STORAGE (U5MODECLR); *bufp = 0; break;
    STORAGE (U5MODESET); *bufp = 0; break;
    STORAGE (U5MODEINV); *bufp = 0; break;
    STORAGE (U5STACLR);  *bufp = 0; break;
    STORAGE (U5STASET);  *bufp = 0; break;
    STORAGE (U5STAINV);  *bufp = 0; break;
    STORAGE (U5BRGCLR);  *bufp = 0; break;
    STORAGE (U5BRGSET);  *bufp = 0; break;
    STORAGE (U5BRGINV);  *bufp = 0; break;

    /*-------------------------------------------------------------------------
     * UART 6.
     */
    STORAGE (U6RXREG);          // Receive data
        *bufp = vtty_get_char (5);              // read a byte from input queue
        VALUE(U6STA) &= ~PIC32_USTA_URXDA;
        if (vtty_is_char_avail (5)) {           // one more byte available
            VALUE(U6STA) |= PIC32_USTA_URXDA;
        } else {
            clear_irq (uart_irq[5] + 1);        // deactivate RX interrupt
        }
        break;
    STORAGE (U6BRG); break;     // Baud rate
    STORAGE (U6MODE); break;    // Mode
    STORAGE (U6STA);            // Status and control
        VALUE(U6STA) |= PIC32_USTA_RIDLE |      // Receiver is idle
                        PIC32_USTA_TRMT;        // Transmit shift register is empty
        if (vtty_is_char_avail (5))
            VALUE(U6STA) |= PIC32_USTA_URXDA;   // Receive data available
#if 0
        printf ("<%x>", VALUE(U6STA));
        fflush (stdout);
#endif
        break;
    STORAGE (U6TXREG);   *bufp = 0; break;      // Transmit
    STORAGE (U6MODECLR); *bufp = 0; break;
    STORAGE (U6MODESET); *bufp = 0; break;
    STORAGE (U6MODEINV); *bufp = 0; break;
    STORAGE (U6STACLR);  *bufp = 0; break;
    STORAGE (U6STASET);  *bufp = 0; break;
    STORAGE (U6STAINV);  *bufp = 0; break;
    STORAGE (U6BRGCLR);  *bufp = 0; break;
    STORAGE (U6BRGSET);  *bufp = 0; break;
    STORAGE (U6BRGINV);  *bufp = 0; break;

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
    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
     * Prefetch controller.
     */
    WRITEOP (CHECON); return;   // Prefetch Control

    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
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

    /*-------------------------------------------------------------------------
     * UART 1.
     */
    STORAGE (U1TXREG);              // Transmit
        vtty_put_char (0, data);
        if ((VALUE (U1MODE) & PIC32_UMODE_ON) &&
            (VALUE (U1STA) & PIC32_USTA_UTXEN) && ! uart_oactive[0]) {
            uart_oactive[0] = 1;
            set_irq (uart_irq[0] + 2);          // activate transmit interrupt
        }
        break;
    WRITEOP (U1MODE);               // Mode
        if (! (VALUE (U1MODE) & PIC32_UMODE_ON)) {
            clear_irq (uart_irq[0] + 1);        // deactivate receive interrupt
            clear_irq (uart_irq[0] + 2);        // deactivate transmit interrupt
            VALUE (U1STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                               PIC32_USTA_PERR | PIC32_USTA_UTXBF);
            VALUE (U1STA) |= PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
        }
        return;
    WRITEOPR (U1STA,                // Status and control
        PIC32_USTA_URXDA | PIC32_USTA_FERR | PIC32_USTA_PERR |
        PIC32_USTA_RIDLE | PIC32_USTA_TRMT | PIC32_USTA_UTXBF);
        if (! (VALUE (U1STA) & PIC32_USTA_URXEN)) {
            clear_irq (uart_irq[0] + 1);        // deactivate receive interrupt
            VALUE (U1STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                PIC32_USTA_PERR);
        }
        if (! (VALUE (U1STA) & PIC32_USTA_UTXEN)) {
            clear_irq (uart_irq[0] + 2);        // deactivate transmit interrupt
            VALUE (U1STA) &= ~PIC32_USTA_UTXBF;
            VALUE (U1STA) |= PIC32_USTA_TRMT;
        }
        return;
    WRITEOP (U1BRG); return;        // Baud rate
    READONLY (U1RXREG);             // Receive

    /*-------------------------------------------------------------------------
     * UART 2.
     */
    STORAGE (U2TXREG);              // Transmit
        vtty_put_char (1, data);
        if ((VALUE (U2MODE) & PIC32_UMODE_ON) &&
            (VALUE (U2STA) & PIC32_USTA_UTXEN) && ! uart_oactive[1]) {
            uart_oactive[1] = 1;
            set_irq (uart_irq[1] + 2);          // activate transmit interrupt
        }
        break;
    WRITEOP (U2MODE);               // Mode
        if (! (VALUE (U2MODE) & PIC32_UMODE_ON)) {
            clear_irq (uart_irq[1] + 1);        // deactivate receive interrupt
            clear_irq (uart_irq[1] + 2);        // deactivate transmit interrupt
            VALUE (U2STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                               PIC32_USTA_PERR | PIC32_USTA_UTXBF);
            VALUE (U2STA) |= PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
        }
        return;
    WRITEOPR (U2STA,                // Status and control
        PIC32_USTA_URXDA | PIC32_USTA_FERR | PIC32_USTA_PERR |
        PIC32_USTA_RIDLE | PIC32_USTA_TRMT | PIC32_USTA_UTXBF);
        if (! (VALUE (U2STA) & PIC32_USTA_URXEN)) {
            clear_irq (uart_irq[1] + 1);        // deactivate receive interrupt
            VALUE (U2STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                PIC32_USTA_PERR);
        }
        if (! (VALUE (U2STA) & PIC32_USTA_UTXEN)) {
            clear_irq (uart_irq[1] + 2);        // deactivate transmit interrupt
            VALUE (U2STA) &= ~PIC32_USTA_UTXBF;
            VALUE (U2STA) |= PIC32_USTA_TRMT;
        }
        return;
    WRITEOP (U2BRG); return;        // Baud rate
    READONLY (U2RXREG);             // Receive

    /*-------------------------------------------------------------------------
     * UART 3.
     */
    STORAGE (U3TXREG);              // Transmit
        vtty_put_char (2, data);
        if ((VALUE (U3MODE) & PIC32_UMODE_ON) &&
            (VALUE (U3STA) & PIC32_USTA_UTXEN) && ! uart_oactive[2]) {
            uart_oactive[2] = 1;
            set_irq (uart_irq[2] + 2);          // activate transmit interrupt
        }
        break;
    WRITEOP (U3MODE);               // Mode
        if (! (VALUE (U3MODE) & PIC32_UMODE_ON)) {
            clear_irq (uart_irq[2] + 1);        // deactivate receive interrupt
            clear_irq (uart_irq[2] + 2);        // deactivate transmit interrupt
            VALUE (U3STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                               PIC32_USTA_PERR | PIC32_USTA_UTXBF);
            VALUE (U3STA) |= PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
        }
        return;
    WRITEOPR (U3STA,                // Status and control
        PIC32_USTA_URXDA | PIC32_USTA_FERR | PIC32_USTA_PERR |
        PIC32_USTA_RIDLE | PIC32_USTA_TRMT | PIC32_USTA_UTXBF);
        if (! (VALUE (U3STA) & PIC32_USTA_URXEN)) {
            clear_irq (uart_irq[2] + 1);        // deactivate receive interrupt
            VALUE (U3STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                PIC32_USTA_PERR);
        }
        if (! (VALUE (U3STA) & PIC32_USTA_UTXEN)) {
            clear_irq (uart_irq[2] + 2);        // deactivate transmit interrupt
            VALUE (U3STA) &= ~PIC32_USTA_UTXBF;
            VALUE (U3STA) |= PIC32_USTA_TRMT;
        }
        return;
    WRITEOP (U3BRG); return;        // Baud rate
    READONLY (U3RXREG);             // Receive

    /*-------------------------------------------------------------------------
     * UART 4.
     */
    STORAGE (U4TXREG);              // Transmit
        vtty_put_char (3, data);
        if ((VALUE (U4MODE) & PIC32_UMODE_ON) &&
            (VALUE (U4STA) & PIC32_USTA_UTXEN) && ! uart_oactive[3]) {
            uart_oactive[3] = 1;
            set_irq (uart_irq[3] + 2);          // activate transmit interrupt
        }
        break;
    WRITEOP (U4MODE);               // Mode
        if (! (VALUE (U4MODE) & PIC32_UMODE_ON)) {
            clear_irq (uart_irq[3] + 1);        // deactivate receive interrupt
            clear_irq (uart_irq[3] + 2);        // deactivate transmit interrupt
            VALUE (U4STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                               PIC32_USTA_PERR | PIC32_USTA_UTXBF);
            VALUE (U4STA) |= PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
        }
        return;
    WRITEOPR (U4STA,                // Status and control
        PIC32_USTA_URXDA | PIC32_USTA_FERR | PIC32_USTA_PERR |
        PIC32_USTA_RIDLE | PIC32_USTA_TRMT | PIC32_USTA_UTXBF);
        if (! (VALUE (U4STA) & PIC32_USTA_URXEN)) {
            clear_irq (uart_irq[3] + 1);        // deactivate receive interrupt
            VALUE (U4STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                PIC32_USTA_PERR);
        }
        if (! (VALUE (U4STA) & PIC32_USTA_UTXEN)) {
            clear_irq (uart_irq[3] + 2);        // deactivate transmit interrupt
            VALUE (U4STA) &= ~PIC32_USTA_UTXBF;
            VALUE (U4STA) |= PIC32_USTA_TRMT;
        }
        return;
    WRITEOP (U4BRG); return;        // Baud rate
    READONLY (U4RXREG);             // Receive

    /*-------------------------------------------------------------------------
     * UART 5.
     */
    STORAGE (U5TXREG);              // Transmit
        vtty_put_char (4, data);
        if ((VALUE (U5MODE) & PIC32_UMODE_ON) &&
            (VALUE (U5STA) & PIC32_USTA_UTXEN) && ! uart_oactive[4]) {
            uart_oactive[4] = 1;
            set_irq (uart_irq[4] + 2);          // activate transmit interrupt
        }
        break;
    WRITEOP (U5MODE);               // Mode
        if (! (VALUE (U5MODE) & PIC32_UMODE_ON)) {
            clear_irq (uart_irq[4] + 1);        // deactivate receive interrupt
            clear_irq (uart_irq[4] + 2);        // deactivate transmit interrupt
            VALUE (U5STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                               PIC32_USTA_PERR | PIC32_USTA_UTXBF);
            VALUE (U5STA) |= PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
        }
        return;
    WRITEOPR (U5STA,                // Status and control
        PIC32_USTA_URXDA | PIC32_USTA_FERR | PIC32_USTA_PERR |
        PIC32_USTA_RIDLE | PIC32_USTA_TRMT | PIC32_USTA_UTXBF);
        if (! (VALUE (U5STA) & PIC32_USTA_URXEN)) {
            clear_irq (uart_irq[4] + 1);        // deactivate receive interrupt
            VALUE (U5STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                PIC32_USTA_PERR);
        }
        if (! (VALUE (U5STA) & PIC32_USTA_UTXEN)) {
            clear_irq (uart_irq[4] + 2);        // deactivate transmit interrupt
            VALUE (U5STA) &= ~PIC32_USTA_UTXBF;
            VALUE (U5STA) |= PIC32_USTA_TRMT;
        }
        return;
    WRITEOP (U5BRG); return;        // Baud rate
    READONLY (U5RXREG);             // Receive

    /*-------------------------------------------------------------------------
     * UART 6.
     */
    STORAGE (U6TXREG);              // Transmit
        vtty_put_char (5, data);
        if ((VALUE (U6MODE) & PIC32_UMODE_ON) &&
            (VALUE (U6STA) & PIC32_USTA_UTXEN) && ! uart_oactive[5]) {
            uart_oactive[5] = 1;
            set_irq (uart_irq[5] + 2);          // activate transmit interrupt
        }
        break;
    WRITEOP (U6MODE);               // Mode
        if (! (VALUE (U6MODE) & PIC32_UMODE_ON)) {
            clear_irq (uart_irq[5] + 1);        // deactivate receive interrupt
            clear_irq (uart_irq[5] + 2);        // deactivate transmit interrupt
            VALUE (U6STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                               PIC32_USTA_PERR | PIC32_USTA_UTXBF);
            VALUE (U6STA) |= PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
        }
        return;
    WRITEOPR (U6STA,                // Status and control
        PIC32_USTA_URXDA | PIC32_USTA_FERR | PIC32_USTA_PERR |
        PIC32_USTA_RIDLE | PIC32_USTA_TRMT | PIC32_USTA_UTXBF);
        if (! (VALUE (U6STA) & PIC32_USTA_URXEN)) {
            clear_irq (uart_irq[5] + 1);        // deactivate receive interrupt
            VALUE (U6STA) &= ~(PIC32_USTA_URXDA | PIC32_USTA_FERR |
                PIC32_USTA_PERR);
        }
        if (! (VALUE (U6STA) & PIC32_USTA_UTXEN)) {
            clear_irq (uart_irq[5] + 2);        // deactivate transmit interrupt
            VALUE (U6STA) &= ~PIC32_USTA_UTXBF;
            VALUE (U6STA) |= PIC32_USTA_TRMT;
        }
        return;
    WRITEOP (U6BRG); return;        // Baud rate
    READONLY (U6RXREG);             // Receive

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

    /*
     * UARTs 1-6.
     */
    VALUE (U1MODE)  = 0;
    VALUE (U1STA)   = PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
    VALUE (U1TXREG) = 0;
    VALUE (U1RXREG) = 0;
    VALUE (U1BRG)   = 0;
    VALUE (U2MODE)  = 0;
    VALUE (U2STA)   = PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
    VALUE (U2TXREG) = 0;
    VALUE (U2RXREG) = 0;
    VALUE (U2BRG)   = 0;
    VALUE (U3MODE)  = 0;
    VALUE (U3STA)   = PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
    VALUE (U3TXREG) = 0;
    VALUE (U3RXREG) = 0;
    VALUE (U3BRG)   = 0;
    VALUE (U4MODE)  = 0;
    VALUE (U4STA)   = PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
    VALUE (U4TXREG) = 0;
    VALUE (U4RXREG) = 0;
    VALUE (U4BRG)   = 0;
    VALUE (U5MODE)  = 0;
    VALUE (U5STA)   = PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
    VALUE (U5TXREG) = 0;
    VALUE (U5RXREG) = 0;
    VALUE (U5BRG)   = 0;
    VALUE (U6MODE)  = 0;
    VALUE (U6STA)   = PIC32_USTA_RIDLE | PIC32_USTA_TRMT;
    VALUE (U6TXREG) = 0;
    VALUE (U6RXREG) = 0;
    VALUE (U6BRG)   = 0;
}

void io_init (void *datap, void *data2p, void *bootp)
{
    iomem = datap;
    iomem2 = data2p;
    bootmem = bootp;

    // Preset DEVCFG data, from Max32 bootloader.
    BOOTMEM(DEVCFG3) = 0xffff0722;
    BOOTMEM(DEVCFG2) = 0xd979f8f9;
    BOOTMEM(DEVCFG1) = 0x5bfd6aff;
    BOOTMEM(DEVCFG0) = 0xffffff7f;

    io_reset();
}
