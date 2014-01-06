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
    t->cycle = 0;
    t->S = 0;
    t->S1 = 0;
    t->L = 0;
    t->T = 0;
    t->opcode = 0;
    t->keyb_x = 0;
    t->keyb_y = 0;
    t->dot = 0;
    t->PC = 0;
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
    unsigned signal_d = t->cycle / 3;

    if (t->cycle == 0) {
        t->PC = t->R[36] + 16 * t->R[39];
        if ((t->cmd_rom[t->PC] & 0xfc0000) == 0)
            t->T = 0;
    }

    unsigned asp;
    unsigned k = t->cycle / 9;

    if (k < 3)
        asp = t->cmd_rom[t->PC] & 0xff;
    else if (k == 3)
        asp = (t->cmd_rom[t->PC] >> 8) & 0xff;
    else {
        asp =(t->cmd_rom[t->PC] >> 16) & 0xff;
        if (asp > 0x1f) {
            if (t->cycle == 36) {
                t->R[37] = asp & 0xf;
                t->R[40] = asp >> 4;
            }
            asp = 0x5f;
        }
    }
    unsigned mod = (t->cmd_rom[t->PC] >> 24) & 0xff;

    unsigned amk = t->prog_rom[asp*9 + remap[t->cycle]] & 0x3f;
    if (amk > 59) {
        amk = (amk - 60) * 2;
        if (t->L == 0)
            amk++;
        amk += 60;
    }
    t->opcode = t->ucmd_rom[amk];

    unsigned alpha = 0, beta = 0, gamma = 0;
    switch ((t->opcode >> 24) & 3) {
    case 2:
    case 3:
        if (signal_d != (t->keyb_x - 1) && t->keyb_y > 0)
            t->S1 |= t->keyb_y;
        break;
    }
    if (t->opcode & 1)     alpha |= t->R[t->cycle];
    if (t->opcode & 2)     alpha |= t->M[t->cycle];
    if (t->opcode & 4)     alpha |= t->ST[t->cycle];
    if (t->opcode & 8)     alpha |= ~t->R[t->cycle] & 0xf;
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
        if (signal_d == (t->keyb_x - 1)) {
            if (t->keyb_y > 0) {
                t->S1 = t->keyb_y;
                t->T = 1;
            }
        }
        if (signal_d < 12 && t->L > 0)
            t->dot = signal_d;
        t->show_dot[signal_d] = (t->L > 0);
    }
    if (t->opcode & 0x4000) gamma = ~t->T & 1;
    if (t->opcode & 0x2000) gamma |= ~t->L & 1;
    if (t->opcode & 0x1000) gamma |= t->L & 1;

    unsigned sum = alpha + beta + gamma;
    unsigned sigma = sum & 0xf;
    unsigned carry = (sum >> 4) & 1;
    if (mod == 0 || t->cycle >= 36) {
        switch ((t->opcode >> 15) & 7) {
        case 1: t->R[t->cycle] = t->R[(t->cycle + 3) % REG_NWORDS]; break;
        case 2: t->R[t->cycle] = sigma;                             break;
        case 3: t->R[t->cycle] = t->S;                              break;
        case 4: t->R[t->cycle] = t->R[t->cycle] | t->S | sigma;     break;
        case 5: t->R[t->cycle] = t->S | sigma;                      break;
        case 6: t->R[t->cycle] = t->R[t->cycle] | t->S;             break;
        case 7: t->R[t->cycle] = t->R[t->cycle] | sigma;            break;
        }
        if ((t->opcode >> 18) & 1)
            t->R[(t->cycle - 1 + REG_NWORDS) % REG_NWORDS] = sigma;
        if ((t->opcode >> 19) & 1)
            t->R[(t->cycle - 2 + REG_NWORDS) % REG_NWORDS] = sigma;
    }
    if ((t->opcode >> 21) & 1)
        t->L = carry;
    if ((t->opcode >> 20) & 1)
        t->M[t->cycle] = t->S;

    switch ((t->opcode >> 22) & 3) {
    case 1: t->S = t->S1;           break;
    case 2: t->S = sigma;           break;
    case 3: t->S = t->S1 | sigma;   break;
    }
    switch ((t->opcode >> 24) & 3) {
    case 1: t->S1 = sigma;          break;
    case 2: t->S1 = t->S1;          break;
    case 3: t->S1 |= sigma;         break;
    }

    unsigned x, y, z;
    switch ((t->opcode >> 26) & 3) {
    case 1:
        t->ST[(t->cycle + 2) % REG_NWORDS] = t->ST[(t->cycle + 1) % REG_NWORDS];
        t->ST[(t->cycle + 1) % REG_NWORDS] = t->ST[t->cycle];
        t->ST[t->cycle] = sigma;
        break;
    case 2:
        x = t->ST[t->cycle];
        t->ST[t->cycle] = t->ST[(t->cycle + 1) % REG_NWORDS];
        t->ST[(t->cycle + 1) % REG_NWORDS] = t->ST[(t->cycle + 2) % REG_NWORDS];
        t->ST[(t->cycle + 2) % REG_NWORDS] = x;
        break;
    case 3:
        x = t->ST[t->cycle];
        y = t->ST[(t->cycle + 1) % REG_NWORDS];
        z = t->ST[(t->cycle + 2) % REG_NWORDS];
        t->ST[(t->cycle + 0) % REG_NWORDS] = sigma | y;
        t->ST[(t->cycle + 1) % REG_NWORDS] = x | z;
        t->ST[(t->cycle + 2) % REG_NWORDS] = y | x;
        break;
    }
    t->output = t->M[t->cycle] & 0xf;
    t->M[t->cycle] = t->input;

    t->cycle++;
    if (t->cycle >= REG_NWORDS)
        t->cycle = 0;
}
