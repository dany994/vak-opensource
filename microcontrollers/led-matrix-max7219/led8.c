/*
 * Demo for LED matrix 8x8, connected to SPI3 port of PIC32
 * via MAX7219 serial display driver.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bitbang.h"

/*
 * Use FT232R adapter to connect to MAX7219.
 * /CS - DTR
 * CLK - CTS
 * DIN - TXD
 */
#define MASK_CS     MASK_DTR
#define MASK_CLK    MASK_CTS
#define MASK_DIN    MASK_TXD

/*
 * Max7219 commands.
 */
#define MAX_NOOP            0x00
#define MAX_DIGIT0          0x01
#define MAX_DIGIT1          0x02
#define MAX_DIGIT2          0x03
#define MAX_DIGIT3          0x04
#define MAX_DIGIT4          0x05
#define MAX_DIGIT5          0x06
#define MAX_DIGIT6          0x07
#define MAX_DIGIT7          0x08
#define MAX_DECODE_MODE     0x09
#define MAX_INTENSITY       0x0a
#define MAX_SCAN_LIMIT      0x0b
#define MAX_SHUTDOWN        0x0c
#define MAX_DISPLAY_TEST    0x0f

void max_command(int addr, int byte)
{
    unsigned char output[64];
    int nbytes = 0, mask = 0, n;

    output[nbytes++] = mask | MASK_CS;      // /CS high
    output[nbytes++] = mask;                // /CS low

    /* Send address. */
    for (n=0; n<8; n++) {
        if (addr & 0x80)
            mask |= MASK_DIN;
        else
            mask &= ~MASK_DIN;
        output[nbytes++] = mask;            // set DIN

        output[nbytes++] = mask | MASK_CLK; // set CLK
        output[nbytes++] = mask;            // clear CLK

        addr <<= 1;
    }

    /* Send data byte. */
    for (n=0; n<8; n++) {
        if (byte & 0x80)
            mask |= MASK_DIN;
        else
            mask &= ~MASK_DIN;
        output[nbytes++] = mask;            // set DIN

        output[nbytes++] = mask | MASK_CLK; // set CLK
        output[nbytes++] = mask;            // clear CLK

        byte <<= 1;
    }
    output[nbytes++] = mask | MASK_CS;      // /CS high

    bitbang_io(output, nbytes, 0);
}

int main()
{
    /* Use FT232R adapter to connect to MAX7219. */
    if (bitbang_open("FT232R USB UART", MASK_CS | MASK_CLK | MASK_DIN) < 0) {
        printf("Cannot connect to FT232R adapter.\n");
        exit(-1);
    }

    /* Set /CS high, CLK and DIN low. */
    unsigned char c = MASK_CS;
    bitbang_io(&c, 1, 0);

    max_command(MAX_SCAN_LIMIT, 7);   /* display digits 0-7 */
    max_command(MAX_DECODE_MODE, 0);  /* no code B decode */
    max_command(MAX_SHUTDOWN, 1);     /* no shutdown mode */
    max_command(MAX_DISPLAY_TEST, 0); /* no display test */
    max_command(MAX_INTENSITY, 1);    /* minimum intensity (from 1 to 15) */

    for (;;) {
        max_command(MAX_DIGIT0, 0x01);
        max_command(MAX_DIGIT1, 0x02);
        max_command(MAX_DIGIT2, 0x04);
        max_command(MAX_DIGIT3, 0x08);
        max_command(MAX_DIGIT4, 0x10);
        max_command(MAX_DIGIT5, 0x20);
        max_command(MAX_DIGIT6, 0x40);
        max_command(MAX_DIGIT7, 0x80);

        usleep(500000);

        max_command(MAX_DIGIT0, 0x80);
        max_command(MAX_DIGIT1, 0x40);
        max_command(MAX_DIGIT2, 0x20);
        max_command(MAX_DIGIT3, 0x10);
        max_command(MAX_DIGIT4, 0x08);
        max_command(MAX_DIGIT5, 0x04);
        max_command(MAX_DIGIT6, 0x02);
        max_command(MAX_DIGIT7, 0x01);

        usleep(500000);
    }
    bitbang_close();
    return 0;
}
