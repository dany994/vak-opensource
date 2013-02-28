/*
 * Simple RTL simulator.
 *
 * Copyright (C) 2013 Serge Vakulenko <serge@vak.ru>
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this file and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING.txt" for more details.
 */
#include <stdlib.h>
#include <setjmp.h>

/*
 * Forward typedefs
 */
typedef struct signal_t signal_t;       /* Signal */
typedef struct process_t process_t;     /* Process */
typedef struct hook_t hook_t;           /* Sensitivity hook */
typedef unsigned long long value_t;     /* Value of signal or time */

/*--------------------------------------
 * Time
 */
value_t time_ticks;             /* Current simulation time */

/*--------------------------------------
 * Signal
 */
struct signal_t {
    signal_t    *next;          /* Member of active list */
    hook_t      *activate;      /* Sensitivity list: processes to activate */
    const char  *name;          /* Name for log file */
    value_t     value;          /* Current value */
    value_t     new_value;      /* Value for next cycle */
};

signal_t *signal_active;        /* List of active signals for the current cycle */

void signal_set (signal_t *sig, value_t value);

#define signal_init(_name, _value) { 0, 0, _name, _value, _value }

/*--------------------------------------
 * Process
 */
struct process_t {
    process_t   *next;          /* Member of event queue */
    const char  *name;          /* Name for log file */
    value_t     delay;          /* Time to wait */
    jmp_buf     context;        /* User context for thread switching.
                                 * Six words for i386 architecture:
                                 * EBX, ESI, EDI, EBP, ESP, PC. */
};

process_t *process_current;     /* Current running process */
process_t *process_queue;       /* Queue of pending events */
process_t process_main;         /* Main process */

void process_wait (void);
void process_delay (unsigned ticks);

#define process_init(_name, _func, _nbytes) ({ \
        process_t *_proc = alloca (_nbytes); \
        _proc->name = _name; \
        _proc->delay = 0; \
        _proc->context[0].__jmpbuf[0] = 0; \
        _proc->context[0].__jmpbuf[1] = 0; \
        _proc->context[0].__jmpbuf[2] = 0; \
        _proc->context[0].__jmpbuf[3] = 0; \
        _proc->context[0].__jmpbuf[4] = _nbytes + (size_t) _proc; \
        _proc->context[0].__jmpbuf[5] = (size_t) _func; \
        _proc->next = process_queue; \
        process_queue = _proc; \
    })


/*--------------------------------------
 * Sensitivity hook
 */
struct hook_t {
    hook_t      *next;          /* Member of sensitivity list */
    process_t   *process;       /* Process to activate */
    int         edge;           /* Edge, if nonzero */
#define POSEDGE 1
#define NEGEDGE 2
};

#define process_sensitive(_sig, _edge) { \
        hook_t *_hook = alloca (sizeof(hook_t)); \
        _hook->process = process_current; \
        _hook->edge = _edge; \
        _hook->next = (_sig)->activate; \
        (_sig)->activate = _hook; \
    }
