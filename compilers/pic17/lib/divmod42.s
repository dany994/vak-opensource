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
 * Unsigned long-by-short divide/remainder.
 * Divide (A3,A2,A1,A4) by (A6,A5).
 * Leave the quotient in (A3,A2,A1,A4),
 * the remainder in (A8,A7).
 * A9 is used as a temporary variable.
 */
#include "pic17c43.inc"

#define X0     A4
#define X1     A1
#define X2     A2
#define X3     A3
#define Y0     A5
#define Y1     A6
#define REM0   A7
#define REM1   A8

#define TEMP   A9

#define DIV(XN) \
	xc<<a   XN; \
	xc<<x   REM0; \
	xc<<x   REM1; \
	xc<<x   TEMP; \
	xta     Y0; \
	bz?     XN,0; \
	goto    1f; \
	x-a     REM0; \
	xta     Y1; \
	x-ba    REM1; \
	az; \
	x-ba    TEMP; \
	goto    2f; \
1:      x+a     REM0; \
	xta     Y1; \
	x+ca    REM1; \
	az; \
	x+ca    TEMP; \
2:      xc<<x   XN

#define DIV2(XN,XK) \
	xc<<a   XK; \
	xc<<x   REM0; \
	xc<<x   REM1; \
	xc<<x   TEMP; \
	xta     Y0; \
	bz?     XN,0; \
	goto    3f; \
	x-a     REM0; \
	xta     Y1; \
	x-ba    REM1; \
	az; \
	x-ba    TEMP; \
	goto    4f; \
3:      x+a     REM0; \
	xta     Y1; \
	x+ca    REM1; \
	az; \
	x+ca    TEMP; \
4:      xc<<x   XK

divmod42:
	xz      REM0
	xz      REM1

	xc<<a   X3
	xc<<x   REM0
	xc<<x   REM1
	xta     Y0
	x-a     REM0
	xta     Y1
	x-ba    REM1
	xza     TEMP
	x-ba    TEMP
	xc<<x   X3

	DIV (X3)
	DIV (X3)
	DIV (X3)
	DIV (X3)
	DIV (X3)
	DIV (X3)
	DIV (X3)
	DIV2 (X3,X2)

	DIV (X2)
	DIV (X2)
	DIV (X2)
	DIV (X2)
	DIV (X2)
	DIV (X2)
	DIV (X2)
	DIV2 (X2,X1)

	DIV (X1)
	DIV (X1)
	DIV (X1)
	DIV (X1)
	DIV (X1)
	DIV (X1)
	DIV (X1)
	DIV2 (X1,X0)

	DIV (X0)
	DIV (X0)
	DIV (X0)
	DIV (X0)
	DIV (X0)
	DIV (X0)
	DIV (X0)

	bs?     X0,0
	ret

	xta     Y0
	x+a     REM0
	xta     Y1
	x+ca    REM1
	ret
