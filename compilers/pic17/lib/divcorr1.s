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
 * Divide by constant correction routine.
 * Parameters:
 *   A0 - divider
 *   A1 - dividend
 *   PRODH - approximation
 * Return:
 *   A0 - quotient
 *   A1 - remainder
 */
#include "pic17c43.inc"

divcorr1:
	atx	A3		/* A3 - divider */
	xtr	PRODH,A2	/* A2 - approximation */
	a*x	A2
	xta	PRODL		/* divider * approximation */
	x-a	A1		/* remainder = divider * approximation */

	nb?			/* if remainder >= 0... */
	goto	1f		/* ...fine */

	x--	A2		/* else decrement approximation */
	xta	A3
	x+a	A1		/* remainder += divider */
1:
	xta	A2
	ret
