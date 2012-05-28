/*
 * Include processor definitions.
 */
#include "pic32mx.h"

/*
 * Chip configuration.
 */
PIC32_DEVCFG (
    DEVCFG0_DEBUG_DISABLED,     /* ICE debugger disabled */

    DEVCFG1_FNOSC_FRCPLL |      /* Fast RC oscillator with PLL */
    DEVCFG1_POSCMOD_DISABLE |   /* Primary oscillator disabled */
    DEVCFG1_FPBDIV_1 |          /* Peripheral bus clock = SYSCLK/1 */
    DEVCFG1_OSCIOFNC_OFF |      /* CLKO output disable */
    DEVCFG1_FCKM_DISABLE |      /* Fail-safe clock monitor disable */
    DEVCFG1_FCKS_DISABLE,       /* Clock switching disable */

    DEVCFG2_FPLLIDIV_2 |        /* PLL divider = 1/2 */
    DEVCFG2_FPLLMUL_20 |        /* PLL multiplier = 20x */
    DEVCFG2_UPLLIDIV_2 |        /* USB PLL divider = 1/2 */
    DEVCFG2_UPLLDIS |           /* Disable USB PLL */
    DEVCFG2_FPLLODIV_2,         /* PLL postscaler = 1/2 */

    DEVCFG3_USERID(0xffff) |    /* User-defined ID */
    DEVCFG3_FSRSSEL_7);         /* Assign irq priority 7 to shadow set */

/*
 * Boot code.
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
 * Delay for a given number of milliseconds.
 */
void mdelay (unsigned msec)
{
    unsigned now = mfc0 (C0_COUNT, 0);
    unsigned final = now + msec * MHZ * 500;

    for (;;) {
        now = mfc0 (C0_COUNT, 0);

        /* This comparison is valid only when using a signed type. */
        if ((int) (now - final) >= 0)
            break;
    }
}

int main()
{
    /* Initialize coprocessor 0. */
    mtc0 (C0_COUNT, 0, 0);
    mtc0 (C0_COMPARE, 0, -1);
    mtc0 (C0_EBASE, 1, 0x9fc00000);     /* Vector base */
    mtc0 (C0_INTCTL, 1, 1 << 5);        /* Vector spacing 32 bytes */
    mtc0 (C0_CAUSE, 0, 1 << 23);        /* Set IV */
    mtc0 (C0_STATUS, 0, 0);             /* Clear BEV */

    /* Disable JTAG port, to make all LEDs available. */
    DDPCON = 0;

    /* Use all ports as digital. */
    ANSELA = 0;
    ANSELB = 0;

    /* All PORTA, PORTB as output. */
    LATA = 0;
    LATB = 0;
    TRISA = 0;
    TRISB = 0;

    int value = 1;
    int dir = 1;
    for (;;) {
        LATA = value & 0x1f;
        LATB = (value >> 5) & 0xffff;
        if (dir > 0)
            value <<= 1;
        else
            value >>= 1;

        if (! (value & 0x1fffff)) {
            /* Reverse direction */
            dir = -dir;
            value = 2;
            if (dir < 0)
                value <<= 18;
        }

        mdelay (100);
    }
}
