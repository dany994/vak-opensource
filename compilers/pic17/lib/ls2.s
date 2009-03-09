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
 * Left unsigned short shift.
 * Shift (A1,A2) by the counter in WREG.
 * Leave the result in (A1,WREG).
 */
#include "pic17c43.inc"

ls2:
	a&c     15
	a?
	goto	1f
	xta	A2
	ret
1:      bz      ALUSTA,C
	xc<<x   A2
	xc<<x   A1
	a--?
	goto    1b
	xta     A2
	ret
