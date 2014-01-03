/*
 * К145ИК130x chip.
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
// Initialize the PLM data structure.
//
void plm_init (plm_t *t, const unsigned ucmd_rom[],
    const unsigned cmd_rom[], const unsigned char prog_rom[])
{
    int i;

    t->ucmd_rom = ucmd_rom;
    t->cmd_rom = cmd_rom;
    t->prog_rom = prog_rom;

    for (i=0; i<REG_NWORDS; i++) {
        t->R[i] = 0;
        t->M[i] = 0;
        t->ST[i] = 0;
    }
    t->input = 0;
    t->output = 0;
    t->ucycle = 0;
    t->S = 0;
    t->S1 = 0;
    t->L = 0;
    t->T = 0;
    t->opcode = 0;
    t->keyb_x = 0;
    t->keyb_y = 0;
    t->dot = 0;
    t->PC = 0;
    t->MOD = 0;
    t->enable_display = 0;
    for (i=0; i<14; i++) {
        t->show_dot[i] = 0;
    }
}

//
// Simulate one cycle of the PLM chip.
//
void plm_step (plm_t *t)
{
    const unsigned char remap[] = {
        0, 1, 2, 3, 4, 5,
        3, 4, 5, 3, 4, 5,
        3, 4, 5, 3, 4, 5,
        3, 4, 5, 3, 4, 5,
        6, 7, 8, 0, 1, 2,
        3, 4, 5, 6, 7, 8,
        0, 1, 2, 3, 4, 5,
    };
    unsigned phase = t->ucycle & 3;
    unsigned signal_I = t->ucycle >> 2;
    unsigned signal_D = t->ucycle / 12;

    if (t->ucycle == 0) {
        t->PC = t->R[36] + 16 * t->R[39];
        if ((t->cmd_rom[t->PC] & 0xfc0000) == 0)
            t->T = 0;
    }
    if (phase == 0) {
        unsigned ASP;
        unsigned k = t->ucycle / 36;

        if (k < 3)
            ASP = 0xff & t->cmd_rom[t->PC];
        else if (k == 3)
            ASP = 0xff & t->cmd_rom[t->PC] >> 8;
        else {
            ASP = 0xff & t->cmd_rom[t->PC] >> 16;
            if (ASP > 0x1f) {
                if (t->ucycle == 144) {
                    t->R[37] = ASP & 0xf;
                    t->R[40] = ASP >> 4;
                }
                ASP = 0x5f;
            }
        }
        t->MOD = 0xff & t->cmd_rom[t->PC] >> 24;
        unsigned AMK = t->prog_rom[ASP * 9 + remap[t->ucycle >> 2]];
        AMK &= 0x3f;
        if (AMK > 59) {
            AMK = (AMK - 60) * 2;
            if (t->L == 0)
                AMK++;
            AMK += 60;
        }
        t->opcode = t->ucmd_rom[AMK];
    }

    unsigned alpha = 0, beta = 0, gamma = 0;
    switch (t->opcode >> 24 & 3) {
    case 2:
    case 3:
        if ((t->ucycle / 12) != (t->keyb_x - 1) &&
            t->keyb_y > 0 && phase == 0)
            t->S1 |= t->keyb_y;
        break;
    }
    if (t->opcode & 1)     alpha |= t->R[signal_I];
    if (t->opcode & 2)     alpha |= t->M[signal_I];
    if (t->opcode & 4)     alpha |= t->ST[signal_I];
    if (t->opcode & 8)     alpha |= ~t->R[signal_I] & 0xf;
    if (t->opcode & 0x10 && t->L == 0) alpha |= 0xa;
    if (t->opcode & 0x20)  alpha |= t->S;
    if (t->opcode & 0x40)  alpha |= 4;
    if (t->opcode & 0x800) beta |= 1;
    if (t->opcode & 0x400) beta |= 6;
    if (t->opcode & 0x200) beta |= t->S1;
    if (t->opcode & 0x100) beta |= ~t->S & 0xf;
    if (t->opcode & 0x80)  beta |= t->S;

    if (t->cmd_rom[t->PC] & 0xfc0000) {
        if (t->keyb_y == 0)
            t->T = 0;
    } else {
        t->enable_display = 1;
        if ((t->ucycle / 12) == (t->keyb_x - 1)) {
            if (t->keyb_y > 0) {
                t->S1 = t->keyb_y;
                t->T = 1;
            }
        }
        if (phase == 0 && signal_D < 12 && t->L > 0)
            t->dot = signal_D;
        t->show_dot[signal_D] = (t->L > 0);
    }
    if (t->opcode & 0x4000) gamma = ~t->T & 1;
    if (t->opcode & 0x2000) gamma |= ~t->L & 1;
    if (t->opcode & 0x1000) gamma |= t->L & 1;

    unsigned sum = alpha + beta + gamma;
    unsigned sigma = sum & 0xf;
    unsigned carry = (sum >> 4) & 1;
    if (t->MOD == 0 || (t->ucycle >> 2) >= 36) {
        switch (t->opcode >> 15 & 7) {
        case 1: t->R[signal_I] = t->R[(signal_I + 3) % REG_NWORDS]; break;
        case 2: t->R[signal_I] = sigma;                             break;
        case 3: t->R[signal_I] = t->S;                              break;
        case 4: t->R[signal_I] = t->R[signal_I] | t->S | sigma;     break;
        case 5: t->R[signal_I] = t->S | sigma;                      break;
        case 6: t->R[signal_I] = t->R[signal_I] | t->S;             break;
        case 7: t->R[signal_I] = t->R[signal_I] | sigma;            break;
        }
        if (t->opcode >> 18 & 1)
            t->R[(signal_I - 1 + REG_NWORDS) % REG_NWORDS] = sigma;
        if (t->opcode >> 19 & 1)
            t->R[(signal_I - 2 + REG_NWORDS) % REG_NWORDS] = sigma;
    }
    if (t->opcode >> 21 & 1)
        t->L = carry;
    if (t->opcode >> 20 & 1)
        t->M[signal_I] = t->S;

    switch (t->opcode >> 22 & 3) {
    case 1: t->S = t->S1;           break;
    case 2: t->S = sigma;           break;
    case 3: t->S = t->S1 | sigma;   break;
    }
    switch (t->opcode >> 24 & 3) {
    case 1: t->S1 = sigma;          break;
    case 2: t->S1 = t->S1;          break;
    case 3: t->S1 = t->S1 | sigma;  break;
    }

    unsigned x, y, z;
    switch (t->opcode >> 26 & 3) {
    case 1:
        t->ST[(signal_I + 2) % REG_NWORDS] = t->ST[(signal_I + 1) % REG_NWORDS];
        t->ST[(signal_I + 1) % REG_NWORDS] = t->ST[signal_I];
        t->ST[signal_I] = sigma;
        break;
    case 2:
        x = t->ST[signal_I];
        t->ST[signal_I] = t->ST[(signal_I + 1) % REG_NWORDS];
        t->ST[(signal_I + 1) % REG_NWORDS] = t->ST[(signal_I + 2) % REG_NWORDS];
        t->ST[(signal_I + 2) % REG_NWORDS] = x;
        break;
    case 3:
        x = t->ST[signal_I];
        y = t->ST[(signal_I + 1) % REG_NWORDS];
        z = t->ST[(signal_I + 2) % REG_NWORDS];
        t->ST[(signal_I + 0) % REG_NWORDS] = sigma | y;
        t->ST[(signal_I + 1) % REG_NWORDS] = x | z;
        t->ST[(signal_I + 2) % REG_NWORDS] = y | x;
        break;
    }
    t->output = 0xf & t->M[signal_I];
    t->M[signal_I] = t->input;

    t->ucycle += 4;
    if (t->ucycle > 167)
        t->ucycle = 0;
}
