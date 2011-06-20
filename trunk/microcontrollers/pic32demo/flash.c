/*
 * Define processor and include header file.
 */
#include "pic32mx.h"

/*
 * Setup chip configuration
 */
#if 0
unsigned __at 0x2007 __CONFIG =
    _FCMEN_OFF &            /* Fail-Safe Clock Monitor disabled */
    _IESO_OFF &             /* Internal External Switchover disabled */
    _BOR_OFF &              /* Brown-out Reset disabled */
    _CPD_OFF &              /* Data memory protection disabled */
    _CP_OFF &               /* Program memory protection disabled */
    _MCLRE_ON &             /* MCLR pin function is reset */
    _PWRTE_ON &             /* Power-up Timer enabled */
    _WDT_OFF &              /* Watchdog Timer disabled */
    _INTRC_OSC_NOCLKOUT;    /* Internal RC oscillator,
                             * I/O function on RA4 pin */
#endif

void mdelay (unsigned msec)
{
    unsigned i;

    while (msec-- > 0) {
        for (i=0; i<200; i++)
            asm volatile ("nop");
    }
}

int main()
{
    TRISA &= ~(1 << 3);
    while (1) {
        PORTA |= (1 << 3);
            mdelay (250);
        PORTA &= ~(1 << 3);
            mdelay (250);
    }
}

asm (".section .exception");
asm ("_start: j main");
asm (".text");
