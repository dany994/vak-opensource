//
// Simple RTL simulator.
// Copyright (C) 2013 Serge Vakulenko <serge@vak.ru>
//
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
        // TODO: check edge
        sig->next = signal_active;
        signal_active = sig;
    }
}

//
// Wait for activation of any signal from a sensitivity list.
// Switch to next active process.
//
void process_wait (void)
{
    process_t *old = process_current;

    // Schedule processes for active signals
    for (; signal_active != 0; signal_active = signal_active->next) {
        hook_t *hook = signal_active->activate;

        // Handle all processes, sensitive to this signal
        for (; hook != 0; hook = hook->next) {
            if (hook->process->next == 0) {
                // Put the process to queue of pending events.
                hook->process->next = process_queue;
                process_queue = hook->process;
            }
        }
    }
    if (process_queue == 0) {
        // Cannot happen.
        printf ("Internal error: empty process queue\n");
        exit (-1);
    }
    // Select next process from the queue.
    process_current = process_queue;
    process_queue = process_queue->next;
    if (process_current->delay != 0) {
        // Advance time.
        time_ticks += process_current->delay;
    }
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
    TODO: enqueue (process_current, ticks);

    // Switch to next active process.
    process_wait();
}
