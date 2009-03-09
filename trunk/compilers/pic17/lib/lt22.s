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
 * Compare "<" unsigned short.
 * Leave the result in WREG.
 * Call (A<B):
 *      xtr Ahigh, A1
 *      xtr Alow, A2
 *      xtr Blow, A3
 *      xta Bhigh
 *      call lt22
 */
#include "pic17c43.inc"

#define Ahigh A1
#define Alow  A2
#define Blow  A3

lt22:
	a-x     Ahigh   /* WREG=Ahigh-Bhigh, C=(Ahigh>=Bhigh) */
	IF_LT           /* if Ahigh<Bhigh... */
	retc    1       /* ...return true */

	nz?
	retc    0       /* if Ahigh>Bhigh return false */

	xta     Blow
	a-x     Alow    /* WREG=Alow-Blow, C=(Alow>=Blow) */
	IF_GE           /* if Alow>=Blow... */
	retc    0       /* ...return false */

	retc    1       /* true */
