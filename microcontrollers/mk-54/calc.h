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

//
// FIFO serial memory chip К145ИР2.
//
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
void fifo_init (fifo_t *t);

//
// Simulate one cycle of the FIFO chip.
//
void fifo_step (fifo_t *t);

//
// Specialized PLM chips К145ИК130x.
//
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
    const unsigned cmd_rom[], const unsigned char prog_rom[]);

//
// Simulate one cycle of the PLM chip.
//
void plm_step (plm_t *t);

//
// Initialize the calculator.
//
void calc_init();

//
// Simulate one cycle of the calculator.
// Return 0 when stopped, or 1 when running a user program.
// Fill digit[] and dit[] arrays with the indicator contents.
// See table below for keycodes x and y.
// Values for rgd are: 10 - radians, 11 - grads, 12 - degrees.
//
int calc_step (unsigned x, unsigned y, unsigned rgd,
    unsigned char digit[], unsigned char dot[],
    void (*callback) (int progress));

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
