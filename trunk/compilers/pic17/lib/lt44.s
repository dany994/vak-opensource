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
 * Compare "<" unsigned long.
 * Leave the result in WREG.
 * Call (X<Y):
 *      xtr X1, A1
 *      xtr X2, A2
 *      xtr X3, A3
 *      xtr X0, A4
 *      xtr Y0, A5
 *      xtr Y1, A6
 *      xtr Y2, A7
 *      xta Y3
 *      call lt44
 */
#include "pic17c43.inc"

#define X1 A1
#define X2 A2
#define X3 A3
#define X0 A4
#define Y0 A5
#define Y1 A6
#define Y2 A7

lt44:
	a-x     X3      /* WREG=X3-Y3, C=(X3>=Y3) */
	IF_LT           /* if X3<Y3... */
	retc    1       /* ...return true */
	nz?
	retc    0       /* if X3>Y3 return false */

	xta     Y2
	a-x     X2      /* WREG=X2-Y2, C=(X2>=Y2) */
	IF_LT           /* if X2<Y2... */
	retc    1       /* ...return true */
	nz?
	retc    0       /* if X2>Y2 return false */

	xta     Y1
	a-x     X1      /* WREG=X1-Y1, C=(X1>=Y1) */
	IF_LT           /* if X1<Y1... */
	retc    1       /* ...return true */
	nz?
	retc    0       /* if X1>Y1 return false */

	xta     Y0
	a-x     X0      /* WREG=X0-Y0, C=(X0>=Y0) */
	IF_GE           /* if X0>=Y0... */
	retc    0       /* ...return false */

	retc    1       /* true */
