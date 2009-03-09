/*
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
 * Compare unsigned long "<" unsigned char.
 * Leave the result in WREG.
 * Call (X<Y):
 *      xtr X1, A1
 *      xtr X2, A2
 *      xtr X3, A3
 *      xtr X0, A4
 *      xta Y
 *      call lt41
 */
#include "pic17c43.inc"

#define X1      A1
#define X2      A2
#define X3      A3
#define X0      A4

lt41:
	x?      X1
1:      retc    0       /* false */
	x?      X2
	retc    0       /* false */
	x?      X3
	retc    0       /* false */

	a-x     X0      /* WREG=X0-Y, C=(X0>=Y) */
	IF_GE           /* if X0>=Y... */
	goto    1b      /* ...goto false */
	retc    1       /* true */
