/*
 * PIC12C5xx registers.
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
 * Unbanked registers.
 */
char  INDF    @ 0;
char  TMR0    @ 1;
char  PCL     @ 2;
char  STATUS  @ 3;
char  FSR     @ 4;
char  OSCCAL  @ 5;
char  GPIO    @ 6;

/*
 * C compiler working registers.
 */
char  A1      @ 7;
char  A2      @ 8;
char  A3      @ 9;

/*
 * STATUS bits.
 */
#define GPWUF   STATUS.7       /* GPIO reset: 1 - due to wake up */
#define PA0     STATUS.5       /* program page preselect */
#define TO      STATUS.4       /* time out: 1 after power up, clrwdt or sleep */
#define PD      STATUS.3       /* power down: 1 after power up or by clrwdt */
#define Z       STATUS.2       /* zero result flag */
#define DC      STATUS.1       /* digit carry/not borrow flag */
#define C       STATUS.0       /* carry/not borrow flag */

#define awake()  asm (" awake")
