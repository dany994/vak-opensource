#include <ucontext.h>

//
// Process
//
typedef struct process_t process_t;

struct process_t {
    process_t   *next;          // Member of event queue
    process_t   *prev;

    uint64_t    delay;          // Time to wait

    ucontext_t  context;        // User context for thread switching
    char        stack[1];       // Space for thread stack
};

process_t *process_queue;       // Queue of pending events

//
// Sensitivity hook
//
typedef struct activation_t activation_t;

struct activation_t {
    activation_t *next;         // Member of sensitivity list
    process_t   *process;       // Process to activate
};

//
// Signal
//
typedef struct signal_t signal_t;

struct signal_t {
    signal_t    *active_next;   // Member of active list
    signal_t    *active_prev;

    uint64_t    value;          // Current value
    uint64_t    new_value;      // Value for next cycle

    activation_t *activate;     // Sensitivity list: processes to call
};

signal_t *signal_active;        // List of active signals for the current cycle

//
// Time
//
uint64_t simulation_time;       // Current time
