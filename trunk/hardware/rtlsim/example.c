//
// Example of simple simulation.
//
#include "rtlsim.h"

signal_t clock;         // Clock input of the design
signal_t reset;         // active high, synchronous Reset input
signal_t enable;        // Active high enable signal for counter
signal_t count;         // 4 bit vector output of the counter

//
// 4-bit up-counter with synchronous active high reset and
// with active high enable signal.
//
void proc_counter ()
{
    // Create a sensitivity list
    process_sensitive (&clock, POSEDGE);
    process_sensitive (&reset, 0);

    for (;;) {
        // Wait for event from the sensitivity list
        process_wait();

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
}

int main (int argc, char **argv)
{
    // Initialize signals
    signal_init (clock,  "clock",  0);
    signal_init (reset,  "reset",  0);
    signal_init (enable, "enable", 0);
    signal_init (count,  "count",  0);

    // Create processes
    process_create (proc_counter, alloca(2048));

    int i;                      // Issue some clock pulses
    for (i=0; i<5; i++) {
        signal_set (clock, 0);
        process_delay (10);
        signal_set (clock, 1);
        process_delay (10);
    }
    signal_set (reset, 1);      // Assert the reset
    printf ("(%llu) Asserting reset\n", simulation_time);
    for (i=0; i<10; i++) {
        signal_set (clock, 0);
        process_delay (10);
        signal_set (clock, 1);
        process_delay (10);
    }
    signal_set (reset, 0);      // De-assert the reset
    printf ("(%llu) De-Asserting reset\n", simulation_time);
    for (i=0; i<5; i++) {
        signal_set (clock, 0);
        process_delay (10);
        signal_set (clock, 1);
        process_delay (10);
    }
    printf ("(%llu) Asserting Enable\n", simulation_time);
    signal_set (enable, 1);     // Assert enable
    for (i=0; i<20; i++) {
        signal_set (clock, 0);
        process_delay (10);
        signal_set (clock, 1);
        process_delay (10);
    }
    printf ("(%llu) De-Asserting Enable\n", simulation_time);
    signal_set (enable, 0);     // De-assert enable
    process_delay (10);

    // Terminate simulation
    printf ("(%llu) Terminating simulation\n", simulation_time);
    return 0;
}
