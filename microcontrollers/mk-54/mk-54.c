/*
 * Simulator of MK-54 programmable soviet calculator.
 * Based on sources of emu145 project: https://code.google.com/p/emu145/
 *
 * Copyright (C) 2013 Serge Vakulenko, <serge@vak.ru>
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
#include "calc.h"

//
// MK-54 calculator consists of two PLM chips ИК1301 and ИК1303,
// and two serial FIFOs К145ИР2.
//
static plm_t ik1302, ik1303;
static fifo_t fifo1, fifo2;

//
// Initialize the calculator.
//
void calc_init()
{
    #include "ik1302.c"
    #include "ik1303.c"

    plm_init (&ik1302, ik1302_ucmd_rom, ik1302_cmd_rom, ik1302_prog_rom);
    plm_init (&ik1303, ik1303_ucmd_rom, ik1303_cmd_rom, ik1303_prog_rom);
    fifo_init (&fifo1);
    fifo_init (&fifo2);
}

//
// Simulate one cycle of the calculator.
// Return 0 when stopped, or 1 when running a user program.
// Fill digit[] and dit[] arrays with the indicator contents.
//
int calc_step (unsigned x, unsigned y, unsigned rgd,
    unsigned char digit[], unsigned char dot[])
{
    int k, i;

    ik1302.keyb_x = x;
    ik1302.keyb_y = y;
    ik1303.keyb_x = rgd;
    ik1303.keyb_y = 1;
    for (k=0; k<560; k++) {
        for (i=0; i<REG_NWORDS; i++) {
            ik1302.input = fifo2.output;
            plm_step (&ik1302);
            ik1303.input = ik1302.output;
            plm_step (&ik1303);
            fifo1.input = ik1303.output;
            fifo_step (&fifo1);
            fifo2.input = fifo1.output;
            fifo_step (&fifo2);
            ik1302.M[((ik1302.ucycle >> 2) - 1 + REG_NWORDS) % REG_NWORDS] = fifo2.output;
        }
        if (ik1302.enable_display) {
            for (i=0; i<=8; i++)
                digit[i] = ik1302.R[(8 - i) * 3];
            for (i=0; i<=2; i++)
                digit[i + 9] = ik1302.R[(11 - i) * 3];
            for (i=0; i<=8; i++)
                dot[i] = ik1302.show_dot[9 - i];
            for (i=0; i<=2; i++)
                dot[i + 9] = ik1302.show_dot[12 - i];
            ik1302.enable_display = 0;
        }
    }
    return (ik1302.dot == 11);
}
