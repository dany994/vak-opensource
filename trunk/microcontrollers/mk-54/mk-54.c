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
#include <stdio.h>
#include <unistd.h>

#define FIFO_NWORDS 252                 // Number of words in FIFO chip

typedef struct {
    unsigned input;                     // Input word
    unsigned output;                    // Output word
    unsigned ucycle;                    // Micro-cycle counter
    unsigned char data [FIFO_NWORDS];   // FIFO memory
} fifo_t;

//
// Initialize the FIFO register data structure.
//
void fifo_init (fifo_t *t)
{
    int i;

    for (i=0; i<FIFO_NWORDS; i++)
        t->data[i] = 0;
    t->input = 0;
    t->output = 0;
    t->ucycle = 0;
}

//
// Simulate one cycle of the FIFO chip.
//
void fifo_step (fifo_t *t)
{
    if (t->ucycle >= FIFO_NWORDS)
        t->ucycle = 0;
    t->output = t->data[t->ucycle];
    t->data[t->ucycle] = t->input;
    t->ucycle++;
}

#define REG_NWORDS  42                  // Number of words in data register

typedef struct {
    unsigned input;                     // Input word
    unsigned output;                    // Output word
    unsigned ucycle;                    // Micro-cycle counter
    unsigned char R [REG_NWORDS];       // R register
    unsigned char M [REG_NWORDS];       // M register
    unsigned char ST [REG_NWORDS];      // ST register
    unsigned S;
    unsigned S1;
    unsigned L;
    unsigned T;
    unsigned carry;
    unsigned opcode;
    unsigned keyb_x;
    unsigned keyb_y;
    unsigned dot;
    unsigned AMK;
    unsigned ASP;
    unsigned PC;
    unsigned MOD;
    unsigned enable_display;
    unsigned char show_dot [14];
    const unsigned *ucmd_rom;           // Micro-instructions
    const unsigned *cmd_rom;            // Instructions
    const unsigned char *prog_rom;      // Program
} plm_t;

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
    t->carry = 0;
    t->opcode = 0;
    t->keyb_x = 0;
    t->keyb_y = 0;
    t->dot = 0;
    t->AMK = 0;
    t->ASP = 0;
    t->PC = 0;
    t->MOD = 0;
    t->enable_display = 0;
    for (i=0; i<14; i++) {
        t->show_dot[i] = 0;
    }
}

const unsigned char remap[] = {
    0, 1, 2, 3, 4, 5,
    3, 4, 5, 3, 4, 5,
    3, 4, 5, 3, 4, 5,
    3, 4, 5, 3, 4, 5,
    6, 7, 8, 0, 1, 2,
    3, 4, 5, 6, 7, 8,
    0, 1, 2, 3, 4, 5,
};

//
// Simulate one cycle of the PLM chip.
//
void plm_step (plm_t *t)
{
    unsigned phase = t->ucycle & 3;
    unsigned signal_I = t->ucycle >> 2;
    unsigned signal_D = t->ucycle / 12;

    if (t->ucycle == 0) {
        t->PC = t->R[36] + 16 * t->R[39];
        if ((t->cmd_rom[t->PC] & 0xfc0000) == 0)
            t->T = 0;
    }
    if (phase == 0) {
        unsigned k = t->ucycle / 36;
        if (k < 3)
            t->ASP = 0xff & t->cmd_rom[t->PC];
        else if (k == 3)
            t->ASP = 0xff & t->cmd_rom[t->PC] >> 8;
        else if (k == 4) {
            t->ASP = 0xff & t->cmd_rom[t->PC] >> 16;
            if (t->ASP > 0x1f) {
                if (t->ucycle == 144) {
                    t->R[37] = t->ASP & 0xf;
                    t->R[40] = t->ASP >> 4;
                }
                t->ASP = 0x5f;
            }
        }
        t->MOD = 0xff & t->cmd_rom[t->PC] >> 24;
        t->AMK = t->prog_rom[t->ASP * 9 + remap[t->ucycle >> 2]];
        t->AMK = t->AMK & 0x3f;
        if (t->AMK > 59) {
            t->AMK = (t->AMK - 60) * 2;
            if (t->L == 0)
                t->AMK++;
            t->AMK += 60;
        }
        t->opcode = t->ucmd_rom[t->AMK];
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
    t->carry = (sum >> 4) & 1;
    if (t->MOD == 0 || (t->ucycle >> 2) >= 36) {
        switch (t->opcode >> 15 & 7) {
        case 1: t->R[signal_I] = t->R[(signal_I + 3) % REG_NWORDS]; break;
        case 2: t->R[signal_I] = sigma; break;
        case 3: t->R[signal_I] = t->S; break;
        case 4: t->R[signal_I] = t->R[signal_I] | t->S | sigma; break;
        case 5: t->R[signal_I] = t->S | sigma; break;
        case 6: t->R[signal_I] = t->R[signal_I] | t->S; break;
        case 7: t->R[signal_I] = t->R[signal_I] | sigma; break;
        }
        if (t->opcode >> 18 & 1)
            t->R[(signal_I + 41) % REG_NWORDS] = sigma;
        if (t->opcode >> 19 & 1)
            t->R[(signal_I + 40) % REG_NWORDS] = sigma;
    }
    if (t->opcode >> 21 & 1)
        t->L = t->carry;
    if (t->opcode >> 20 & 1)
        t->M[signal_I] = t->S;

    switch (t->opcode >> 22 & 3) {
    case 1: t->S = t->S1; break;
    case 2: t->S = sigma; break;
    case 3: t->S = t->S1 | sigma; break;
    }
    switch (t->opcode >> 24 & 3) {
    case 1: t->S1 = sigma; break;
    case 2: t->S1 = t->S1; break;
    case 3: t->S1 = t->S1 | sigma; break;
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

//
// MK-54 calculator consists of two PLM chips ИК1301 and ИК1303,
// and two serial FIFOs.
//
plm_t ik1302, ik1303;
fifo_t fifo1, fifo2;

#include "ik1302.c"
#include "ik1303.c"

//
// Initialize the calculator.
//
void calc_init()
{
    plm_init (&ik1302, ik1302_ucmd_rom, ik1302_cmd_rom, ik1302_prog_rom);
    plm_init (&ik1303, ik1303_ucmd_rom, ik1303_cmd_rom, ik1303_prog_rom);
    fifo_init (&fifo1);
    fifo_init (&fifo2);
}

//
// Symbols on display.
//
unsigned char display [12];
unsigned char display_old [12];         // Previous state
unsigned char show_dot [12];            // Show a decimal dot

unsigned step_num = 0;

//
// Simulate one cycle of the calculator.
// Return 0 when stopped, or 1 when running a user program.
//
int calc_step (int x, int y, unsigned rgd)
{
    int k, i, refresh;

    step_num++;
    ik1302.keyb_x = x;
    ik1302.keyb_y = y;
    ik1303.keyb_x = rgd;
    ik1303.keyb_y = 1;
    for (k=1; k<=560; k++) {
        for (i=0; i<REG_NWORDS; i++) {
            ik1302.input = fifo2.output;
            plm_step (&ik1302);
            ik1303.input = ik1302.output;
            plm_step (&ik1303);
            fifo1.input = ik1303.output;
            fifo_step (&fifo1);
            fifo2.input = fifo1.output;
            fifo_step (&fifo2);
            ik1302.M[((ik1302.ucycle >> 2) + 41) % REG_NWORDS] = fifo2.output;
        }
        if (ik1302.enable_display) {
            for (i=0; i<=8; i++)
                display[i] = ik1302.R[(8 - i) * 3];
            for (i=0; i<=2; i++)
                display[i + 9] = ik1302.R[(11 - i) * 3];
            for (i=0; i<=8; i++)
                show_dot[i] = ik1302.show_dot[9 - i];
            for (i=0; i<=2; i++)
                show_dot[i + 9] = ik1302.show_dot[12 - i];
            ik1302.enable_display = 0;
        }
    }
    refresh = 0;
    for (k=0; k<=12; k++) {
        if (display_old[k] != display[k])
            refresh = 1;
        display_old[k] = display[k];
    }
    if (refresh) {
        printf ("(%u) DISPLAY = '", step_num);
        for (k=0; k<12; k++) {
            //print(k, display[k], show_dot[k]);
            putchar ("0123456789-LCrE " [display[k]]);
            if (show_dot[k])
                putchar ('.');
        }
        printf ("'\n");
    }
    return (ik1302.dot == 11);
}

//  Key Function    X,Y
// ---------------------
//  0   10^x        2,1
//  1   e^x         3,1
//  2   lg          4,1
//  3   ln          5,1
//  4   sin-1       6,1
//  5   cos-1       7,1
//  6   tg-1        8,1
//  7   sin         9,1
//  8   cos        10,1
//  9   tg         11,1
//  +   pi          2,8
//  -   sqrt        3,8
//  *   x^2         4,8
//  /   1/x         5,8
//  xy  x^y         6,8
//  ,   O           7,8
//  /-/ АВТ         8,8
//  ВП  ПРГ         9,8
//  Cx  CF         10,8
//  B^  Bx         11,8
//  C/П x!=0        2,9
//  БП  L2          3,9
//  B/O x>=0        4,9
//  ПП  L3          5,9
//  П   L1          6,9
//  ШГ> x<0         7,9
//  ИП  L0          8,9
//  <ШГ x=0         9,9
//  K              10,9
//  F              11,9

int main()
{
    // 10 - radians, 11 - grads, 12 - degrees
    unsigned rad_grad_deg = 10;

    printf ("Started MK-54.\n");
    calc_init();

    usleep (100000); calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (5, 1, rad_grad_deg);    // 3
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (6, 1, rad_grad_deg);    // 4
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (11,8, rad_grad_deg);    // B^
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (9, 1, rad_grad_deg);    // 7
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (5, 8, rad_grad_deg);    // /
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (3, 1, rad_grad_deg);    // 1
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (9, 8, rad_grad_deg);    // ВП
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (7, 1, rad_grad_deg);    // 5
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (2, 1, rad_grad_deg);    // 0
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (11,9, rad_grad_deg);    // F
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (4, 8, rad_grad_deg);    // x^2
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (11,9, rad_grad_deg);    // F
                     calc_step (0, 0, rad_grad_deg);
    usleep (100000); calc_step (4, 8, rad_grad_deg);    // x^2
#if 0
    for (;;) {
        usleep (100000); calc_step (0, 0, rad_grad_deg);
    }
#endif
}
