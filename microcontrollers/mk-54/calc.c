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
int calc_step()
{
    int k, i, digit, dot;
    unsigned cycle;

    for (k=0; k<560; k++) {
        // Scan keypad.
        i = calc_keypad();
        ik1302.keyb_x = i >> 4;
        ik1302.keyb_y = i & 0xf;
        ik1303.keyb_x = calc_rgd();
        ik1303.keyb_y = 1;

        // Do computations.
        for (cycle=0; cycle<REG_NWORDS; cycle++) {
            ik1302.input = fifo2.output;
            plm_step (&ik1302, cycle);
            ik1303.input = ik1302.output;
            plm_step (&ik1303, cycle);
            fifo1.input = ik1303.output;
            fifo_step (&fifo1);
            fifo2.input = fifo1.output;
            fifo_step (&fifo2);
            ik1302.M[cycle] = fifo2.output;
        }

        i = k % 14;
        if (i >= 12) {
            // Clear display.
            calc_display (-1, 0, 0);
        } else {
            if (i < 3) {
                // Exponent.
                digit = ik1302.R [(i + 9) * 3];
                dot = ik1302.show_dot [i + 10];
            } else {
                // Mantissa.
                digit = ik1302.R [(i - 3) * 3];
                dot = ik1302.show_dot [i - 2];
            }

            if (ik1302.dot == 11) {
                // Run mode: blink once per step with dots enabled.
                calc_display (i, k < 14 ? digit : -1, 1);

            } else if (ik1302.enable_display) {
                // Manual mode.
                calc_display (i, digit, dot);
                ik1302.enable_display = 0;
            } else {
                // Clear display.
                calc_display (i, -1, -1);
            }
        }
    }
    return (ik1302.dot == 11);
}
