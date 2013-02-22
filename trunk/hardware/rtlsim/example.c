//
// 4-bit up-counter with synchronous active high reset and
// with active high enable signal.
//
#include "rtlsim.h"

signal_t    clock;          // Clock input of the design
signal_t    reset;          // active high, synchronous Reset input
signal_t    enable;         // Active high enable signal for counter
signal_t    count;          // 4 bit vector output of the counter

process_t proc_main;        // Descriptor for main() process

char stack_counter [4000];  // Space for counter process

// We trigger the below block with respect to positive
// edge of the clock and also when ever reset changes state
activation_t sensitive_counter_reset = { &reset };
activation_t sensitive_counter_clock = { &clock };

// The actual counter logic
void func_counter ()
{
    // At every rising edge of clock we check if reset is active
    // If active, we load the counter output with 4'b0000
    if (reset.value != 0) {
        signal_set (&count, 0);

    // If enable is active, then we increment the counter
    } else if (enable.value != 0) {
        int newval = (count.value + 1) & 15;
        signal_set (&count, newval);
        printf ("(%llu) Incremented Counter %u\n", simulation_time, newval);
    }
}

int main (int argc, char **argv)
{
    process_main (&proc_main, argc, argv);

    // Counter: initialize the process
    process_t *proc_counter = process_init (stack_counter, func_counter);

    // Create the sensitivity list
    process_sensitive (proc_counter, &sensitive_counter_reset);
    process_sensitive (proc_counter, &sensitive_counter_clock);

    // Initialize all variables
    signal_set (reset, 0);      // initial value of reset
    signal_set (enable, 0);     // initial value of enable
    process_wait (10);

    int i;
    for (i=0; i<5; i++) {
        signal_set (clock, 0);
        process_wait (10);
        signal_set (clock, 1);
        process_wait (10);
    }
    signal_set (reset, 1);      // Assert the reset
    printf ("(%llu) Asserting reset\n", simulation_time);
    for (i=0; i<10; i++) {
        signal_set (clock, 0);
        process_wait (10);
        signal_set (clock, 1);
        process_wait (10);
    }
    signal_set (reset, 0);      // De-assert the reset
    printf ("(%llu) De-Asserting reset\n", simulation_time);
    for (i=0; i<5; i++) {
        signal_set (clock, 0);
        process_wait (10);
        signal_set (clock, 1);
        process_wait (10);
    }
    printf ("(%llu) Asserting Enable\n", simulation_time);
    signal_set (enable, 1);     // Assert enable
    for (i=0; i<20; i++) {
        signal_set (clock, 0);
        process_wait (10);
        signal_set (clock, 1);
        process_wait (10);
    }
    printf ("(%llu) De-Asserting Enable\n", simulation_time);
    signal_set (enable, 0);     // De-assert enable
    process_wait (10);

    // Terminate simulation
    printf ("(%llu) Terminating simulation\n", simulation_time);
    process_finish();
    return 0;
}
