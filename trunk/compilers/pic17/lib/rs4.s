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
 * Right unsigned long shift.
 * Shift (A3,A2,A1,A4) by the counter in WREG.
 * Leave the result in (A3,A2,A1,WREG).
 */
#include "pic17c43.inc"

rs4:
	a&c     31
	a?
	goto	1f
	xta	A4
	ret
1:      bz      ALUSTA,C
	xc>>x   A3
	xc>>x   A2
	xc>>x   A1
	xc>>x   A4
	a--?
	goto    1b
	xta     A4
	ret
