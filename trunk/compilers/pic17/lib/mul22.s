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
 * Multiply unsigned short by short.
 * Left arg is (A1,A4), right is (A6,A5).
 * Leave the result in (A3,A2,A1,WREG).
 * Leave A6,A5 untouched.
 */
#include "pic17c43.inc"

#define X0   A4
#define X1   A1
#define X2   A2
#define X3   A3
#define Y0   A5
#define Y1   A6
#define TEMP A7

mul22:
	xta     X1
	atx     TEMP            /* temp := x1 */

	a*x     Y1
	rtx     PRODH,X3        /* x3x2 := x1 * y1 */
	rtx     PRODL,X2

	xta     X0
	a*x     Y0
	rtx     PRODH,X1        /* x1x0 := x0 * y0 */
	rtx     PRODL,X0

	a*x     Y1              /* x0 * y1 */
	xta     PRODL
	x+a     X1
	xta     PRODH
	x+ca    X2
	az
	x+ca    X3

	xta     TEMP
	a*x     Y0              /* x1 * y0 */
	xta     PRODL
	x+a     X1
	xta     PRODH
	x+ca    X2
	az
	x+ca    X3

	xta     X0
	ret
