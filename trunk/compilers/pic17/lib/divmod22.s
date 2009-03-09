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
 * Unsigned short-by-short divide/remainder.
 * Divide (A1,A2) by (A3,A4).
 * Leave the quotient in (A1,A2),
 * the remainder in (A5,A6).
 */
#include "pic17c43.inc"

#define A_l     A2
#define A_h     A1
#define B_l     A3
#define B_h     A4
#define REM_l   A5
#define REM_h   A6

#define DIVHIGH \
	xc<<a   A_h; \
	xc<<x   REM_l; \
	xc<<x   REM_h; \
	xta     B_l; \
	x-a     REM_l; \
	xta     B_h; \
	x-ba    REM_h; \
	nb?; \
	goto    1f; \
	xta     B_l; \
	x+a     REM_l; \
	xta     B_h; \
	x+ca    REM_h; \
	bz      ALUSTA,C; \
1:      xc<<x   A_h

#define DIVLOW \
	xc<<a   A_l; \
	xc<<x   REM_l; \
	xc<<x   REM_h; \
	xta     B_l; \
	x-a     REM_l; \
	xta     B_h; \
	x-ba    REM_h; \
	nb?; \
	goto    1f; \
	xta     B_l; \
	x+a     REM_l; \
	xta     B_h; \
	x+ca    REM_h; \
	bz      ALUSTA,C; \
1:      xc<<x   A_l

divmod22:
	xz      REM_l
	xz      REM_h

	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH

	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW

	ret
