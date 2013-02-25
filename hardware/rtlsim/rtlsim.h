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
#include <ucontext.h>

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
    ucontext_t  context;        /* User context for thread switching */
};

process_t *process_current;     /* Current running process */
process_t *process_queue;       /* Queue of pending events */
process_t process_main;         /* Main process */

void process_wait (void);
void process_delay (unsigned ticks);

#define process_init(_name, _func, _nbytes) ({ \
        process_t *proc = alloca (_nbytes); \
        proc->name = _name; \
        proc->delay = 0; \
        getcontext (&proc->context); \
        proc->context.uc_stack.ss_sp = (char*) proc; \
        proc->context.uc_stack.ss_size = _nbytes; \
        proc->context.uc_link = 0; \
        makecontext (&proc->context, _func, 0); \
        proc->next = process_queue; \
        process_queue = proc; \
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
        hook_t *a = alloca (sizeof(hook_t)); \
        a->process = process_current; \
        a->edge = _edge; \
        a->next = (_sig)->activate; \
        (_sig)->activate = a; \
    }
