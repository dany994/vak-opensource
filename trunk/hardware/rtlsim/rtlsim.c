//
// Simple RTL simulator.
// Copyright (C) 2013 Serge Vakulenko <serge@vak.ru>
//
#include <stdio.h>
#include "rtlsim.h"

//
// Set a value of the signal.
// Value will be updated on next simulation cycle.
// If the value changed, put the signal to active list.
//
void signal_set (signal_t *sig, value_t value)
{
    sig->new_value = value;

    if (value != sig->value && sig->next == 0) {
        // Value changed - put to list of active signals
        printf ("(%llu) Signal '%s' activated\n", time_ticks, sig->name);
        sig->next = signal_active;
        signal_active = sig;
    }
}

static int edge_is_sensitive (signal_t *sig, hook_t *hook)
{
    if (hook->edge == 0)
        return 1;
    if ((hook->edge & POSEDGE) && sig->value == 0 && sig->new_value != 0)
        return 1;
    if ((hook->edge & NEGEDGE) && sig->value != 0 && sig->new_value == 0)
        return 1;
    return 0;
}

//
// Wait for activation of any signal from a sensitivity list.
// Switch to next active process.
//
void process_wait (void)
{
    process_t *old = process_current;

    if (process_queue == 0) {
        // Cannot happen.
        printf ("Internal error: empty process queue\n");
        exit (-1);
    }
    if (process_current->delay != 0) {
        // Delta cycle finished.
        printf ("(%llu) Delta cycle finished\n", time_ticks);

        // Schedule processes for active signals.
        while (signal_active != 0) {
            hook_t *hook = signal_active->activate;
            signal_t *next = signal_active->next;

            // Handle all processes, sensitive to this signal.
            for (; hook != 0; hook = hook->next) {
                if (hook->process->next == 0 &&
                    edge_is_sensitive (signal_active, hook))
                {
                    // Put the process to queue of pending events.
                    printf ("(%llu) Process '%s' activated\n", time_ticks, hook->process->name);
                    hook->process->next = process_queue;
                    process_queue = hook->process;
                }
            }
            // Setup new value
            signal_active->value = signal_active->new_value;
            signal_active->next = 0;
            signal_active = next;
        }
    }
    // Select next process from the queue.
    process_current = process_queue;
    process_queue = process_queue->next;
    process_current->next = 0;
    if (process_current->delay != 0) {
        // Advance time.
        time_ticks += process_current->delay;
        printf ("(%llu) Time advanced\n", time_ticks);
    }
    printf ("(%llu) Switch process '%s' -> '%s'\n", time_ticks, old->name, process_current->name);
    swapcontext(&old->context, &process_current->context);
}

//
// Delay the current process by a given number of clock ticks.
//
void process_delay (unsigned ticks)
{
    // On first call, initialize the main process
    if (process_current == 0) {
        process_main.name = "main";
        getcontext (&process_main.context);
        process_current = &process_main;
    }

    // Put the current process to queue of pending events.
    //TODO: enqueue (process_current, ticks);

    // Switch to next active process.
    process_wait();
}
