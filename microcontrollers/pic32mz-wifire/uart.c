/*
 * Include processor definitions.
 */
#include "pic32mz.h"

#define MHZ     200             /* CPU clock. */

/*
 * Secondary entry point at bd000000.
 */
asm ("          .section .exception");
asm ("          .globl _init");
asm ("          .type _init, function");
asm ("_init:    la      $ra, _start");
asm ("          jr      $ra");
asm ("          .org    0xfc");
asm ("          .word   -1");           /* Image header pointer. */
asm ("          .text");

/*
 * Main entry point at bd001000.
 * Setup stack pointer and $gp registers, and jump to main().
 */
asm ("          .section .startup");
asm ("          .globl _start");
asm ("          .type _start, function");
asm ("_start:   la      $sp, _estack");
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

/*
 * Send a byte to the UART transmitter, with interrupts disabled.
 */
void putch (unsigned char c)
{
    /* Wait for transmitter shift register empty. */
    while (! (U4STA & PIC32_USTA_TRMT))
        continue;

again:
    /* Send byte. */
    U4TXREG = c;

    /* Wait for transmitter shift register empty. */
    while (! (U4STA & PIC32_USTA_TRMT))
        continue;

    if (c == '\n') {
        c = '\r';
        goto again;
    }
}

/*
 * Wait for the byte to be received and return it.
 */
unsigned getch (void)
{
    unsigned c;

    for (;;) {
        /* Wait until receive data available. */
        if (U4STA & PIC32_USTA_URXDA) {
            c = (unsigned char) U4RXREG;
            break;
        }
    }
    return c;
}

int hexchar (unsigned val)
{
    return "0123456789abcdef" [val & 0xf];
}

void printreg (const char *p, unsigned val)
{
    for (; *p; ++p)
        putch (*p);

    putch ('=');
    putch (' ');
    putch (hexchar (val >> 28));
    putch (hexchar (val >> 24));
    putch (hexchar (val >> 20));
    putch (hexchar (val >> 16));
    putch (hexchar (val >> 12));
    putch (hexchar (val >> 8));
    putch (hexchar (val >> 4));
    putch (hexchar (val));
    putch ('\n');
}

int main()
{
#if 0
    /* Initialize UART. */
    U4BRG = PIC32_BRG_BAUD (MHZ * 500000, 115200);
    U4STA = 0;
    U4MODE = PIC32_UMODE_PDSEL_8NPAR |      /* 8-bit data, no parity */
             PIC32_UMODE_ON;                /* UART Enable */
    U4STASET = PIC32_USTA_URXEN |           /* Receiver Enable */
               PIC32_USTA_UTXEN;            /* Transmit Enable */
#endif
    /*
     * Print initial state of control registers.
     */
    putch ('-');
    putch ('\n');
    printreg ("Status  ", mfc0(12, 0));
    printreg ("IntCtl  ", mfc0(12, 1));
    printreg ("SRSCtl  ", mfc0(12, 2));
    printreg ("Cause   ", mfc0(13, 0));
    printreg ("PRId    ", mfc0(15, 0));
    printreg ("EBase   ", mfc0(15, 1));
    printreg ("CDMMBase", mfc0(15, 2));
    printreg ("Config  ", mfc0(16, 0));
    printreg ("Config1 ", mfc0(16, 1));
    printreg ("Config2 ", mfc0(16, 2));
    printreg ("Config3 ", mfc0(16, 3));
    printreg ("Config4 ", mfc0(16, 4));
    printreg ("Config5 ", mfc0(16, 5));
    printreg ("Config7 ", mfc0(16, 7));
    printreg ("WatchHi ", mfc0(19, 0));
    printreg ("WatchHi1", mfc0(19, 1));
    printreg ("WatchHi2", mfc0(19, 2));
    printreg ("WatchHi3", mfc0(19, 3));
    printreg ("Debug   ", mfc0(23, 0));
    printreg ("PerfCtl0", mfc0(25, 0));
    printreg ("PerfCtl1", mfc0(25, 2));

    while (1) {
        /* Stop simulation. */
        asm volatile ("sltiu $zero, $zero, 0xABC2");

        putch ('.');
    }
}
