//
// Simple RTL simulator.
// Copyright (C) 2013 Serge Vakulenko <serge@vak.ru>
//
#include "rtlsim.h"

void signal_init (signal_t *sig, const char *name, value_t value)
{
    sig->name      = name;
    sig->value     = value;
    sig->new_value = value;
    sig->activate  = 0;
    sig->next      = 0;
    sig->prev      = 0;
}

void signal_set (signal_t *sig, value_t value)
{
    sig->new_value = value;

    if (value != sig->value && ! sig->next) {
        // Value changed - put to list of active signals
        sig->next = signal_active;
        signal_active = sig;
    }
}

process_t *process_create (const char *name, void (*func)(void), void *stack)
{
}

void process_wait (void)
{
}

void process_delay (unsigned ticks)
{
}
