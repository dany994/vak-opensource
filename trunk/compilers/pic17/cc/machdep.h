/*
 * C Compiler for PIC17C4x processors.
 *
 * Copyright (C) 1997-1998 Cronyx Engineering Ltd.
 * Author: Serge Vakulenko, <vak@cronyx.ru>
 *
 * This software is distributed with NO WARRANTIES, not even the implied
 * warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors grant any other persons or organisations permission to use
 * or modify this software as long as this message is kept with the software,
 * all derivative works or modified versions.
 *
 * For permission to use this software in commercial purposes,
 * please, contact the author.
 */
#define MAXARGLEN	4

#define WREG		0x0a

#define ASM_TRUE        " cta 1;"
#define ASM_FALSE       " cta 0;"
#define ASM_RETURN      " ret;"
#define ASM_RETINT      " reti;"
#define ASM_GOTO        " goto %n;"
#define ASM_CALL        " call %n;"
#define ASM_GOTOLAB     " goto L%d;"
#define ASM_COND        " a|c 0;"
#define ASM_DEFLAB      "L%d:;"
