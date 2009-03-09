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
 * Unsigned divide/remainder.
 * Divide A1 by A2.
 * Leave the quotient in A1,
 * the remainder in A3.
 * (A1, A3) := (A1/A2, A1%A2)
 */
#include "pic17c43.inc"

#define LA      A1
#define LDIV    A2
#define LREM    A3

#define DIVBIT \
	xc<<a   LA; \
	xc<<x   LREM; \
	xta     LDIV; \
	x-a     LREM; \
	nb?; \
	goto    1f; \
	x+a     LREM; \
	bz      ALUSTA,C; \
1:      xc<<x   LA

divmod11:
	xz      LREM
	DIVBIT
	DIVBIT
	DIVBIT
	DIVBIT
	DIVBIT
	DIVBIT
	DIVBIT
	DIVBIT
	ret

