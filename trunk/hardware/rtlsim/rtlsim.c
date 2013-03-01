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
#include <stdio.h>
#include <string.h>
#include "rtlsim.h"

/*
 * Set a value of the signal.
 * Value will be updated on next simulation cycle.
 * If the value changed, put the signal to active list.
 */
void signal_set (signal_t *sig, value_t value)
{
    sig->new_value = value;

    if (value != sig->value && sig->next == 0) {
        /* Value changed - put to list of active signals. */
        //printf ("(%llu) Signal '%s' changed %s\n", time_ticks, sig->name, sig->new_value ? "HIGH" : "LOW");
        sig->next = signal_active;
        signal_active = sig;
    }
}

/*
 * Return true when the signal change matches the edge flag.
 */
static int edge_is_sensitive (signal_t *sig, int edge)
{
    if (edge == 0)
        return 1;
    if ((edge & POSEDGE) && sig->value == 0 && sig->new_value != 0)
        return 1;
    if ((edge & NEGEDGE) && sig->value != 0 && sig->new_value == 0)
        return 1;
    return 0;
}

/*
 * Wait for activation of any signal from a sensitivity list.
 * Switch to next active process.
 */
void process_wait (void)
{
    process_t *old = process_current;

    if (process_queue == 0) {
        /* Cannot happen. */
        printf ("Internal error: empty process queue\n");
        exit (-1);
    }
    if (process_queue->delay != 0) {
        /* Delta cycle finished.
         * Schedule processes for active signals. */
        while (signal_active != 0) {
            hook_t *hook = signal_active->activate;
            signal_t *next = signal_active->next;

            /* Handle all processes, sensitive to this signal. */
            for (; hook != 0; hook = hook->next) {
                if (hook->process->next == 0 &&
                    edge_is_sensitive (signal_active, hook->edge))
                {
                    /* Put the process to queue of pending events. */
                    //printf ("(%llu) Process '%s' activated\n", time_ticks, hook->process->name);
                    hook->process->next = process_queue;
                    process_queue = hook->process;
                }
            }
            /* Setup a new signal value. */
            signal_active->value = signal_active->new_value;
            signal_active->next = 0;
            signal_active = next;
        }
    	//printf ("(%llu) ---\n", time_ticks);
    }
    /* Select next process from the queue. */
    process_current = process_queue;
    process_queue = process_queue->next;
    process_current->next = 0;
    if (process_current->delay != 0) {
        /* Advance time. */
        time_ticks += process_current->delay;
    }
    if (process_current == old)
        return;

    /* Switch to new process. */
    //printf ("(%llu) Switch process '%s' -> '%s'\n", time_ticks, old->name, process_current->name);
#ifdef __i386__
    asm (
        "mov %%esp, 0(%1) \n"       /* Save ESP to context[0] */
        "call 1f \n"
     "1: pop %%ebx \n"              /* Compute address of label 1 */
        "lea 2f-1b(%%ebx), %%ebx \n"
        "mov %%ebx, 4(%1) \n"       /* Save address of label 2 to context[1] */
        "mov 0(%0),%%esp \n"        /* Restore ESP from context[0] */
        "mov 4(%0),%%ecx \n"        /* Get address from context[1] */
        "jmp *%%ecx \n "            /* Jump to address */
     "2: "
        : : "r" (process_current->context), "r" (old->context)
        : "bx", "si", "di", "bp");
#endif
}

/*
 * Delay the current process by a given number of clock ticks.
 */
void process_delay (unsigned ticks)
{
    /* On first call, initialize the main process. */
    if (process_current == 0) {
        process_main.name = "main";
        process_current = &process_main;
    }

    /* Put the current process to queue of pending events.
     * Keep the queue sorted. */
    process_t **q = &process_queue;
    process_t *p;
    while ((p = *q) && p->delay <= ticks) {
        if (p->delay > 0)
            ticks -= p->delay;
        q = &p->next;
    }
    process_current->delay = ticks;
    process_current->next = p;
    if (p)
            p->delay -= ticks;
    *q = process_current;

    /* Switch to next active process. */
    process_wait();
}

/*
 * Setup a context of the process, to call a given function
 * on a given stack.
 */
void _process_setup (process_t *proc, void (*func)(), unsigned nbytes)
{
    memset (proc->context, 0, sizeof(proc->context));

#ifdef __i386__
    /* For i386 architecture, we need to save ESP and EIP.
     * Stack must be aligned at 16-byte boundary.
     * We use jmp_buf in non-standard way. */
    int *context = (int*) proc->context;
    context[0] = (((int) proc + nbytes) & ~15) - 4;
    context[1] = (int) func;
#endif
}
