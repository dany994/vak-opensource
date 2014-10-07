/*
 * Include processor definitions.
 */
#include "pic32mx.h"

#define MHZ     80              /* CPU clock is 80 MHz. */

/*
 * Entry point at 9d07c000.
 */
asm ("          .section .startup,\"ax\",@progbits");
asm ("          .globl _init");
asm ("          .type _init, function");
asm ("_start:   la      $ra, _init");
asm ("          jr      $ra");
asm ("          .text");

/*
 * Exception section at 9d000000.
 * Setup stack pointer and $gp registers, and jump to main().
 */
asm ("          .section .exception,\"ax\",@progbits");
asm ("          .globl _start");
asm ("          .type _start, function");
asm ("_init:    la      $sp, _estack");
asm ("          la      $ra, main");
asm ("          la      $gp, _gp");
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

int main()
{
    /* Set memory wait states, for speedup. */
    //CHECON = 2;
    //BMXCONCLR = 0x40;
    //CHECONSET = 0x30;

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
    //DDPCONCLR = 3 << 2;

    /* Use all ports as digital. */
    AD1PCFG = ~0;
    LATB = 0;
    LATG = 0;

    /* Use pin RG6 as output: LED2 control. */
    TRISGCLR = 1 << 6;

    /* Use pin RD6 as output: LED3 control. */
    TRISDCLR = 1 << 6;
    LATDSET = 1 << 6;

    for (;;) {
        /* Invert pin RG6. */
        LATGINV = 1 << 6;

        /* Invert pin RD6. */
        LATDINV = 1 << 6;

        /* Delay. */
        udelay (500000);
    }
}
