/*
 * SPI ports.
 * Copyright (c) 2005,2006 Christophe Fillot (cf@utc.fr)
 * Copyright (C) yajin 2008 <yajinzhou@gmail.com>
 * Copyright (C) 2014 Serge Vakulenko <serge@vak.ru>
 *
 * "Interactive" part idea by Mtve.
 * TCP console added by Mtve.
 * Serial console by Peter Ross (suxen_drol@hotmail.com)
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#include <stdlib.h>
#include "globals.h"
#include "pic32mx.h"

#define NUM_SPI         4               // number of SPI ports
#define SPI_IRQ_FAULT   0               // error irq offset
#define SPI_IRQ_TX      1               // transmitter irq offset
#define SPI_IRQ_RX      2               // receiver irq offset

static unsigned spi_irq[NUM_SPI] = {    // SPI interrupt numbers
    PIC32_IRQ_SPI1E,
    PIC32_IRQ_SPI2E,
    PIC32_IRQ_SPI3E,
    PIC32_IRQ_SPI4E,
};
static unsigned spi_buf[NUM_SPI][4];    // SPI transmit and receive buffer
static unsigned spi_rfifo[NUM_SPI];     // SPI read fifo counter
static unsigned spi_wfifo[NUM_SPI];     // SPI write fifo counter
static unsigned spi_con[NUM_SPI] =      // SPIxCON address
    { SPI1CON, SPI2CON, SPI3CON, SPI4CON };
static unsigned spi_stat[NUM_SPI] =     // SPIxSTAT address
    { SPI1STAT, SPI2STAT, SPI3STAT, SPI4STAT };

unsigned sdcard_spi_port;               // SPI port number of SD card

unsigned spi_readbuf (int port)
{
    unsigned result = spi_buf[port][spi_rfifo[port]];
    
    if (VALUE(spi_con[port]) & PIC32_SPICON_ENHBUF) {
	spi_rfifo[port]++;
	spi_rfifo[port] &= 3;
    }
    if (VALUE(spi_stat[port]) & PIC32_SPISTAT_SPIRBF) {
	VALUE(spi_stat[port]) &= ~PIC32_SPISTAT_SPIRBF;
	//clear_irq (spi_irq[port] + SPI_IRQ_RX);  
    }
    return result;
}

void spi_writebuf (int port, unsigned val)
{
    /* Perform SD card i/o on configured SPI port. */
    if (port == sdcard_spi_port) {
        unsigned result = 0;

        if (VALUE(spi_con[port]) & PIC32_SPICON_MODE32) {
            /* 32-bit data width */
            result  = (unsigned char) sdcard_io (val >> 24) << 24;
            result |= (unsigned char) sdcard_io (val >> 16) << 16;
            result |= (unsigned char) sdcard_io (val >> 8) << 8;
            result |= (unsigned char) sdcard_io (val);

        } else if (VALUE(spi_con[port]) & PIC32_SPICON_MODE16) {
            /* 16-bit data width */
            result = (unsigned char) sdcard_io (val >> 8) << 8;
            result |= (unsigned char) sdcard_io (val);

        } else {
            /* 8-bit data width */
            result = (unsigned char) sdcard_io (val);
        }
        spi_buf[port][spi_wfifo[port]] = result;
    } else {
        /* No device */
        spi_buf[port][spi_wfifo[port]] = ~0;
    }
    if (VALUE(spi_stat[port]) & PIC32_SPISTAT_SPIRBF) {
        VALUE(spi_stat[port]) |= PIC32_SPISTAT_SPIROV;
        //set_irq (spi_irq[port] + SPI_IRQ_FAULT);
    } else if (VALUE(spi_con[port]) & PIC32_SPICON_ENHBUF) {
        spi_wfifo[port]++;
        spi_wfifo[port] &= 3;
        if (spi_wfifo[port] == spi_rfifo[port]) {
            VALUE(spi_stat[port]) |= PIC32_SPISTAT_SPIRBF;
            //set_irq (spi_irq[port] + SPI_IRQ_RX);
        }
    } else {
        VALUE(spi_stat[port]) |= PIC32_SPISTAT_SPIRBF;
        //set_irq (spi_irq[port] + SPI_IRQ_RX);
    }
}

void spi_control (int port)
{
    if (! (VALUE(spi_con[port]) & PIC32_SPICON_ON)) {
	clear_irq (spi_irq[port] + SPI_IRQ_FAULT);
	clear_irq (spi_irq[port] + SPI_IRQ_RX);
	clear_irq (spi_irq[port] + SPI_IRQ_TX);
	VALUE(spi_stat[port]) = PIC32_SPISTAT_SPITBE;
    } else if (! (VALUE(spi_con[port]) & PIC32_SPICON_ENHBUF)) {
	spi_rfifo[port] = 0;
	spi_wfifo[port] = 0;
    }
}

void spi_reset()
{
    VALUE(SPI1CON)  = 0;
    VALUE(SPI1STAT) = PIC32_SPISTAT_SPITBE;     // Transmit buffer is empty
    spi_wfifo[0]    = 0;
    spi_rfifo[0]    = 0;
    VALUE(SPI1BRG)  = 0;
    VALUE(SPI2CON)  = 0;
    VALUE(SPI2STAT) = PIC32_SPISTAT_SPITBE;     // Transmit buffer is empty
    spi_wfifo[1]    = 0;
    spi_rfifo[1]    = 0;
    VALUE(SPI2BRG)  = 0;
    VALUE(SPI3CON)  = 0;
    VALUE(SPI3STAT) = PIC32_SPISTAT_SPITBE;     // Transmit buffer is empty
    spi_wfifo[2]    = 0;
    spi_rfifo[2]    = 0;
    VALUE(SPI3BRG)  = 0;
    VALUE(SPI4CON)  = 0;
    VALUE(SPI4STAT) = PIC32_SPISTAT_SPITBE;     // Transmit buffer is empty
    spi_wfifo[3]    = 0;
    spi_rfifo[3]    = 0;
    VALUE(SPI4BRG)  = 0;
}

