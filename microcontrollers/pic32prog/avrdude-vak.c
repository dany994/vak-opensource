/*
 * avrdude - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2005 Erik Walthinsen
 * Copyright (C) 2002-2004 Brian S. Dean <bsd@bsdhome.com>
 * Copyright (C) 2006 David Moore
 * Copyright (C) 2006,2007 Joerg Wunsch <j@uriah.heep.sax.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* $Id: pickit2.c 2010-05-03 dbrown$ */
/* Based on Id: stk500v2.c 836 2009-07-10 22:39:37Z joerg_wunsch */

/*
 * avrdude interface for PicKit2 programmer
 *
 * The PicKit2 programmer is a cheap device capable
 * of 2 (bidirectional data line), 3, 4 wire SPI comms
 *
 * The PICkit2 software license doesn't allow the source to be
 * modified to program other devices - nor can we distribute
 * their source code. This program is not derived from nor does it
 * contain any of the pickit2 source and should be exempt from any
 * licensing issues.
 *
 * ISP Pinout (AVR - PICKit2 (pin)):
 * RST  - VPP/MCLR (1)
 * VDD  - VDD Target (2) -- possibly optional if AVR self powered
 * GND  - GND (3)
 * MISO - PGD (4)
 * SCLK - PDC (5)
 * MOSI - AUX (6)
 */

#include "ac_cfg.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <inttypes.h>

#include "avrdude.h"
#include "avr.h"
#include "pgm.h"
#include "usbdevs.h"

#include <usb.h>

#if 0
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...)
#endif

#if 0
#define DEBUGRECV(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUGRECV(...)
#endif

#define PICKIT2_VID 0x04d8
#define PICKIT2_PID 0x0033

#define SPI_MAX_CHUNK (64 - 10)    // max packet size less the command overhead

int usb_open_device(struct usb_dev_handle **dev, int vid, int pid);

//#define INVALID_HANDLE_VALUE NULL
#define USB_ERROR_NONE      0
#define USB_ERROR_ACCESS    1
#define USB_ERROR_NOTFOUND  2
#define USB_ERROR_BUSY      16
#define USB_ERROR_IO        5

int pickit2_write_report(PROGRAMMER *pgm, unsigned char report[65]);
int pickit2_read_report(PROGRAMMER *pgm, unsigned char report[65]);

#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

/*
 * Private data for this programmer.
 */
struct pdata
{
    struct usb_dev_handle *usb_handle;     // LIBUSB STUFF
    uint8_t clock_period;  // SPI clock period in us
    int transaction_timeout;    // usb trans timeout in ms
};

#define PDATA(pgm) ((struct pdata *)(pgm->cookie))

#define CMD_NOP             0x5A
#define CMD_GET_VERSION     0x76
#define CMD_SET_VDD_4(v)    0xA0, (uint8_t)((v)*2048+672), (uint8_t)(((v)*2048+672)/256), (uint8_t)((v)*36)
#define CMD_SET_VPP_4(v)    0xA1, 0x40, (uint8_t)((v)*18.61), (uint8_t)((v)*13)
#define CMD_READ_VDD_VPP    0xA3
#define CMD_EXEC_SCRIPT_2(len)  0xA6, (len)
#define CMD_CLR_DLOAD_BUFF  0xA7
#define CMD_DOWNLOAD_DATA_2(len)  0xA8, (len)
#define CMD_CLR_ULOAD_BUFF  0xA9
#define CMD_UPLOAD_DATA     0xAA
#define CMD_UPLOAD_DATA_NO_LEN     0xAC
#define CMD_END_OF_BUFFER   0xAD

#define SCR_VDD_ON          0xFF
#define SCR_VDD_OFF         0xFE
#define SCR_VPP_ON          0xFB
#define SCR_VPP_OFF         0xFA
#define SCR_VPP_PWM_ON      0xF9
#define SCR_VPP_PWM_OFF     0xF8
#define SCR_MCLR_GND_ON     0xF7
#define SCR_MCLR_GND_OFF    0xF6
#define SCR_BUSY_LED_ON     0xF5
#define SCR_BUSY_LED_OFF    0xF4
#define SCR_SET_ICSP_DELAY_2(us) 0xEA,(us)
#define SCR_SET_PINS_2(dd, cd, dv, cv) 0xF3, (((cd)!=0) | (((dd)!=0)<<1) | (((cv)!=0)<<2) | (((dv)!=0)<<3))
#define SCR_GET_PINS        0xDC
#define SCR_LOOP_3(rel, cnt)    0xE9, rel, cnt
#define SCR_DELAY_2(sec)    ((sec)>0.0054528 ? 0xE8 : 0xE7), (uint8_t)((sec)>0.0054528?(.999+(sec)/.00546):(.999+(sec)/.0000213))
#define SCR_SET_AUX_2(ad, av)   0xCF, (((ad)!=0) | (((av)!=0)<<1))
#define SCR_SPI_SETUP_PINS_4    SCR_SET_PINS_2(1,0,0,0), SCR_SET_AUX_2(0,0)
#define SCR_SPI             0xC3
#define SCR_SPI_LIT_2(v)    0xC7,(v)

static void pickit2_setup(PROGRAMMER * pgm)
{
    if ((pgm->cookie = malloc(sizeof(struct pdata))) == 0)
    {
        fprintf(stderr,
                "%s: pickit2_setup(): Out of memory allocating private data\n",
                progname);
        exit(1);
    }
    memset(pgm->cookie, 0, sizeof(struct pdata));

    PDATA(pgm)->transaction_timeout = 1500;    // default value, may be overridden with -x timeout=ms
    PDATA(pgm)->clock_period = 10;    // default value, may be overridden with -x clockrate=us or -B or -i
}

static void pickit2_teardown(PROGRAMMER * pgm)
{
    free(pgm->cookie);
}

static int pickit2_open(PROGRAMMER * pgm, char * port)
{
    if (usb_open_device(&(PDATA(pgm)->usb_handle), PICKIT2_VID, PICKIT2_PID) < 0)
    {
        /* no PICkit2 found */
        fprintf(stderr,
                "%s: error: could not find PICkit2 with vid=0x%x pid=0x%x\n",
                progname, PICKIT2_VID, PICKIT2_PID);
        exit(1);
    }

    if (pgm->ispdelay > 0)
    {
        PDATA(pgm)->clock_period = MIN(pgm->ispdelay, 255);
    }
    else if (pgm->bitclock > 0.0)
    {
        PDATA(pgm)->clock_period = MIN(pgm->bitclock * 1e6, 255);
    }

    return 0;
}

static void pickit2_close(PROGRAMMER * pgm)
{
    usb_close(PDATA(pgm)->usb_handle);
}

static int pickit2_initialize(PROGRAMMER * pgm, AVRPART * p)
{
    unsigned char temp[4];
    memset(temp, 0, sizeof(temp));

    int errorCode = 0;

    /* set sck period */
    if (pgm->set_sck_period)
        pgm->set_sck_period(pgm, pgm->bitclock);

    /* connect to target device -- we'll just ask for the firmware version */
    char report[65] = {0, CMD_GET_VERSION, CMD_END_OF_BUFFER};
    if ((errorCode = pickit2_write_report(pgm, report)) <= 0) {
        fprintf(stderr, "pickit2_write_report failed (ec %d). %s\n", errorCode, usb_strerror());
        return -1;
    }
    memset(report, 0, sizeof(report));

    if ((errorCode = pickit2_read_report(pgm, report)) < 4) {
	fprintf(stderr, "pickit2_read_report failed (ec %d). %s\n", errorCode, usb_strerror());
	return -1;
    }

    if (verbose)
    {
	fprintf(stderr, "%s: %s firmware version %d.%d.%d\n", progname, pgm->desc, (int)report[1], (int)report[2], (int)report[3]);
    }

    // set the pins, apply reset,
    // TO DO: apply vtarget (if requested though -x option)
    char report[65] =
    {
	0, CMD_SET_VDD_4(5),
	CMD_SET_VPP_4(5),
	CMD_EXEC_SCRIPT_2(24),
	SCR_SPI_SETUP_PINS_4,   // SDO, SDI, SCK
	SCR_SET_ICSP_DELAY_2(PDATA(pgm)->clock_period),    // slow down the SPI
	SCR_VDD_ON,
	SCR_MCLR_GND_OFF,       // let reset float high
	SCR_VPP_PWM_ON,
	SCR_DELAY_2(.1),
	SCR_VPP_ON,
	SCR_DELAY_2(.1),
	SCR_VPP_OFF,
	SCR_DELAY_2(.01),

	SCR_MCLR_GND_ON,        // reset low - programming mode
	SCR_DELAY_2(.1),

	SCR_BUSY_LED_ON,
	SCR_DELAY_2(.3),
	SCR_BUSY_LED_OFF,

	CMD_CLR_DLOAD_BUFF,
	CMD_CLR_ULOAD_BUFF,

	CMD_END_OF_BUFFER
    };

    if (pickit2_write_report(pgm, report) < 0)
    {
	fprintf(stderr, "pickit2_read_report failed (ec %d). %s\n", errorCode, usb_strerror());
	return -1;
    }

    if (pgm->program_enable)
        return pgm->program_enable(pgm, p);
    else
        return -1;
}

static void pickit2_disable(PROGRAMMER * pgm)
{
    /* make sure all pins are floating & all voltages are off */
    uint8_t report[65] =
    {
        0, CMD_EXEC_SCRIPT_2(8),
        SCR_SET_PINS_2(1,1,0,0),
        SCR_SET_AUX_2(1,0),
        SCR_MCLR_GND_OFF,
        SCR_VPP_OFF,
        SCR_VDD_OFF,
        SCR_BUSY_LED_OFF,
        CMD_END_OF_BUFFER
    };

    pickit2_write_report(pgm, report);

    return;
}

#define sendReport(x)
#define readReport(x) 0

int  pickit2_err_led(struct programmer_t * pgm, int value)
{
    // there is no error led, so just flash the busy led a few times
    uint8_t report[65] =
    {
        0, CMD_EXEC_SCRIPT_2(9),
        SCR_BUSY_LED_ON,
        SCR_DELAY_2(.2),
        SCR_BUSY_LED_OFF,
        SCR_DELAY_2(.2),
        SCR_LOOP_3(6, 9),
        CMD_END_OF_BUFFER
    };

    // busy stops flashing by itself, so just return
    if (!value)
    {
        return 0;
    }

    return pickit2_write_report(pgm, report) != -1;
}

int  pickit2_pgm_led (struct programmer_t * pgm, int value)
{
    // script to set busy led appropriately
    uint8_t report[65] = {0, CMD_EXEC_SCRIPT_2(1),
                        value ? SCR_BUSY_LED_ON : SCR_BUSY_LED_OFF,
                        CMD_END_OF_BUFFER
                       };

    return pickit2_write_report(pgm, report) != -1;
}

void pickit2_powerup(struct programmer_t * pgm)
{
    // turn vdd on?
}

void pickit2_powerdown(struct programmer_t * pgm)
{
    // do what?
    pgm->disable(pgm);
}

int  pickit2_program_enable(struct programmer_t * pgm, AVRPART * p)
{
    unsigned char cmd[4];
    unsigned char res[4];

    if (p->op[AVR_OP_PGM_ENABLE] == NULL)
    {
        fprintf(stderr, "program enable instruction not defined for part \"%s\"\n",
                p->desc);
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    avr_set_bits(p->op[AVR_OP_PGM_ENABLE], cmd);
    pgm->cmd(pgm, cmd, res);

    if (verbose)
    {
        int i;
        fprintf(stderr, "program_enable(): sending command. Resp = ");

        for (i = 0; i < 4; i++)
        {
            fprintf(stderr, "%x ", (int)res[i]);
        }
        fprintf(stderr, "\n");
    }

    // check for sync character
    if (res[2] != cmd[1])
        return -2;

    return 0;
}

int  pickit2_chip_erase(struct programmer_t * pgm, AVRPART * p)
{
    unsigned char cmd[4];
    unsigned char res[4];

    if (p->op[AVR_OP_CHIP_ERASE] == NULL)
    {
        fprintf(stderr, "chip erase instruction not defined for part \"%s\"\n",
                p->desc);
        return -1;
    }

    pgm->pgm_led(pgm, ON);

    memset(cmd, 0, sizeof(cmd));

    avr_set_bits(p->op[AVR_OP_CHIP_ERASE], cmd);
    pgm->cmd(pgm, cmd, res);
    usleep(p->chip_erase_delay);
    pgm->initialize(pgm, p);

    pgm->pgm_led(pgm, OFF);

    return 0;
}

int  pickit2_paged_load(struct programmer_t * pgm, AVRPART * p, AVRMEM * mem,
                        int page_size, int n_bytes)
{
    // only supporting flash & eeprom page reads
    if ((!mem->paged || page_size <= 1) || (strcmp(mem->desc, "flash") != 0 && strcmp(mem->desc, "eeprom") != 0))
    {
        return -1;
    }

    //fprintf(stderr, "paged read ps %d, mem %s\n", page_size, mem->desc);

    OPCODE *readop, *lext = mem->op[AVR_OP_LOAD_EXT_ADDR];
    uint8_t data = 0, cmd[SPI_MAX_CHUNK], res[SPI_MAX_CHUNK];
    int addr_base;

    pgm->pgm_led(pgm, ON);

    for (addr_base = 0; addr_base < n_bytes; )
    {
        if ((addr_base == 0 || (addr_base % /*ext_address_boundary*/ 65536) == 0)
                && lext != NULL)
        {
            memset(cmd, 0, sizeof(cmd));

            avr_set_bits(lext, cmd);
            avr_set_addr(lext, cmd, addr_base);
            pgm->cmd(pgm, cmd, res);
        }

        // bytes to send in the next packet -- not necessary as pickit2_spi() handles breaking up
        // the data into packets -- but we need to keep transfers frequent so that we can update the
        // status indicator bar
        uint32_t blockSize = MIN(65536 - (addr_base % 65536), MIN(n_bytes - addr_base, SPI_MAX_CHUNK / 4));

        memset(cmd, 0, sizeof(cmd));
        memset(res, 0, sizeof(res));

        uint8_t addr_off;
        for (addr_off = 0; addr_off < blockSize; addr_off++)
        {
            int addr = addr_base + addr_off, caddr = addr;

            if (mem->op[AVR_OP_READ_LO] != NULL && mem->op[AVR_OP_READ_HI] != NULL)
            {
                if (addr & 0x00000001)
                    readop = mem->op[AVR_OP_READ_HI];
                else
                    readop = mem->op[AVR_OP_READ_LO];

                caddr /= 2;
            }
            else if (mem->op[AVR_OP_READ] != NULL)
            {
                readop = mem->op[AVR_OP_READ];
            }
            else
            {
                fprintf(stderr, "no read command specified\n");
                return -1;
            }

            avr_set_bits(readop, &cmd[addr_off*4]);
            avr_set_addr(readop, &cmd[addr_off*4], caddr);
        }

        int bytes_read = pgm->spi(pgm, cmd, res, blockSize*4);

        if (bytes_read < 0)
        {
            fprintf(stderr, "Failed @ pgm->spi()\n");
            pgm->err_led(pgm, ON);
            return -1;
§        }

//        fprintf(stderr, "\npaged_load @ %X, wrote: %d, read: %d bytes\n", addr_base, blockSize*4, bytes_read);

        for (addr_off = 0; addr_off < bytes_read / 4; addr_off++)
        {
            data = 0;
            avr_get_output(readop, &res[addr_off*4], &data);
            mem->buf[addr_base + addr_off] = data;

            //fprintf(stderr, "%2X(%c)", (int)data, data<0x20?'.':data);
        }
        //fprintf(stderr, "\n");

        addr_base += blockSize;
        report_progress (addr_base, n_bytes, NULL);
    }

    pgm->pgm_led(pgm, OFF);

    return n_bytes;
}


int pickit2_commit_page(PROGRAMMER * pgm, AVRPART * p, AVRMEM * mem,
                        unsigned long addr)
{
    OPCODE * wp, * lext;

    wp = mem->op[AVR_OP_WRITEPAGE];
    if (wp == NULL)
    {
        fprintf(stderr,
                "pickit2_commit_page(): memory \"%s\" not configured for page writes\n",
                mem->desc);
        return -1;
    }

    // adjust the address if this memory is word-addressable
    if ((mem->op[AVR_OP_LOADPAGE_LO]) || (mem->op[AVR_OP_READ_LO]))
        addr /= 2;

    unsigned char cmd[8];
    memset(cmd, 0, sizeof(cmd));

    // use the "load extended address" command, if available
    lext = mem->op[AVR_OP_LOAD_EXT_ADDR];
    if (lext != NULL)
    {
        avr_set_bits(lext, cmd);
        avr_set_addr(lext, cmd, addr);
    }

    // make up the write page command in the 2nd cmd position
    avr_set_bits(wp, &cmd[4]);
    avr_set_addr(wp, &cmd[4], addr);

    if (lext != NULL)
    {
        // write the load extended address cmd && the write_page cmd
        pgm->spi(pgm, cmd, NULL, 8);
    }
    else
    {
        // write just the write_page cmd
        pgm->spi(pgm, &cmd[4], NULL, 4);
    }

    // just delay the max (we could do the delay in the PICkit2 if we wanted)
    usleep(mem->max_write_delay);

    return 0;
}

// not actually a paged write, but a bulk/batch write
int  pickit2_paged_write(struct programmer_t * pgm, AVRPART * p, AVRMEM * mem,
                         int page_size, int n_bytes)
{
    // only paged write for flash implemented
    if (strcmp(mem->desc, "flash") != 0 && strcmp(mem->desc, "eeprom") != 0)
    {
        fprintf(stderr, "Part does not support %d paged write of %s\n", page_size, mem->desc);
        return -1;
    }

    //fprintf(stderr, "page size %d mem %s supported: %d\n", page_size, mem->desc, mem->paged);
    //fprintf(stderr, "loadpagehi %x, loadpagelow %x, writepage %x\n", (int)mem->op[AVR_OP_LOADPAGE_HI], (int)mem->op[AVR_OP_LOADPAGE_LO], (int)mem->op[AVR_OP_WRITEPAGE]);

    OPCODE *writeop;
    uint8_t cmd[SPI_MAX_CHUNK], res[SPI_MAX_CHUNK];
    int addr_base;

    pgm->pgm_led(pgm, ON);

    for (addr_base = 0; addr_base < n_bytes; )
    {
        uint32_t blockSize;

        if (mem->paged)
        {
            blockSize = MIN(page_size - (addr_base % page_size), SPI_MAX_CHUNK/4);     // bytes remaining in page
        }
        else
        {
            blockSize = 1;
        }

        memset(cmd, 0, sizeof(cmd));
        memset(res, 0, sizeof(res));

        uint8_t addr_off;
        for (addr_off = 0; addr_off < blockSize; addr_off++)
        {
            int addr = addr_base + addr_off;
            int caddr = 0;

            /*
             * determine which memory opcode to use
             */
            if (mem->paged && mem->op[AVR_OP_LOADPAGE_HI] && mem->op[AVR_OP_LOADPAGE_LO])
            {
                if (addr & 0x01)
                    writeop = mem->op[AVR_OP_LOADPAGE_HI];
                else
                    writeop = mem->op[AVR_OP_LOADPAGE_LO];
                caddr = addr / 2;
            }
            else if (mem->paged && mem->op[AVR_OP_LOADPAGE_LO])
            {
                writeop = mem->op[AVR_OP_LOADPAGE_LO];
                caddr = addr;
            }
            else if (mem->op[AVR_OP_WRITE_LO])
            {
                writeop = mem->op[AVR_OP_WRITE_LO];
                caddr = addr;       // maybe this should divide by 2 & use the write_high opcode also

                fprintf(stderr, "Error AVR_OP_WRITE_LO defined only (where's the HIGH command?)\n");
                return -1;
            }
            else
            {
                writeop = mem->op[AVR_OP_WRITE];
                caddr = addr;
            }

            if (writeop == NULL)
            {
                pgm->err_led(pgm, ON);
                // not supported!
                return -1;
            }

            avr_set_bits(writeop, &cmd[addr_off*4]);
            avr_set_addr(writeop, &cmd[addr_off*4], caddr);
            avr_set_input(writeop, &cmd[addr_off*4], mem->buf[addr]);
        }

        int bytes_read = pgm->spi(pgm, cmd, res, blockSize*4);

        if (bytes_read < 0)
        {
            fprintf(stderr, "Failed @ pgm->spi()\n");
            pgm->err_led(pgm, ON);
            return -1;
        }

        addr_base += blockSize;

        // write the page - this function looks after extended address also
        if (mem->paged && (((addr_base % page_size) == 0) || (addr_base == n_bytes)))
        {
            //fprintf(stderr, "Calling pickit2_commit_page()\n");
            //avr_write_page(pgm, p, mem, addr_base-1);
            pickit2_commit_page(pgm, p, mem, addr_base-1);
        }
        else if (!mem->paged)
        {
            usleep(mem->max_write_delay);
        }

        report_progress (addr_base, n_bytes, NULL);
    }

    pgm->pgm_led(pgm, OFF);

    return n_bytes;
}


int pickit2_cmd(struct programmer_t * pgm, unsigned char cmd[4],
                unsigned char res[4])
{
    return pgm->spi(pgm, cmd, res, 4);
}

// painful bitbang bandaid -- don't need it as the real spi can go even slower (and seems to work even though the clock seems inverted...)
#if 0
int pickit2_spi_slow(struct programmer_t * pgm, unsigned char cmd[],
                     unsigned char res[], int count)
{
    int rc, i;
    /*
            fprintf(stderr, "spi(): Data to send = ");
            for (i = 0; i < count; i++)
            {
                fprintf(stderr, "%x ", (int)cmd[i]);
            }
            fprintf(stderr, "\n");
    */
    // 1 byte per packet
    while (count > 0)
    {
        int uploadSize = count > 8 ? 8 : count;
        for (i = uploadSize; i > 0; i--)
        {
            // shift a single byte (8 bytes in the upload buffer)
            uint8_t report[65] =
            {
                0, CMD_EXEC_SCRIPT_2(58),
                SCR_SET_AUX_2(0, (0x80 & *cmd)?2:0),   // set MOSI
                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0
                SCR_SET_PINS_2(1,0, 0, 1),    // clk 1
                SCR_GET_PINS,
                SCR_SET_AUX_2(0, (0x40 & *cmd)?2:0),   // set MOSI
                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0
                SCR_SET_PINS_2(1,0, 0, 1),    // clk 1
                SCR_GET_PINS,
                SCR_SET_AUX_2(0, (0x20 & *cmd)?2:0),   // set MOSI
                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0
                SCR_SET_PINS_2(1,0, 0, 1),    // clk 1
                SCR_GET_PINS,
                SCR_SET_AUX_2(0, (0x10 & *cmd)?2:0),   // set MOSI
                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0
                SCR_SET_PINS_2(1,0, 0, 1),    // clk 1
                SCR_GET_PINS,
                SCR_SET_AUX_2(0, (0x8 & *cmd)?2:0),   // set MOSI
                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0
                SCR_SET_PINS_2(1,0, 0, 1),    // clk 1
                SCR_GET_PINS,
                SCR_SET_AUX_2(0, (0x4 & *cmd)?2:0),   // set MOSI
                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0
                SCR_SET_PINS_2(1,0, 0, 1),    // clk 1
                SCR_GET_PINS,
                SCR_SET_AUX_2(0, (0x2 & *cmd)?2:0),   // set MOSI
                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0
                SCR_SET_PINS_2(1,0, 0, 1),    // clk 1
                SCR_GET_PINS,
                SCR_SET_AUX_2(0, (0x1 & *cmd)?2:0),   // set MOSI
                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0
                SCR_SET_PINS_2(1,0, 0, 1),    // clk 1
                SCR_GET_PINS,

                SCR_SET_PINS_2(1,0, 0, 0),    // clk 0

                (i==1)?CMD_UPLOAD_DATA_NO_LEN:CMD_END_OF_BUFFER,
                CMD_END_OF_BUFFER
            };

            pickit2_write_report(pgm, report);
            count--;
            cmd++;
        }

        uint8_t report[65];
        memset(report, 0, sizeof(report));
        rc = pickit2_read_report(pgm, report);
        if (rc <= 0)
        {
            fprintf(stderr, "Error reading spi buffer, rc = %d\n", rc);
            return -1;
        }

        int j = 1;
        while (uploadSize--)
        {
            int val = 0;
            for (i = 0; i < 8; i++)
            {
//                fprintf(stderr, "%x ", report[j]);
                val *= 2;
                val += (report[j++] & 2)?1:0;
            }
            *res++ = val;
        }
    }

    return -1;
}
#endif  // 0

// breaks up the cmd[] data into  packets & sends to the pickit2. Data shifted in is stored in res[].
int pickit2_spi(struct programmer_t * pgm, unsigned char cmd[],
                unsigned char res[], int n_bytes)
{
    int retval = 0, temp1 = 0, temp2 = 0, count = n_bytes;

    while (count > 0)
    {
        uint8_t i, blockSize = MIN(count, SPI_MAX_CHUNK);
        uint8_t report[65] = {0, CMD_DOWNLOAD_DATA_2(blockSize)};
        uint8_t *repptr = report + 3;

        memset(report + 3, CMD_END_OF_BUFFER, sizeof(report) - 3);

        // append some data to write to SPI
        for (i = 0; i < blockSize; i++)
        {
            *repptr++ = *cmd++;
            count--;    // 1 less byte to pack
        }

        if (blockSize == 1)
        {
            *repptr++ = 0xa6;       //CMD_EXECUTE_SCRIPT;
            *repptr++ = 1;
            *repptr++ = SCR_SPI;
        }
        else
        {
            *repptr++ = 0xa6;       //CMD_EXECUTE_SCRIPT_2;
            *repptr++ = 4;
            *repptr++ = SCR_SPI;
            *repptr++ = 0xe9;       //SCR_LOOP_3;
            *repptr++ = 1;
            *repptr++ = blockSize - 1;
        }

        // request the data read to be sent to us
        *repptr++ = CMD_UPLOAD_DATA;

        // check return values
        if ((temp1=pickit2_write_report(pgm, report)) < 0 ||
                (temp2=pickit2_read_report(pgm, report)) < 0)
        {
            return -1;
        }/*
        else
        {
            int i;
            fprintf(stderr, "in spi. wrote %d, read %d\n", temp1, temp2);

            for (i = 0; i < temp2; i++)
            {
                  fprintf(stderr, "%2.2x ", report[i]);
            }

            fprintf(stderr, "\n");
        }*/

        retval = report[1]; // upload-length field
        repptr = &report[2];    // actual data starts here

        if (res)                // copy data if user has specified a storage location
        {
            memcpy(res, repptr, retval);
            res += retval;
        }
    }

    return n_bytes;
}

/* taken (modified) from avrdude usbasp.c */
int usb_open_device(struct usb_dev_handle **device, int vendor, int product)
{
    struct usb_bus      *bus;
    struct usb_device   *dev;
    usb_dev_handle      *handle = NULL;
    int                 errorCode = USB_ERROR_NOTFOUND;
    static int          didUsbInit = 0;

    if (!didUsbInit)
    {
        didUsbInit = 1;
        usb_init();
    }
    usb_find_busses();
    usb_find_devices();
    for (bus=usb_get_busses(); bus; bus=bus->next)
    {
        for (dev=bus->devices; dev; dev=dev->next)
        {
            //fprintf(stderr, "Enumerating device list.. VID: 0x%4.4x, PID: 0x%4.4x\n", dev->descriptor.idVendor, dev->descriptor.idProduct);
            if (dev->descriptor.idVendor == vendor && dev->descriptor.idProduct == product)
            {
                /* we need to open the device in order to query strings */
                handle = usb_open(dev);
                if (handle == NULL)
                {
                    errorCode = USB_ERROR_ACCESS;
                    fprintf(stderr, "%s: Warning: cannot open USB device: %s\n", progname, usb_strerror());
                    continue;
                }

                // return with opened device handle
                else
                {
                    if (verbose)
                    {
                        fprintf(stderr, "Device 0x%4.4X seemed to open OK.\n", (int)handle);
                    }

                    if ((errorCode = usb_set_configuration(handle, 1)) < 0)
                    {
                        fprintf(stderr, "Could not set configuration. Error code %d, %s.\n"
                                "You may need to run avrdude as root or set up correct usb port permissions.", errorCode, usb_strerror());
                    }

                    if ((errorCode = usb_claim_interface(handle, 0)) < 0)
                    {
                        fprintf(stderr, "Could not claim interface. Error code %d, %s\n"
                                "You may need to run avrdude as root or set up correct usb port permissions.", errorCode, usb_strerror());
                    }

                    errorCode = 0;
                    *device = handle;
                    return 0;
                }
            }
        }
    }

    return -1;
}

int pickit2_write_report(PROGRAMMER * pgm, unsigned char report[65])
{
    // endpoint 1 OUT??
    return usb_interrupt_write(PDATA(pgm)->usb_handle, USB_ENDPOINT_OUT | 1, report+1, 64, PDATA(pgm)->transaction_timeout);
}

int pickit2_read_report(PROGRAMMER * pgm, unsigned char report[65])
{
    // endpoint 1 IN??
    return usb_interrupt_read(PDATA(pgm)->usb_handle, USB_ENDPOINT_IN | 1, report+1, 64, PDATA(pgm)->transaction_timeout);
}

int  pickit2_parseextparams(struct programmer_t * pgm, LISTID extparms)
{
    LNODEID ln;
    const char *extended_param;
    int rv = 0;

    for (ln = lfirst(extparms); ln; ln = lnext(ln))
    {
        extended_param = ldata(ln);

        if (strncmp(extended_param, "clockrate=", strlen("clockrate=")) == 0)
        {
            int clock_rate;
            if (sscanf(extended_param, "clockrate=%i", &clock_rate) != 1 || clock_rate <= 0)
            {
                fprintf(stderr,
                        "%s: pickit2_parseextparms(): invalid clockrate '%s'\n",
                        progname, extended_param);
                rv = -1;
                continue;
            }

            int clock_period = MIN(1000000 / clock_rate, 255);    // max period is 255
            clock_rate = (int)(1000000 / (clock_period + 5e-7));    // assume highest speed is 2MHz - should probably check this

            if (verbose >= 2)
            {
                fprintf(stderr,
                        "%s: pickit2_parseextparms(): clockrate set to 0x%02x\n",
                        progname, clock_rate);
            }
            PDATA(pgm)->clock_period = clock_period;

            continue;
        }

        if (strncmp(extended_param, "timeout=", strlen("timeout=")) == 0)
        {
            int timeout;
            if (sscanf(extended_param, "timeout=%i", &timeout) != 1 || timeout <= 0)
            {
                fprintf(stderr,
                        "%s: pickit2_parseextparms(): invalid timeout '%s'\n",
                        progname, extended_param);
                rv = -1;
                continue;
            }

            if (verbose >= 2)
            {
                fprintf(stderr,
                        "%s: pickit2_parseextparms(): usb timeout set to 0x%02x\n",
                        progname, timeout);
            }
            PDATA(pgm)->transaction_timeout = timeout;

            continue;
        }

        fprintf(stderr,
                "%s: avr910_parseextparms(): invalid extended parameter '%s'\n",
                progname, extended_param);
        rv = -1;
    }

    return rv;
}


void pickit2_initpgm (PROGRAMMER * pgm)
{
    /*
     * mandatory functions - these are called without checking to see
     * whether they are assigned or not
     */

    pgm->initialize     = pickit2_initialize;
    pgm->disable        = pickit2_disable;
    pgm->powerup        = pickit2_powerup;
    pgm->powerdown      = pickit2_powerdown;
    pgm->program_enable = pickit2_program_enable;
    pgm->chip_erase     = pickit2_chip_erase;
    pgm->open           = pickit2_open;
    pgm->close          = pickit2_close;

    pgm->read_byte      = avr_read_byte_default;
    pgm->write_byte     = avr_write_byte_default;

    /*
     * predefined functions - these functions have a valid default
     * implementation. Hence, they don't need to be defined in
     * the programmer.
     */
//    pgm->err_led        = pickit2_err_led;
    pgm->pgm_led        = pickit2_pgm_led;

    /*
     * optional functions - these are checked to make sure they are
     * assigned before they are called
     */

    pgm->cmd            = pickit2_cmd;
    pgm->spi            = pickit2_spi;
    pgm->paged_write    = pickit2_paged_write;
    pgm->paged_load     = pickit2_paged_load;
    /*      pgm->write_setup    = NULL;
          pgm->read_sig_bytes = NULL;
          pgm->set_vtarget    = NULL;//pickit2_vtarget;
          pgm->set_varef      = NULL;
          pgm->set_fosc       = NULL;
          pgm->perform_osccal = NULL;*/

    pgm->parseextparams = pickit2_parseextparams;

    pgm->setup          = pickit2_setup;
    pgm->teardown       = pickit2_teardown;
    //pgm->page_size      = 256;        // not sure what this does... maybe the max page size that the page read/write function can handle

    strncpy(pgm->type, "pickit2", sizeof(pgm->type));
}
