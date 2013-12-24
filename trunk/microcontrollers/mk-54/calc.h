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
int calc_step (unsigned keycode, unsigned rgd,
    unsigned char digit[], unsigned char dot[],
    void (*callback) (int progress));

                            //  Key Function
#define KEY_0       0x21    //  0   10^x
#define KEY_1       0x31    //  1   e^x
#define KEY_2       0x41    //  2   lg
#define KEY_3       0x51    //  3   ln
#define KEY_4       0x61    //  4   sin-1
#define KEY_5       0x71    //  5   cos-1
#define KEY_6       0x81    //  6   tg-1
#define KEY_7       0x91    //  7   sin
#define KEY_8       0xa1    //  8   cos
#define KEY_9       0xb1    //  9   tg
#define KEY_ADD     0x28    //  +   pi
#define KEY_SUB     0x38    //  -   sqrt
#define KEY_MUL     0x48    //  *   x^2
#define KEY_DIV     0x58    //  /   1/x
#define KEY_XY      0x68    //  xy  x^y
#define KEY_DOT     0x78    //  ,   O
#define KEY_NEG     0x88    //  /-/ АВТ
#define KEY_EXP     0x98    //  ВП  ПРГ
#define KEY_CLEAR   0xa8    //  Cx  CF
#define KEY_ENTER   0xb8    //  B^  Bx
#define KEY_STOPGO  0x29    //  C/П x!=0
#define KEY_GOTO    0x39    //  БП  L2
#define KEY_RET     0x49    //  B/O x>=0
#define KEY_PP      0x59    //  ПП  L3
#define KEY_P       0x69    //  П   L1
#define KEY_NEXT    0x79    //  ШГ> x<0
#define KEY_IP      0x89    //  ИП  L0
#define KEY_PREV    0x99    //  <ШГ x=0
#define KEY_K       0xa9    //  K
#define KEY_F       0xb9    //  F

#define MODE_RADIANS    10
#define MODE_GRADS      11
#define MODE_DEGREES    12
