/*
 * Include processor definitions.
 */
#include "pic32mx.h"

/*
 * Chip configuration.
 */
PIC32_DEVCFG (
    DEVCFG0_DEBUG_ENABLED,      /* ICE debugger disabled */

    DEVCFG1_FNOSC_FRCPLL |      /* Fast RC oscillator with PLL */
    DEVCFG1_POSCMOD_DISABLE |   /* Primary oscillator disabled */
    DEVCFG1_OSCIOFNC |          /* CLKO output active */
    DEVCFG1_FPBDIV_2 |          /* Peripheral bus clock = SYSCLK/2 */
    DEVCFG1_FCKM_DISABLE |      /* Fail-safe clock monitor disable */
    DEVCFG1_FCKS_DISABLE |      /* Clock switching disable */
    DEVCFG1_WDTPS_1024,         /* Watchdog postscale = 1/1024 */

    DEVCFG2_FPLLIDIV_2 |        /* PLL divider = 1/2 */
    DEVCFG2_FPLLMUL_20 |        /* PLL multiplier = 20x */
    DEVCFG2_UPLLIDIV_2 |        /* USB PLL divider = 1/2 */
    DEVCFG2_UPLLDIS |           /* Disable USB PLL */
    DEVCFG2_FPLLODIV_1,         /* PLL postscaler = 1/1 */

    DEVCFG3_USERID(0xffff) |    /* User-defined ID */
    DEVCFG3_FSRSSEL_7 |         /* Assign irq priority 7 to shadow set */
    DEVCFG3_FETHIO);            /* Default Ethernet i/o pins */

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

int main()
{
    /* Initialize coprocessor 0. */
    mtc0 (C0_COUNT, 0, 0);
    mtc0 (C0_COMPARE, 0, -1);
    mtc0 (C0_EBASE, 1, 0x9fc00000);     /* Vector base */
    mtc0 (C0_INTCTL, 1, 1 << 5);        /* Vector spacing 32 bytes */
    mtc0 (C0_CAUSE, 0, 1 << 23);        /* Set IV */
    mtc0 (C0_STATUS, 0, 0);             /* Clear BEV */
#if 0
    /* Setup wait states. */
    CHECON = 2;
    BMXCONCLR = 0x40;
    CHECONSET = 0x30;
#endif
    /* Disable JTAG port, to make all LEDs available. */
    DDPCON = 0;

    /* Use all B ports as digital. */
    ANSELA = 0;
    ANSELB = 0;

    /* Use pin RA4 as output: LED1 control. */
    TRISACLR = 1 << 4;
    PORTACLR = 1 << 4;

    /* Use pin RB8 as output: LED2 control. */
    TRISBCLR = 1 << 8;
    PORTBSET = 1 << 8;

    while (1) {

        /* Invert pin RA4. */
        PORTAINV = 1 << 4;

        /* Invert pin RB8. */
        PORTBINV = 1 << 8;

        /* Delay. */
        unsigned i;
        for (i=0; i<4000000; i++)
            asm volatile ("");
    }
}
