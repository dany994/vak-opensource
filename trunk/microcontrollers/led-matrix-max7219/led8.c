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

void max_init()
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
}

/*
 * Twelve pictures from Master Kit NM5101 kit.
 */
#define ROW(a,b,c,d,e,f,g) (a | b<<1 | c<<2 | d<<3 | e<<4 | f<<5 | g<<6)
#define _ 0
#define O 1

unsigned char pic_heart[] = {
    ROW (_,O,O,_,O,O,_),
    ROW (O,O,O,O,O,O,O),
    ROW (O,O,O,O,O,O,O),
    ROW (O,O,O,O,O,O,O),
    ROW (_,O,O,O,O,O,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,O,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,O,O,_,O,O,_),
    ROW (O,O,O,O,O,O,O),
    ROW (O,O,O,O,O,O,O),
    ROW (O,O,O,O,O,O,O),
    ROW (_,O,O,O,O,O,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,O,_,_,_),

    0xff
};

unsigned char pic_bar[] = {
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),

    ROW (_,_,_,_,_,_,O),
    ROW (_,_,_,_,_,O,_),
    ROW (_,_,_,_,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,O,_,_,_,_),
    ROW (_,O,_,_,_,_,_),
    ROW (O,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (O,O,O,O,O,O,O),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (O,_,_,_,_,_,_),
    ROW (_,O,_,_,_,_,_),
    ROW (_,_,O,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,O,_,_),
    ROW (_,_,_,_,_,O,_),
    ROW (_,_,_,_,_,_,O),

    0xff
};

unsigned char pic_coin[] = {
    ROW (_,_,O,O,O,_,_),
    ROW (_,O,_,O,_,O,_),
    ROW (O,_,_,O,_,_,O),
    ROW (O,_,_,O,_,_,O),
    ROW (O,_,O,_,O,_,O),
    ROW (_,O,_,_,_,O,_),
    ROW (_,_,O,O,O,_,_),

    ROW (_,_,O,O,O,_,_),
    ROW (_,O,_,O,_,O,_),
    ROW (_,O,_,O,_,O,_),
    ROW (_,O,_,O,_,O,_),
    ROW (_,O,O,_,O,O,_),
    ROW (_,O,_,_,_,O,_),
    ROW (_,_,O,O,O,_,_),

    ROW (_,_,_,O,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,_,O,_,_,_),

    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),

    0xff
};

unsigned char pic_cross[] = {
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (O,O,O,O,O,O,O),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),

    ROW (O,_,_,_,_,_,O),
    ROW (_,O,_,_,_,O,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,O,_,_,_,O,_),
    ROW (O,_,_,_,_,_,O),

    0xff
};

unsigned char pic_bird[] = {
    ROW (_,_,_,_,_,_,_),
    ROW (O,_,_,_,_,_,O),
    ROW (_,O,_,_,_,O,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (O,_,_,_,_,_,O),
    ROW (_,O,O,_,O,O,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (O,O,O,_,O,O,O),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,O,O,_,O,O,_),
    ROW (O,_,_,O,_,_,O),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    0xff
};

unsigned char pic_asterisk[] = {
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,O,_,O,_,O,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,O,_,O,_,O,_),
    ROW (_,_,_,_,_,_,_),

    ROW (O,_,_,O,_,_,O),
    ROW (_,O,_,O,_,O,_),
    ROW (_,_,O,O,O,_,_),
    ROW (O,O,O,O,O,O,O),
    ROW (_,_,O,O,O,_,_),
    ROW (_,O,_,O,_,O,_),
    ROW (O,_,_,O,_,_,O),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    0xff
};

unsigned char pic_snow[] = {
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (O,O,O,O,O,O,O),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),

    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (O,O,O,_,O,O,O),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),

    ROW (_,_,_,O,_,_,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,O,_,_,_,O,_),
    ROW (O,O,_,_,_,O,O),
    ROW (_,O,_,_,_,_,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,_,_,O,_,_,_),

    ROW (O,O,O,O,O,O,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,O,O,O,O,O,O),

    0xff
};

unsigned char pic_fire[] = {
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,O,_,_,_,_,_),
    ROW (_,O,_,_,O,_,_),
    ROW (_,O,_,O,O,_,O),
    ROW (O,O,O,O,O,O,O),
    ROW (O,O,O,O,O,O,O),

    ROW (_,_,_,_,_,_,_),
    ROW (O,_,_,_,_,_,_),
    ROW (O,_,_,_,_,_,_),
    ROW (O,_,O,_,O,_,_),
    ROW (O,O,O,O,O,O,_),
    ROW (O,O,O,O,O,O,O),
    ROW (O,O,O,O,O,O,O),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,O),
    ROW (_,_,_,O,_,O,O),
    ROW (_,O,_,O,O,O,O),
    ROW (O,O,O,O,O,O,O),
    ROW (O,O,O,O,O,O,O),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,O,_,_,O),
    ROW (_,O,_,O,_,O,O),
    ROW (O,O,O,O,O,O,O),
    ROW (O,O,O,O,O,O,O),

    0xff
};

unsigned char pic_square[] = {
    ROW (O,O,O,O,O,O,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,O,O,O,O,O,O),

    ROW (_,_,_,_,_,_,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,O,_,_,_,O,_),
    ROW (_,O,_,_,_,O,_),
    ROW (_,O,_,_,_,O,_),
    ROW (_,O,O,O,O,O,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    0xff
};

unsigned char pic_round[] = {
    ROW (_,_,O,O,O,_,_),
    ROW (_,O,_,_,_,O,_),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (_,O,_,_,_,O,_),
    ROW (_,_,O,O,O,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,O,_,_,_,O,_),
    ROW (_,O,_,_,_,O,_),
    ROW (_,O,_,_,_,O,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    0xff
};

unsigned char pic_romb[] = {
    ROW (O,O,O,O,O,O,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,O,O,O,O,O,O),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,O,_,_,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,O,_,_,_,O,_),
    ROW (O,_,_,_,_,_,O),
    ROW (_,O,_,_,_,O,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,_,O,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    0xff
};

unsigned char pic_ice[] = {
    ROW (O,O,O,O,O,O,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,O,O,O,O,O,O),

    ROW (_,O,O,O,O,O,_),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (_,O,O,O,O,O,_),

    ROW (_,_,O,O,O,_,_),
    ROW (_,O,_,_,_,O,_),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (O,_,_,_,_,_,O),
    ROW (_,O,_,_,_,O,_),
    ROW (_,_,O,O,O,_,_),

    ROW (_,_,_,O,_,_,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,O,O,_,O,O,_),
    ROW (O,_,_,_,_,_,O),
    ROW (_,O,O,_,O,O,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,_,O,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,O,O,_,O,O,_),
    ROW (_,_,_,O,_,O,_),
    ROW (_,O,O,_,O,O,_),
    ROW (_,_,O,_,O,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,O,O,O,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,O,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),
    ROW (_,_,_,_,_,_,_),

    0xff
};

void show(int msec, unsigned char *sequence)
{
    unsigned char *data = sequence;

    for (;;) {
        /* Display every frame for 125 msec. */
        max_command(MAX_DIGIT0, *data++);
        max_command(MAX_DIGIT1, *data++);
        max_command(MAX_DIGIT2, *data++);
        max_command(MAX_DIGIT3, *data++);
        max_command(MAX_DIGIT4, *data++);
        max_command(MAX_DIGIT5, *data++);
        max_command(MAX_DIGIT6, *data++);
        max_command(MAX_DIGIT7, 0);

        usleep(125000);
        msec -= 125;
        if (*data == 0xff) {
            if (msec < 0)
                break;
            data = sequence;
        }
    }
}

int main()
{
    max_init();

    for (;;) {
	show(3000, pic_heart);
	show(3000, pic_bar);
	show(3000, pic_coin);
	show(3000, pic_cross);
	show(3000, pic_bird);
	show(3000, pic_asterisk);
	show(3000, pic_snow);
	show(3000, pic_fire);
	show(3000, pic_square);
	show(3000, pic_round);
	show(3000, pic_romb);
	show(3000, pic_ice);
    }
    return 0;
}
