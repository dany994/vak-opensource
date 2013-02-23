//
// Simple RTL simulator.
// Copyright (C) 2013 Serge Vakulenko <serge@vak.ru>
//
#include <ucontext.h>

//
// Forward typedefs
//
typedef struct signal_t signal_t;       // Signal
typedef struct process_t process_t;     // Process
typedef struct hook_t hook_t;           // Sensitivity hook
typedef unsigned long long value_t;     // Value of signal or time

//--------------------------------------
// Time
//
value_t time_ticks;             // Current simulation time

//--------------------------------------
// Signal
//
struct signal_t {
    signal_t    *next;          // Member of active list
    signal_t    *prev;

    const char  *name;          // Name for log file
    hook_t      *activate;      // Sensitivity list: processes to activate

    value_t     value;          // Current value
    value_t     new_value;      // Value for next cycle
};

signal_t *signal_active;        // List of active signals for the current cycle

void signal_init (signal_t *sig, const char *name, value_t value);
void signal_set (signal_t *sig, value_t value);

//--------------------------------------
// Process
//
struct process_t {
    process_t   *next;          // Member of event queue
    process_t   *prev;

    const char  *name;          // Name for log file
    value_t     delay;          // Time to wait
    ucontext_t  context;        // User context for thread switching

    char        stack[1];       // Space for thread stack
};

process_t *process_current;     // Current running process
process_t *process_queue;       // Queue of pending events
process_t proc_main;            // Main process

process_t *process_create (const char *name, void (*func)(void), void *stack);
void process_wait (void);
void process_delay (unsigned ticks);

//--------------------------------------
// Sensitivity hook
//
struct hook_t {
    hook_t      *next;          // Member of sensitivity list
    process_t   *process;       // Process to activate
    int         edge;           // Edge, if nonzero
#define POSEDGE 1
#define NEGEDGE 2
};

#define process_sensitive(sig, edge) { \
        hook_t *a = alloca (sizeof(hook_t)); \
        a->process = process_current; \
        a->edge = edge; \
        a->next = sig->activate; \
        sig->activate = a; \
    }
