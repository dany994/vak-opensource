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
 * Divide by constant correction routine, two-byte numbers.
 * Parameters:
 *   A1,A0 - divider
 *   A9,A8 - dividend
 *   A3,A2 - approximation
 * Return:
 *   A1,A0 - quotient
 *   A9,A8 - remainder
 */
#include "pic17c43.inc"

#define DividerL  A4
#define DividerH  A1
#define ApproxL   A2
#define ApproxH   A3
#define RemL      A8
#define RemH      A9

divcorr2:
	atx	DividerL		/* A1,A4 - divider */

	/* multiply divider and approximation into (A7,A6,A5),
	 * msb lost, A7 later ignored */
#define R0	A5
#define R1	A6
#define R2	A7

	xta     DividerH
	a*x     ApproxH
	rtx     PRODL,R2        /* r2 := x1 * y1 */

	xta     DividerL
	a*x     ApproxL
	rtx     PRODH,R1        /* r1r0 := x0 * y0 */
	rtx     PRODL,R0

	a*x     ApproxH         /* x0 * y1 */
	xta     PRODL
	x+a     R1
	xta     PRODH
	x+ca    R2

	xta     DividerH	/* x1 */
	a*x     ApproxL         /* x1 * y0 */
	xta     PRODL
	x+a     R1
	xta     PRODH
	x+ca    R2

	/* remainder contains the dividend;
	 * subtract (A6,A5) from it */
	xta	R0
	x-a	RemL
	b?
	x--	RemH
	b?			/* if remainder < 0... */
	goto	2f

	xta	R1
	x-a	RemH
	b?
	goto	4f		/* if remainder < 0... */
	x?	R2
	goto	4f
1:
	xtr	ApproxH,A1	/* get quotient */
	xta	ApproxL
	ret

2:	/* remainder < 0 */
	xta	R1
	x-a	RemH
4:
	x--	ApproxL		/* decrement approximation */
	b?
	x--	ApproxH

	xta	DividerL
	x+a	RemL		/* remainder += divider */
	xta	DividerH
	x+ca	RemH

	goto	1b
