/*
 * Include processor definitions.
 */
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
void set_segments (unsigned digit)
{
    static const unsigned pattern[16] = {
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
    unsigned mask = pattern [digit & 15];

    if (mask & 1)   TRISECLR = PIN(0);  // signal RE0
    if (mask & 2)   TRISECLR = PIN(1);  // signal RE1
    if (mask & 4)   TRISECLR = PIN(2);  // signal RE2
    if (mask & 8)   TRISECLR = PIN(3);  // signal RE3
    if (mask & 16)  TRISECLR = PIN(4);  // signal RE4
    if (mask & 32)  TRISECLR = PIN(5);  // signal RE5
    if (mask & 64)  TRISECLR = PIN(6);  // signal RE6
    if (mask & 128) TRISECLR = PIN(7);  // signal RE7
}

void toggle_clk()
{
    LATFSET = PIN(0);               // set clock
    udelay (1);                     // 1 usec
    LATFCLR = PIN(0);               // clear clock
}

int main()
{
    char display[12] = "-013579 CREL";

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

    /* Use pin RB15 as output: LED1 control. */
    TRISBCLR = PIN(15);

    /* Use pin RB12 as output: LED2 control. */
    TRISBCLR = PIN(12);
    LATBSET = PIN(12);

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

    int count = 0;
    for (;;) {
        if (++count >= 40) {
            count = 0;

            /* Invert pin RB15. */
            LATBINV = PIN(15);

            /* Invert pin RB12. */
            LATBINV = PIN(12);
        }

        LATFCLR = PIN(1);               // clear data
        int i;
        for (i=0; i<16; i++)            // clear register
            toggle_clk();

        LATFSET = PIN(1);               // set data
        toggle_clk();
        LATFCLR = PIN(1);               // clear data

        for (i=0; i<12; i++) {
            toggle_clk();
            set_segments (display[11-i]);
            udelay (1000);              // 1 msec
            TRISESET = 0xff;            // tristate
        }
    }
}
