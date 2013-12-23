#include <stdio.h>
#include <unistd.h>

#include "calc.h"

//
// Symbols on display.
//
unsigned char display [12];
unsigned char display_old [12];         // Previous state
unsigned char show_dot [12];            // Show a decimal dot

unsigned step_num = 0;

//
// Simulate one cycle of the calculator.
// Return 0 when stopped, or 1 when running a user program.
//
void step (int x, int y, unsigned rgd)
{
    int i, refresh;

    step_num++;
    calc_step (x, y, rgd, display, show_dot);

    refresh = 0;
    for (i=0; i<12; i++) {
        if (display_old[i] != display[i])
            refresh = 1;
        display_old[i] = display[i];
    }
    if (refresh) {
        printf ("%3u -- '", step_num);
        for (i=0; i<12; i++) {
            putchar ("0123456789-LCRE " [display[i]]);
            if (show_dot[i])
                putchar ('.');
        }
        printf ("'\n");
    }
}

int main()
{
    // 10 - radians, 11 - grads, 12 - degrees
    unsigned rad_grad_deg = 10;

    printf ("Started MK-54.\n");
    calc_init();

    usleep (100000); step (0, 0, rad_grad_deg);
    usleep (100000); step (5, 1, rad_grad_deg);    // 3
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (6, 1, rad_grad_deg);    // 4
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (11,8, rad_grad_deg);    // B^
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (9, 1, rad_grad_deg);    // 7
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (5, 8, rad_grad_deg);    // /
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (3, 1, rad_grad_deg);    // 1
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (9, 8, rad_grad_deg);    // ВП
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (7, 1, rad_grad_deg);    // 5
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (2, 1, rad_grad_deg);    // 0
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (11,9, rad_grad_deg);    // F
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (4, 8, rad_grad_deg);    // x^2
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (11,9, rad_grad_deg);    // F
                     step (0, 0, rad_grad_deg);
    usleep (100000); step (4, 8, rad_grad_deg);    // x^2
#if 0
    for (;;) {
        usleep (100000); step (0, 0, rad_grad_deg);
    }
#endif
    return 0;
}
