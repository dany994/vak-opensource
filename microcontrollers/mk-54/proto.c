/*
 * Prototype based on Olimex PIC32-T795 board.
 *
 * Copyright (C) 2013 Serge Vakulenko, <serge@vak.ru>
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
#include "calc.h"
#include "pic32mx.h"

#define MHZ     80              /* CPU clock is 80 MHz. */

#define PIN(n)  (1 << (n))

/*
 * Main entry point at bd003000.
 * Setup stack pointer and $gp registers, and jump to main().
 */
asm ("          .section .exception");
asm ("          .globl _start");
asm ("          .type _start, function");
asm ("_start:   la      $sp, _estack");
asm ("          la      $ra, main");
asm ("          la      $gp, _gp");
asm ("          jr      $ra");
asm ("          .text");

/*
 * Secondary entry point at bd004000.
 */
asm ("          .section .startup");
asm ("          .globl _init");
asm ("          .type _init, function");
asm ("_init:    la      $ra, _start");
asm ("          jr      $ra");
asm ("          .text");

/*
 * Delay for a given number of microseconds.
 * The processor has a 32-bit hardware Count register,
 * which increments at half CPU rate.
 * We use it to get a precise delay.
 */
void udelay (unsigned usec)
{
    unsigned now = mfc0 (C0_COUNT, 0);
    unsigned final = now + usec * MHZ / 2;

    for (;;) {
        now = mfc0 (C0_COUNT, 0);

        /* This comparison is valid only when using a signed type. */
        if ((int) (now - final) >= 0)
            break;
    }
}

//
// Display a symbol on 7-segment LED
//
void set_segments (unsigned digit, unsigned dot)
{
    static const unsigned segments[16] = {
    //- A - B - C - D - E -- F -- G --
        1 + 2 + 4 + 8 + 16 + 32,        // digit 0
            2 + 4,                      // digit 1
        1 + 2     + 8 + 16      + 64,   // digit 2
        1 + 2 + 4 + 8           + 64,   // digit 3
            2 + 4          + 32 + 64,   // digit 4
        1     + 4 + 8      + 32 + 64,   // digit 5
        1     + 4 + 8 + 16 + 32 + 64,   // digit 6
        1 + 2 + 4,                      // digit 7
        1 + 2 + 4 + 8 + 16 + 32 + 64,   // digit 8
        1 + 2 + 4 + 8      + 32 + 64,   // digit 9
                                  64,   // symbol -
                    8 + 16 + 32,        // symbol L
        1 +         8 + 16 + 32,        // symbol C
        1 +             16 + 32,        // symbol Г
        1 +         8 + 16 + 32 + 64,   // symbol E
        0,                              // empty
    };

    switch (digit) {
    case '-': digit = 10; break;
    case 'L': digit = 11; break;
    case 'C': digit = 12; break;
    case 'R': digit = 13; break;
    case 'E': digit = 14; break;
    case ' ': digit = 15; break;
    }
    unsigned mask = segments [digit & 15];

    if (mask & 1)   TRISECLR = PIN(0);  // segment A - signal RE0
    if (mask & 2)   TRISECLR = PIN(1);  // segment B - signal RE1
    if (mask & 4)   TRISECLR = PIN(2);  // segment C - signal RE2
    if (mask & 8)   TRISECLR = PIN(3);  // segment D - signal RE3
    if (mask & 16)  TRISECLR = PIN(4);  // segment E - signal RE4
    if (mask & 32)  TRISECLR = PIN(5);  // segment F - signal RE5
    if (mask & 64)  TRISECLR = PIN(6);  // segment G - signal RE6
    if (dot)        TRISECLR = PIN(7);  // dot       - signal RE7
}

//
// Set or clear data signal.
//
static inline void clear_segments()
{
    TRISESET = 0xff;                    // tristate
}

//
// Toggle clock signal.
//
static inline void clk()
{
    LATFSET = PIN(0);                   // set clock
    udelay (1);                         // 1 usec
    LATFCLR = PIN(0);                   // clear clock
}

//
// Set or clear data signal.
//
static inline void data (int on)
{
    if (on)
        LATFSET = PIN(1);               // set data
    else
        LATFCLR = PIN(1);               // clear data
}

//
// Symbols on display.
//
unsigned char display [12];
unsigned char dot [12];                 // Show a decimal dot

//
// Show the display contents for a given number of milliseconds.
//
void show (unsigned msec)
{
    unsigned now = mfc0 (C0_COUNT, 0);
    unsigned final = now + msec * MHZ * 500;
    int i;

    for (;;) {
        now = mfc0 (C0_COUNT, 0);

        /* This comparison is valid only when using a signed type. */
        if ((int) (now - final) >= 0)
            break;

        data (1);                       // set data
        clk();                          // toggle clock
        data (0);                       // clear data

        for (i=0; i<12; i++) {
            clk();                      // toggle clock
            set_segments (display[11-i], dot[11-i]);
            udelay (1000);              // 1 msec
            clear_segments();
        }
    }
}

//
// Simulate one cycle of the calculator.
// Return 0 when stopped, or 1 when running a user program.
//
void step (unsigned x, unsigned y)
{
    show (500);
    calc_step (x, y, 10, display, dot);
    calc_step (0, 0, 10, display, dot);
}

int main()
{
    /* Set memory wait states, for speedup. */
    CHECON = 2;
    BMXCONCLR = 0x40;
    CHECONSET = 0x30;

    /* Enable cache for kseg0 segment. */
    int config = mfc0 (C0_CONFIG, 0);
    mtc0 (C0_CONFIG, 0, config | 3);

    /* Initialize coprocessor 0. */
    mtc0 (C0_COUNT, 0, 0);
    mtc0 (C0_COMPARE, 0, -1);
    mtc0 (C0_EBASE, 1, 0x9fc00000);     /* Vector base */
    mtc0 (C0_INTCTL, 1, 1 << 5);        /* Vector spacing 32 bytes */
    mtc0 (C0_CAUSE, 0, 1 << 23);        /* Set IV */
    mtc0 (C0_STATUS, 0, 0);             /* Clear BEV */

    /* Disable JTAG and Trace ports, to make more pins available. */
    DDPCONCLR = 3 << 2;

    /* Use all ports as digital. */
    AD1PCFG = ~0;
    LATA = 0;
    LATB = 0;
    LATE = 0;
    LATF = 0;

    /* Use pins RB15, RB12 as output: LED1 and LED2 control. */
    TRISBCLR = PIN(15);
    TRISBCLR = PIN(12);

    /*
     * RF0 - clock
     * RF1 - data
     */
    TRISFCLR = PIN(0) | PIN(1);

    /*
     * RE0 - segment A
     * RE1 - segment B
     * RE2 - segment C
     * RE3 - segment D
     * RE4 - segment E
     * RE5 - segment F
     * RE6 - segment G
     * RE7 - dot
     */
    TRISESET = 0xff;                    // tristate
    LATECLR = 0xff;

    int i;
    data (0);                           // clear data
    for (i=0; i<16; i++)                // clear register
        clk();

    calc_init();
    calc_step (0, 0, 10, display, dot);

    for (;;) {
        step (5,  1);           // 3
        step (6,  1);           // 4
        step (11, 8);           // B^
        step (9,  1);           // 7
        step (5,  8);           // /
        step (3,  1);           // 1
        step (9,  8);           // ВП
        step (7,  1);           // 5
        step (2,  1);           // 0
        step (11, 9);           // F
        step (4,  8);           // x^2
        step (11, 9);           // F
        step (4,  8);           // x^2
        step (10, 8);           // Cx
    }
}
