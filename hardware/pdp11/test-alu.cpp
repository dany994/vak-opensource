// DESCRIPTION: Verilator Example: Top level main for invoking model
//
// Copyright 2003-2011 by Wilson Snyder. This program is free software; you can
// redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.

#include <verilated.h>		// Defines common routines
#include "Valu.h"		// From Verilating "alu.v"
#include "opcode.h"		// From Verilating "alu.v"
#if VM_TRACE
# include <verilated_vcd_c.h>	// Trace file format header
#endif

#define N (1 << 3)
#define Z (1 << 2)
#define V (1 << 1)
#define C (1 << 0)

Valu *alu;			// Instantiation of module

unsigned main_time = 0;         // Current simulation time

double sc_time_stamp ()
{                               // Called by $time in Verilog
    return main_time;
}

static void delay (unsigned n)
{
    unsigned t = main_time + n;
    while (main_time != t && !Verilated::gotFinish()) {
	alu->eval();		// Evaluate model
	main_time++;		// Time passes...
    }
}

static const char *opname (unsigned op)
{
    switch (op & ~077) {
    case MOV:  return "MOV";
    case MOVB: return "MOVB";
    case CMP:  return "CMP";
    case CMPB: return "CMPB";
    case ADD:  return "ADD";
    case SUB:  return "SUB";
    case BIT:  return "BIT";
    case BITB: return "BITB";
    case BIC:  return "BIC";
    case BICB: return "BICB";
    case BIS:  return "BIS";
    case BISB: return "BISB";
    }
    switch (op & ~07) {
    case ASH:  return "ASH";
    case ASHC: return "ASHC";
    case MUL:  return "MUL";
    case DIV:  return "DIV";
    case XOR:  return "XOR";
    }
    switch (op) {
    case CLR:  return "CLR";
    case CLRB: return "CLRB";
    case COM:  return "COM";
    case COMB: return "COMB";
    case INC:  return "INC";
    case INCB: return "INCB";
    case DEC:  return "DEC";
    case DECB: return "DECB";
    case NEG:  return "NEG";
    case NEGB: return "NEGB";
    case TST:  return "TST";
    case TSTB: return "TSTB";
    case ASR:  return "ASR";
    case ASRB: return "ASRB";
    case ASL:  return "ASL";
    case ASLB: return "ASLB";
    case ROR:  return "ROR";
    case RORB: return "RORB";
    case ROL:  return "ROL";
    case ROLB: return "ROLB";
    case SWAB: return "SWAB";
    case ADC:  return "ADC";
    case ADCB: return "ADCB";
    case SBC:  return "SBC";
    case SBCB: return "SBCB";
    case SXT:  return "SXT";
    case MFPS: return "MFPS";
    case MTPS: return "MTPS";
    case INC2: return "INC2";
    case DEC2: return "DEC2";
    }
    return "?OP?";
}

static void show (unsigned op,	// [9:0] op code
	unsigned a,		// [15:0] `source' register
	unsigned b,		// [15:0] `destination' register
	unsigned ps,		// [7:0] processor state register
	unsigned d,		// [15:0] result
	unsigned psr,		// [7:0] processor state result
	unsigned ok)
{
	VL_PRINTF ("(%u) %-4s", main_time, opname (op));
	VL_PRINTF (" %04x, %04x [%u%u%u%u] -> %04x [%u%u%u%u]", a, b,
            ps >> 3 & 1, ps >> 2 & 1, ps >> 1 & 1, ps & 1, d,
            psr >> 3 & 1, psr >> 2 & 1, psr >> 1 & 1, psr & 1);
	if (psr >> 3 & 1)
		VL_PRINTF (" N");
	if (psr >> 2 & 1)
		VL_PRINTF (" Z");
	if (psr >> 1 & 1)
		VL_PRINTF (" V");
	if (psr & 1)
		VL_PRINTF (" C");

	if (ok)
		VL_PRINTF (" - Ok\n");
	else
		VL_PRINTF (" - ERROR\n");
}

struct stimulus {
    unsigned op_in;
    unsigned a_in;
    unsigned b_in;
    unsigned ps_in;
    unsigned delay;
    unsigned d_out;
    unsigned psr_out;
};

struct stimulus tab[] = {
    { CLR,  0x5555, 0x3333, 0,      5,  0,      Z       },
    { CLRB, 0x5555, 0x3333, 0,      5,  0x3300, Z       },
    { COM,  0x5555, 0x3333, 0,      5,  0xcccc, N+C	},
    { COMB, 0x5555, 0x3333, 0,      5,  0x33cc, N+C	},
    { INC,  0x5555, 0x7fff, 0,      5,  0x8000, N+V	},
    { INC,  0x5555, 0xffff, 0,      5,  0,      Z	},
    { INCB, 0x5555, 0x33ff, 0,      5,  0x3300, Z	},
    { INC2, 0x5555, 0x3fff, N+Z+V+C,5,  0x4001, N+Z+V+C	},
    { DEC,  0x5555, 0x8000, 0,      5,  0x7fff, V	},
    { DECB, 0x5555, 0x3300, 0,      5,  0x33ff, N	},
    { DEC2, 0x5555, 0x4001, N+Z+V+C,5,  0x3fff, N+Z+V+C	},
    { NEG,  0x5555, 0x7fff, 0,      5,  0x8001, N+C	},
    { NEGB, 0x5555, 0x7fff, 0,      5,  0x7f01, C	},
    { TST,  0x5555, 0xaaaa, 0,      5,  0xaaaa, N	},
    { TSTB, 0x5555, 0xaa00, 0,      5,  0xaa00, Z	},
    { ASR,  0x3333, 0xaa55, 0,      5,  0xd52a, N+C	},
    { ASRB, 0x3333, 0xaa55, 0,      5,  0xaa2a, V+C	},
    { ASL,  0x3333, 0xaa55, 0,      5,  0x54aa, V+C	},
    { ASLB, 0x3333, 0xaa55, 0,      5,  0xaaaa, N+V	},
    { ROR,  0x3333, 0xaa55, 0,      5,  0x552a, V+C	},
    { RORB, 0x3333, 0xaa55, 0,      5,  0xaa2a, V+C	},
    { ROL,  0x3333, 0xaa55, 0,      5,  0x54aa, V+C	},
    { ROLB, 0x3333, 0xaa55, 0,      5,  0xaaaa, N+V	},
    { SWAB, 0x3333, 0xaa55, N+Z+V+C,5,  0x55aa, 0	},
    { ADC,  0x3333, 0x7fff, N+Z+V+C,5,  0x8000, N+V	},
    { ADCB, 0x3333, 0x7fff, N+Z+V+C,5,  0x7f00, Z+C	},
    { SBC,  0x3333, 0x8000, N+Z+V+C,5,  0x7fff, V	},
    { SBCB, 0x3333, 0x8000, N+Z+V+C,5,  0x80ff, N+C	},
    { SXT,  0x3333, 0x8000, N+Z+V+C,5,  0xffff, N+C	},
    { SXT,  0x3333, 0x8000, 0,      5,  0x0000, Z	},
    { MFPS, 0x3333, 0x8000, N+V,    5,  N+V,    0	},
    { MTPS, 0x3333, 0x00ff, N+V,    5,  0x00ff, 0xef	},

    // Double-operand instructions.
    { MOV,  0x3333, 0x7fff, N+Z+V+C,5,  0x3333, C       },
    { MOVB, 0x3333, 0xaaaa, N+Z+V+C,5,  0xaa33, C	},
    { CMP,  0x3333, 0x4444, N+Z+V+C,5,  0xeeef, N	},
    { CMPB, 0x3333, 0x4444, N+Z+V+C,5,  0x44ef, N	},
    { ADD,  0x9999, 0x8888, N+Z+V+C,5,  0x2221, V+C	},
    { SUB,  0x9999, 0x8888, N+Z+V+C,5,  0xeeef, N	},
    { ASH,  0x0007, 0x9999, N+Z+V+C,5,  0x0133, V	},
    { ASH,  0x0039, 0x9999, N+Z+V+C,5,  0xcc80, N	},
    { BIT,  0x1234, 0x5678, N+Z+V+C,5,  0x1230, C	},
    { BITB, 0x1234, 0x5678, N+Z+V+C,5,  0x5630, C	},
    { BIC,  0x1234, 0x5678, N+Z+V+C,5,  0x4448, C	},
    { BICB, 0x1234, 0x5678, N+Z+V+C,5,  0x5648, C	},
    { BIS,  0x2134, 0x5678, N+Z+V+C,5,  0x777c, C	},
    { BISB, 0x2134, 0x5678, N+Z+V+C,5,  0x567c, C	},
    { XOR,  0x1234, 0x5678, N+Z+V+C,5,  0x444c, C	},

    { 0,    0,      0,      0,      0,  0,      0       },
};

int main (int argc, char **argv)
{
    alu = new Valu;		// Create instance of module

    Verilated::commandArgs (argc, argv);
    Verilated::debug (0);

    alu->op = 0,                // Initialize inputs
    alu->a = 0;
    alu->b = 0;
    alu->ps = 0;

    // Wait for global reset to finish
    delay (10);
    VL_PRINTF ("(%u) started ALU testing\n", main_time);

    struct stimulus *s;
    for (s=tab; s->delay != 0; s++) {
        alu->op = s->op_in;
        alu->a = s->a_in;
        alu->b = s->b_in;
        alu->ps = s->ps_in;
        delay (s->delay);
        show (alu->op, alu->a, alu->b, alu->ps, alu->d, alu->psr,
            (alu->d == s->d_out && alu->psr == s->psr_out));
    }
    delay (5);
    VL_PRINTF ("(%u) finished ALU testing\n", main_time);
    alu->final();

    VL_PRINTF ("All Tests passed\n");
    return 0;
}
