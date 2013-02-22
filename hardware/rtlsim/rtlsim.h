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

process_t *process_current;     // Current running process
process_t *process_queue;       // Queue of pending events

//
// Sensitivity hook
//
typedef struct activation_t activation_t;

struct activation_t {
    signal_t    *signal;        // Sensitive to this signal
    process_t   *process;       // Process to activate
    activation_t *next;         // Member of sensitivity list
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

//
// Functions
//
void process_main (process_t *proc, int argc, char **argv);
process_t *process_init (char *stack, void (*func)());
void process_sensitive (process_t *proc, activation_t *act);
void process_wait (unsigned delay);
void process_finish (void);

void signal_set (signal_t *sig, uint64_t value);
