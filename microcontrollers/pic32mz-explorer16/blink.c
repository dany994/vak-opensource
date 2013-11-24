/*
 * Include processor definitions.
 */
#include "pic32mz.h"

#define MHZ     80              /* CPU clock is 80 MHz. */

/*
 * Chip configuration.
 */
PIC32_DEVCFG (
    DEVCFG0_JTAG_DISABLE |      /* Disable JTAG port */
    DEVCFG0_TRC_DISABLE,        /* Disable trace port */

//    DEVCFG1_FNOSC_POSC |        /* Primary oscillator */
//    DEVCFG1_POSCMOD_HS |        /* HS oscillator */
    DEVCFG1_FNOSC_FRCDIV |      /* Fast RC with divide-by-N */
    DEVCFG1_CLKO_DISABLE,       /* CLKO output disable */

    DEVCFG2_FPLLRNG_8_16 |      /* PLL input range is 8-16 MHz */
    DEVCFG2_FPLLIDIV_1 |        /* PLL divider = 1 */
    DEVCFG1_FPLLMULT(40) |      /* PLL multiplier = 40x */
    DEVCFG2_FPLLODIV_2,         /* PLL postscaler = 1/2 */

    DEVCFG3_USERID(0xffff));    /* User-defined ID */

PIC32_DEVSIGN (0x7fffffff,
               0xffffffff,
               0xffffffff,
               0xffffffff);

/*
 * Boot code at bfc00000.
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
asm ("_estack   = _end + 0x1000");

volatile unsigned loop;

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

int main()
{
    /* Initialize coprocessor 0. */
    mtc0 (C0_COUNT, 0, 0);
    mtc0 (C0_COMPARE, 0, -1);
    mtc0 (C0_EBASE, 1, 0x9fc00000);     /* Vector base */
    mtc0 (C0_INTCTL, 1, 1 << 5);        /* Vector spacing 32 bytes */
    mtc0 (C0_CAUSE, 0, 1 << 23);        /* Set IV */
    mtc0 (C0_STATUS, 0, 0);             /* Clear BEV */

    /* Use pins PA0-PA7 as output: LED control. */
    LATACLR = 0xFF;
    TRISACLR = 0xFF;

    while (1) {
        /* Invert pins PA7-PA0. */
        LATAINV = 1 << 0; udelay (100000);
        LATAINV = 1 << 1; udelay (100000);
        LATAINV = 1 << 2; udelay (100000);
        LATAINV = 1 << 3; udelay (100000);
        LATAINV = 1 << 4; udelay (100000);
        LATAINV = 1 << 5; udelay (100000);
        LATAINV = 1 << 6; udelay (100000);
        LATAINV = 1 << 7; udelay (100000);
        loop++;
    }
}
