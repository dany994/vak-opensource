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
 * Compare ">" unsigned short.
 * Leave the result in WREG.
 * Call (A>B):
 *      xtr Ahigh, A1
 *      xtr Alow, A2
 *      xtr Blow, A3
 *      xta Bhigh
 *      call gt22
 */
#include "pic17c43.inc"

#define Ahigh A1
#define Alow  A2
#define Blow  A3

gt22:
	a-x     Ahigh   /* WREG=Ahigh-Bhigh, C=(Ahigh>=Bhigh) */
	IF_LT           /* if Ahigh<Bhigh... */
	retc    0       /* ...return false */

	nz?
	retc    1       /* if Ahigh>Bhigh return true */

	xta     Blow
	a-x     Alow    /* WREG=Alow-Blow, C=(Alow>=Blow) */
	IF_LE_GOTO 1f   /* if Alow<=Blow goto false */
	retc    1       /* true */

1:	retc    0       /* false */
