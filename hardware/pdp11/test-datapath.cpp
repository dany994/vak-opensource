#include <getopt.h>
#include <verilated.h>		// Defines common routines
#include "Vdatapath.h"		// From Verilating "datapath.v"
#include "opcode.h"
#if VM_TRACE
# include <verilated_vcd_c.h>	// Trace file format header
#endif

Vdatapath *uut;			// Unit under test

static void delay (unsigned n)
{
    unsigned t = main_time + n;
    while (main_time != t && !Verilated::gotFinish()) {
	uut->eval();                    // Evaluate model
	main_time++;                    // Time passes...
    }
}

int main (int argc, char **argv)
{
    for (;;) {
        switch (getopt (argc, argv, "h")) {
        case EOF:
            break;
        default:
usage:      fprintf (stderr, "Usage:\n");
            fprintf (stderr, "        simx [-h] file.out\n");
            fprintf (stderr, "Options:\n");
            fprintf (stderr, "        -h    Print this message\n");
            exit (-1);
        }
        break;
    }
    argc -= optind;
    argv += optind;
    if (argc > 1)
        goto usage;

    uut = new Vdatapath;		// Create instance of module

    Verilated::commandArgs (argc, argv);
    Verilated::debug (0);
#if VM_TRACE                            // If verilator was invoked with --trace
    Verilated::traceEverOn (true);	// Verilator must compute traced signals
    VL_PRINTF ("Enabling waves...\n");
    VerilatedVcdC* tfp = new VerilatedVcdC;
    uut->trace (tfp, 99);               // Trace 99 levels of hierarchy
    tfp->open ("datapath_dump.vcd");    // Open the dump file
#endif

    if (argc == 1) {                    // Load executable file
        load_file (argv[0], 0500, uut->v__DOT__ram__DOT__memory);
    }
    uut->reset = 1;                     // Initialize inputs
    uut->clk = 0;
    VL_PRINTF ("(%u) started datapath testing\n", main_time);

    // Wait for global reset to finish
    delay (5);
    uut->reset = 0;
    VL_PRINTF ("(%u) turn reset off\n", main_time);

    uut->v__DOT__regfile__DOT__r[7] = 0500;
    VL_PRINTF ("(%u) PC := %6o\n", main_time, uut->v__DOT__regfile__DOT__r[7]);

    while (main_time < 200 && ! Verilated::gotFinish()) {
        VL_PRINTF ("\n");
	uut->eval();                    // Evaluate model
#if VM_TRACE
	if (tfp) tfp->dump (main_time);	// Create waveform trace for this timestamp
#endif
        uut->clk ^= 1;
	main_time++;                    // Time passes...
    }
    uut->final();
#if VM_TRACE
    if (tfp) tfp->close();
#endif
    VL_PRINTF ("(%u) finished datapath testing\n", main_time);
    return 0;
}
