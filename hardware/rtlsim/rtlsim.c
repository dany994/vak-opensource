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
    if (process_current != old) {
    	//printf ("(%llu) Switch process '%s' -> '%s'\n", time_ticks, old->name, process_current->name);
        if (! _setjmp (old->context))
            longjmp(process_current->context, 1);
    }
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
 * Simplified versions of longjmp() and _setjmp().
 */
asm (
    ".global longjmp \n"
    ".type longjmp,@function \n"
"longjmp: \n"
    "mov 4(%esp),%edx \n"
    "mov 8(%esp),%eax \n"
    "test %eax,%eax \n"
    "jnz 1f \n"
    "inc %eax \n"
"1: \n"
    "mov (%edx),%ebx \n"
    "mov 4(%edx),%esi \n"
    "mov 8(%edx),%edi \n"
    "mov 12(%edx),%ebp \n"
    "mov 16(%edx),%ecx \n"
    "mov %ecx,%esp \n"
    "mov 20(%edx),%ecx \n"
    "jmp *%ecx ");

asm (
    ".global _setjmp \n"
    ".type _setjmp,@function \n"
"_setjmp: \n"
    "mov 4(%esp), %eax \n"
    "mov %ebx, (%eax) \n"
    "mov %esi, 4(%eax) \n"
    "mov %edi, 8(%eax) \n"
    "mov %ebp, 12(%eax) \n"
    "lea 4(%esp), %ecx \n"
    "mov %ecx, 16(%eax) \n"
    "mov (%esp), %ecx \n"
    "mov %ecx, 20(%eax) \n"
    "xor %eax, %eax \n"
    "ret ");
