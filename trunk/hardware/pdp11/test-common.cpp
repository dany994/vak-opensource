#include <verilated.h>		// Defines common routines
#include "opcode.h"		// From Verilating "alu.v"

unsigned main_time = 0;         // Current simulation time

double sc_time_stamp ()
{                               // Called by $time in Verilog
    return main_time;
}

const char *opname (unsigned op)
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
