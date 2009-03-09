/*
 * PIC12C5xx assembler definitions.
 *
 * Copyright (C) 1997-1998 Cronyx Engineering Ltd.
 * Author: Serge Vakulenko, <vak@cronyx.ru>
 *
 * This software is distributed with NO WARRANTIES, not even the implied
 * warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors grant any other persons or organisations permission to use
 * or modify this software as long as it is not sold for profit,
 * and this message is kept with the software, all derivative works or
 * modified versions.
 *
 * For permission to use this software in commercial purposes,
 * please, contact the author.
 */

/*
 * Registers.
 */
#define INDF    0
#define TMR0    1
#define PCL     2
#define STATUS  3
#define FSR     4
#define OSCCAL  5
#define GPIO    6

/*
 * C compiler working registers.
 */
#define A1      0x07
#define A2      0x08
#define A3      0x09

/*
 * STATUS bits.
 */
#define GPWUF   7       /* GPIO reset: 1 - due to wake up */
#define PA0     5       /* program page preselect */
#define TO      4       /* time out: 1 after power up, clrwdt or sleep */
#define PD      3       /* power down: 1 after power up or by clrwdt */
#define Z       2       /* zero result flag */
#define DC      1       /* digit carry/not borrow flag */
#define C       0       /* carry/not borrow flag */

#define IF_GE c?
#define IF_LT nc?
#define IF_LE_GOTO c?; z?; goto

A4      .data   1
A5      .data   1
A6      .data   1
A7      .data   1
A8      .data   1
A9      .data   1
A10     .data   1
A11     .data   1
A12     .data   1
A13     .data   1
