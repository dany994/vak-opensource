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
 * Unsigned short-by-char divide/remainder.
 * Divide (A1,A2) by A3.
 * Leave the quotient in (A1,A2),
 * the remainder in A4.
 */
#include "pic17c43.inc"

#define Alow    A2
#define Ahigh   A1
#define B       A3
#define REM     A4
#define TEMP    A5

#define DIVHIGH \
	xc<<a   Ahigh; \
	xc<<x   REM; \
	xta     B; \
	x-a     REM; \
	nb?; \
	goto    1f; \
	x+a     REM; \
	bz      ALUSTA,C; \
1:      xc<<x   Ahigh

#define DIVLOW \
	xc<<a   Alow; \
	xc<<x   REM; \
	xc<<x   TEMP; \
	xta     B; \
	x-a     REM; \
	az; \
	x-ba    TEMP; \
	nb?; \
	goto 1f; \
	xta     B; \
	x+a     REM; \
	az; \
	x+ca    TEMP; \
	bz      ALUSTA,C; \
1:      xc<<x   Alow

divmod21:
	xz      REM
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH
	DIVHIGH

	xz      TEMP
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW
	DIVLOW

	ret
