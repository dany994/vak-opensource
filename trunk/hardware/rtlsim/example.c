/*
 * Example of simple simulation.
 */
#include <stdio.h>
#include "rtlsim.h"

#define STACK_NBYTES    4096    /* Stack size for processes */

/* Initialize signals */
signal_t clock  = signal_init ("clock",  0); /* Clock input of the design */
signal_t reset  = signal_init ("reset",  0); /* Active high, synchronous Reset input */
signal_t enable = signal_init ("enable", 0); /* Active high enable signal for counter */
signal_t count  = signal_init ("count",  0); /* 4-bit vector output of the counter */

/*
 * Clock generator.
 */
void do_clock ()
{
    for (;;) {
        signal_set (&clock, 0);
        process_delay (10);
        signal_set (&clock, 1);
        process_delay (10);
    }
}

/*
 * 4-bit up-counter with synchronous active high reset and
 * with active high enable signal.
 */
void do_counter ()
{
    /* Create a sensitivity list. */
    process_sensitive (&clock, POSEDGE);
    process_sensitive (&reset, 0);

    for (;;) {
        /* Wait for event from the sensitivity list. */
        process_wait();

        /* At every rising edge of clock we check if reset is active.
         * If active, we load the counter output with 4'b0000. */
        if (reset.value != 0) {
            signal_set (&count, 0);

        /* If enable is active, then we increment the counter. */
        } else if (enable.value != 0) {
            printf ("(%llu) Incremented Counter %llu\n", time_ticks, count.value);
            signal_set (&count, (count.value + 1) & 15);
        }
    }
}

int main (int argc, char **argv)
{
    /* Create processes. */
    process_init ("clock", do_clock, STACK_NBYTES);
    process_init ("counter", do_counter, STACK_NBYTES);

    /* Issue some clock pulses */
    process_delay (100);
    signal_set (&reset, 1);     /* Assert the reset */
    printf ("(%llu) Asserting reset\n", time_ticks);
    process_delay (200);
    signal_set (&reset, 0);     /* De-assert the reset */
    printf ("(%llu) De-Asserting reset\n", time_ticks);
    process_delay (100);
    printf ("(%llu) Asserting Enable\n", time_ticks);
    signal_set (&enable, 1);    /* Assert enable */
    process_delay (400);
    printf ("(%llu) De-Asserting Enable\n", time_ticks);
    signal_set (&enable, 0);    /* De-assert enable */

    /* Terminate simulation. */
    printf ("(%llu) Terminating simulation\n", time_ticks);
    return 0;
}
