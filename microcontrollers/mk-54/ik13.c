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
void plm_init (plm_t *t, const unsigned inst_rom[],
    const unsigned cmd_rom[], const unsigned char prog_rom[])
{
    int i;

    t->inst_rom = inst_rom;
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
    t->carry = 0;
    t->keypad_event = 0;
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
    unsigned d = t->cycle / 3;          // in range 0...13

    /*
     * Fetch program counter from the R register.
     */
    if (t->cycle == 0) {
        t->PC = t->R[36] + 16 * t->R[39];
        if ((t->cmd_rom[t->PC] & 0xfc0000) == 0)
            t->keypad_event = 0;
    }

    /*
     * Use PC to get the program index.
     */
    unsigned prog_index;
    if (t->cycle < 27)
        prog_index = t->cmd_rom[t->PC] & 0xff;
    else if (t->cycle < 36)
        prog_index = (t->cmd_rom[t->PC] >> 8) & 0xff;
    else {
        prog_index = (t->cmd_rom[t->PC] >> 16) & 0xff;
        if (prog_index > 0x1f) {
            if (t->cycle == 36) {
                t->R[37] = prog_index & 0xf;
                t->R[40] = prog_index >> 4;
            }
            prog_index = 0x5f;
        }
    }
    unsigned modifier = (t->cmd_rom[t->PC] >> 24) & 0xff;

    /*
     * Fetch the instruction opcode.
     */
    const unsigned char remap[42] = {
        0, 1, 2, 3, 4, 5, 3, 4, 5, 3, 4, 5, 3, 4,
        5, 3, 4, 5, 3, 4, 5, 3, 4, 5, 6, 7, 8, 0,
        1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5,
    };
    unsigned inst_addr = t->prog_rom[prog_index*9 + remap[t->cycle]] & 0x3f;
    if (inst_addr > 59) {
        inst_addr = (inst_addr - 60) * 2;
        if (! t->carry)
            inst_addr++;
        inst_addr += 60;
    }
    t->opcode = t->inst_rom[inst_addr];

    /*
     * Execute the opcode.
     */
    unsigned alpha = 0, beta = 0, gamma = 0;
    switch ((t->opcode >> 24) & 3) {
    case 2:
    case 3:
        if (d != (t->keyb_x - 1) && t->keyb_y > 0)
            t->S1 |= t->keyb_y;
        break;
    }
    if (t->opcode & 1)     alpha |= t->R[t->cycle];
    if (t->opcode & 2)     alpha |= t->M[t->cycle];
    if (t->opcode & 4)     alpha |= t->ST[t->cycle];
    if (t->opcode & 8)     alpha |= t->R[t->cycle] ^ 0xf;
    if (t->opcode & 0x10
        && ! t->carry)     alpha |= 0xa;
    if (t->opcode & 0x20)  alpha |= t->S;
    if (t->opcode & 0x40)  alpha |= 4;

    if (t->opcode & 0x80)  beta |= t->S;
    if (t->opcode & 0x100) beta |= t->S ^ 0xf;
    if (t->opcode & 0x200) beta |= t->S1;
    if (t->opcode & 0x400) beta |= 6;
    if (t->opcode & 0x800) beta |= 1;

    /*
     * Poll keypad.
     */
    if (t->cmd_rom[t->PC] & 0xfc0000) {
        if (t->keyb_y == 0)
            t->keypad_event = 0;
    } else {
        t->enable_display = 1;
        if (d == (t->keyb_x - 1)) {
            if (t->keyb_y > 0) {
                t->S1 = t->keyb_y;
                t->keypad_event = 1;
            }
        }
        if (t->carry && d < 12)
            t->dot = d;
        t->show_dot[d] = t->carry;
    }
    if (t->opcode & 0x1000) gamma |= t->carry;
    if (t->opcode & 0x2000) gamma |= t->carry ^ 1;
    if (t->opcode & 0x4000) gamma |= t->keypad_event ^ 1;

    /*
     * Update carry bit.
     */
    unsigned sum = alpha + beta + gamma;
    unsigned sigma = sum & 0xf;
    if ((t->opcode >> 21) & 1)
        t->carry = (sum >> 4) & 1;

    if (modifier == 0 || t->cycle >= 36) {
        /*
         * Update R register.
         */
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

    /*
     * Update M register.
     */
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

    /*
     * Update ST register.
     */
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

    /*
     * Store input, pass output.
     */
    t->output = t->M[t->cycle] & 0xf;
    t->M[t->cycle] = t->input;

    /*
     * Increase the cycle counter.
     */
    t->cycle++;
    if (t->cycle >= REG_NWORDS)
        t->cycle = 0;
}
