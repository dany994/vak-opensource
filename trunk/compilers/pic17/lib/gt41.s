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
 * Compare unsigned long ">" unsigned char.
 * Leave the result in WREG.
 * Call (X>Y):
 *      xtr X1, A1
 *      xtr X2, A2
 *      xtr X3, A3
 *      xtr X0, A4
 *      xta Y
 *      call gt41
 */
#include "pic17c43.inc"

#define X1      A1
#define X2      A2
#define X3      A3
#define X0      A4

gt41:
	x?      X1
	retc    1       /* true */
	x?      X2
	retc    1       /* true */
	x?      X3
	retc    1       /* true */

	a-x     X0      /* WREG=X0-Y, C=(X0>=Y) */
	IF_LE_GOTO 1f   /* if X0<=Y goto false */
	retc    1       /* true */

1:      retc    0       /* false */
