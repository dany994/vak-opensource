/*
 * Define processor and include header file.
 */
#include "pic32mx.h"

/*
 * Setup chip configuration
 */
PIC32_DEVCFG (
    DEVCFG0_DEBUG_ENABLED,      /* ICE debugger disabled */

    DEVCFG1_FNOSC_PRIPLL |      /* Primary oscillator with PLL */
    DEVCFG1_POSCMOD_XT |        /* XT oscillator */
    DEVCFG1_OSCIOFNC |          /* CLKO output active */
    DEVCFG1_FPBDIV_8 |          /* Peripheral bus clock = SYSCLK/8 */
    DEVCFG1_FCKM_DISABLE |      /* Fail-safe clock monitor disable */
    DEVCFG1_FCKS_DISABLE |      /* Clock switching disable */
    DEVCFG1_WDTPS_1024,         /* Watchdog postscale = 1/1024 */

    DEVCFG2_FPLLIDIV_2 |        /* PLL divider = 1/2 */
    DEVCFG2_FPLLMUL_20 |        /* PLL multiplier = 20x */
    DEVCFG2_UPLLIDIV_2 |        /* USB PLL divider = 1/2 */
    DEVCFG2_FPLLODIV_1,         /* PLL postscaler = 1/1 */

    DEVCFG3_USERID(0xffff) |    /* User-defined ID */
    DEVCFG3_FSRSSEL_7 |         /* Assign irq priority 7 to shadow set */
    DEVCFG3_FETHIO);            /* Default Ethernet i/o pins */

#if 0
void mdelay (unsigned msec)
{
    unsigned i;

    while (msec-- > 0) {
        for (i=0; i<200; i++)
            asm volatile ("nop");
    }
}
#endif

int main()
{
    TRISACLR = 1 << 3;
    while (1) {
            LATAINV = 1 << 3;
//        PORTAINV = 1 << 3;
//            mdelay (250);
//        PORTAINV = 1 << 3;
//            mdelay (250);
    }
}

asm ("          .section .exception");
asm ("_start:   la      $sp, _estack");
asm ("          la      $gp, _gp");
asm ("          j       main");
asm ("          .text");
