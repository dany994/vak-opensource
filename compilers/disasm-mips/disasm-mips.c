/* disassemble.cpp. Opcode disassembly routines.
   Copyright 2008-2010 Free Software Foundation, Inc.
   Contributed by MIPS Technologies, Inc.

   This file is part of mipsdis.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <stdio.h>
#include <string.h>

#if !defined(_MSC_VER) || (__STDC_VERSION__ >= 199901L)
#include <inttypes.h>
#else
/*
 * typedef's for integers with specific bit-widths.
 */
typedef unsigned __int64   uint64_t;
typedef   signed __int64    int64_t;

typedef unsigned int       uint32_t;
typedef   signed int        int32_t;

typedef unsigned short int uint16_t;
typedef   signed short int  int16_t;

typedef unsigned char       uint8_t;
typedef   signed char        int8_t;
#endif

static const uint64_t M_64ALL = ~0ULL;

#ifndef MIPS_Model64
#define MIPS_T_BaseData  uint32_t
#else
#define MIPS_T_BaseData  uint64_t
#endif

extern const char* disasm_mips (uint32_t opcode, uint32_t pc);
extern const char* disasm_mips16 (uint32_t opcode, uint32_t pc);

#define MIPS16_EXT_OPCODE 0xf8000000
#define MIPS16_EXTEND	0xf0000000
#define MIPS16_JAL	0x18000000
#define IS_EXTENDED(OPCODE) ((((OPCODE) & MIPS16_EXT_OPCODE) == MIPS16_EXTEND) || \
                             (((OPCODE) & MIPS16_EXT_OPCODE) == MIPS16_JAL))
#define MIPS16_INSTR_INDEX(OPCODE) (( (((OPCODE) & MIPS16_JAL) == MIPS16_JAL ) ? \
                              ((OPCODE) >> 27) : ((OPCODE) >> 11) ) & 0x1f  )

//extern int SymNames;       // if set, disass prints sym register names
//extern int Cop0SymNames;   // if set, disass prints sym register names for Cop0

static const int SymNames = 0;
static const int Cop0SymNames = 1;

static const uint32_t BITMASK1_0  = 0x3;
static const uint32_t BITMASK2_0  = 0x7;
static const uint32_t BITMASK3_0  = 0xf;
static const uint32_t BITMASK4_0  = 0x1f;
static const uint32_t BITMASK5_0  = 0x3f;
static const uint32_t BITMASK6_0  = 0x7f;
static const uint32_t BITMASK7_0  = 0xff;
static const uint32_t BITMASK8_0  = 0x1ff;
static const uint32_t BITMASK9_0  = 0x3ff;
static const uint32_t BITMASK10_0 = 0x7ff;
static const uint32_t BITMASK11_0 = 0xfff;
static const uint32_t BITMASK12_0 = 0x1fff;
static const uint32_t BITMASK13_0 = 0x3fff;
static const uint32_t BITMASK14_0 = 0x7fff;
static const uint32_t BITMASK15_0 = 0xffff;


enum format {
    FUN, DST, DTS, DIB, TSO, CO, STO, SI, SO, O, SOB, TSPS, TSSA,
    TOS, OS, B,   TO,  CTD, CF, ST,  DorTS, J, S, DS, D, DTA, T, TD, DT, BRK, X,

    // DSP specific
    TS, DI, DTSA, ACST, TACSA, TACS, SAC, ACS, ACSA, AC, MTAC, MFAC, DTI, SDI,

    // MT specific
    MFTAC, MTTAC, CFDT, CTTD,

    C1FDST, C1FDSGT, C1FDS, C1FCCST, C1DIB, C1SIB, C1TOB, C1HIB, C1DSTS,
    C1DRST, C1DS, C1DST, C1BCCO, C1FDSCC, C1GRDSCC,
    C1CVTS, C1CVTW, C1CVTPS,

    UDISPC2,
    RES, RESSPC, RESREG, RESCRS, RESCRT, RESSPC2, RESSPC3,

    M16_R, M16_RR, M16_RR_YX, M16_CNVT,
    M16_RRR, M16_RI, M16_BRI, M16_RI64, M16_RRI, M16_RRI_YOX,
    M16_RRIA,
    M16_I, M16_I8, M16_BI8, M16_SAVRES, M16_I64,
    M16_JAL,M16_JALR,
    M16_SR, M16_DSHIFT, M16_MOV32R, M16_MOV32,
    M16_RAC

#ifdef MIPS_MDMX
    , MDMX, MDMX_AC, MDMX_ALNI, MDMX_ALNV, MDMX_C, MDMX_R, MDMX_RAC,
    MDMX_SAC, MDMX_SHFL, MDMX_WAC, RESMDMX
#endif // MIPS_MDMX
};

struct iType {
    enum format fmt;
    union {
        char         *name;
        struct iType *(*f)(uint32_t );
    } n;
};

/*
 * Function Prototypes
 */
static const struct iType *DecodeSpecial(uint32_t opc);
static const struct iType *DecodeRegImm(uint32_t opc);
static const struct iType *DecodeRegImmAtomic(uint32_t opc);
static const struct iType *DecodeCopRs(uint32_t);
static const struct iType *DecodeCopRsOp(uint32_t opcode);
static const struct iType *DecodeMFMC0(uint32_t opcode);
static const struct iType *DecodeMTTR(uint32_t opcode);
static const struct iType *DecodeMFTR(uint32_t opcode);
static const struct iType *DecodeCopRt(uint32_t);
static const struct iType *DecodeCopOp(uint32_t);
static const struct iType *DecodeSLL(uint32_t);
static const struct iType *DecodeBEQ(uint32_t);
static const struct iType *DecodeSpec2(uint32_t opc);
static const struct iType *DecodeSpec3(uint32_t opc);
static const struct iType *DecodeBSHFL(uint32_t opc);
static const struct iType *DecodeJR(uint32_t opc);
static const struct iType *DecodeJALR(uint32_t opc);

static const struct iType *DecodeDBSHFL(uint32_t opc);
static const struct iType *DecodeCop1X(uint32_t opc);
static const struct iType *DecodeMovCond(uint32_t opcode);

// MIPS16 decode routines
static const struct iType *DecodeMIPS16_SHIFT(uint32_t opcode);
static const struct iType *DecodeMIPS16_RRIA(uint32_t opcode);
static const struct iType *DecodeMIPS16_RRR(uint32_t opcode);
static const struct iType *DecodeMIPS16_I8(uint32_t opcode);
static const struct iType *DecodeMIPS16_I64(uint32_t opcode);
static const struct iType *DecodeMIPS16_RR(uint32_t opcode);

// MIPS Crypto decode routines
static const struct iType *DecodeMADDU(uint32_t opcode);
static const struct iType *DecodeMFLO(uint32_t opcode);
static const struct iType *DecodeMTLO(uint32_t opcode);
static const struct iType *DecodeMULTU(uint32_t opcode);

// MIPS Crypto or Release 2 routines
static const struct iType *DecodeSRL(uint32_t opcode);
static const struct iType *DecodeSRLV(uint32_t opcode);
static const struct iType *DecodeDSRL(uint32_t opcode);
static const struct iType *DecodeDSRLV(uint32_t opcode);
static const struct iType *DecodeDSRL32(uint32_t opcode);

// MIPS DSP routine
static const struct iType *DecodeLX(uint32_t opcode);
static const struct iType *DecodeADDUQB(uint32_t opcode);
//static const struct iType *DecodeADDUOB(uint32_t opcode);
static const struct iType *DecodeCMPEQQB(uint32_t opcode);
//static const struct iType *DecodeCMPEQOB(uint32_t opcode);
static const struct iType *DecodeABSQPH(uint32_t opcode);
//static const struct iType *DecodeABSQQH(uint32_t opcode);
static const struct iType *DecodeSHLLQB(uint32_t opcode);
//static const struct iType *DecodeSHLLOB(uint32_t opcode);
static const struct iType *DecodeDPAQWPH(uint32_t opcode);
static const struct iType *DecodeDPAQWQH(uint32_t opcode);
static const struct iType *DecodeEXTRW(uint32_t opcode);
//static const struct iType *DecodeDEXTRW(uint32_t opcode);
static const struct iType *DecodeADDUHQB(uint32_t opcode);
static const struct iType *DecodeAPPEND(uint32_t opcode);

#ifdef MIPS_MDMX
// MDMX decode routines
static const struct iType  * DecodeMDMX(uint32_t opcode);
#endif // MIPS_MDMX

#define SIGNEXTEND(d, n) if ((d&(1<<((n)-1)))!=0) d |= (MIPS_T_BaseData)( M_64ALL << (n))

/*
 * Tables to drive disassembly
 */

/////////////////////////////////////////////////////////////////
//  MIPS16 main opcode table

static const struct iType M16Table[32] =
{
    { M16_RI,      {"ADDIUSP"}},
    { M16_RI,      {"ADDIUPC"}},
    { M16_I,       {"B"}},
    { M16_JAL,     {"JAL"}},
    { M16_BRI,     {"BEQZ"}},
    { M16_BRI,     { "BNEZ"}},
    { FUN,         {(char *)DecodeMIPS16_SHIFT}},
    { M16_RRI_YOX, {"LD"}},

    { FUN,         {(char *)DecodeMIPS16_RRIA}},
    { M16_RI ,     {"ADDIU8"}},
    { M16_RI,      {"SLTI"}},
    { M16_RI,      {"SLTIU"}},
    { FUN,         {(char *)DecodeMIPS16_I8 }},
    { M16_RI,      {"LI"}},
    { M16_RI,      {"CMPI"}},
    { M16_RRI_YOX, {"SD"}},

    { M16_RRI_YOX, {"LB"}},
    { M16_RRI_YOX, {"LH"}},
    { M16_RI,      {"LWSP"}},
    { M16_RRI_YOX, {"LW"}},
    { M16_RRI_YOX, {"LBU"}},
    { M16_RRI_YOX, {"LHU"}},
    { M16_RI,      {"LWPC"}},
    { M16_RRI_YOX, {"LWU"}},

    { M16_RRI_YOX, {"SB"}},
    { M16_RRI_YOX, {"SH"}},
    { M16_RI,      {"SWSP"}},
    { M16_RRI_YOX, {"SW"}},
    { FUN,         {(char *)DecodeMIPS16_RRR}},
    { FUN,         {(char *)DecodeMIPS16_RR}},
    { RES,         {NULL}},
    { FUN,         {(char *)DecodeMIPS16_I64}}
};

    // function table for RR typeinstructions
static const struct iType M16RRTable[32] =
{
    { M16_JALR,  {"J(AL)R"}},
    { RES,       {NULL}},
    { M16_RR,    {"SLT"}},
    { M16_RR,    {"SLTU"}},
    { M16_RR_YX, {"SLLV"}},
    { BRK,       {"BREAK"}},
    { M16_RR_YX, {"SRLV"}},
    { M16_RR_YX, {"SRAV"}},

    { M16_SR,    {"DSRL"}},
    { RES,       {NULL}},
    { M16_RR,    {"CMP"}},
    { M16_RR,    {"NEG"}},
    { M16_RR,    {"AND"}},
    { M16_RR,    {"OR"}},
    { M16_RR,    {"XOR"}},
    { M16_RR,    {"NOT"}},

    { M16_RAC,   {"MFHI"}},
    { M16_CNVT,  {"CNVT"}},
    { M16_RAC,   {"MFLO"}},
    { M16_SR,    {"DSRA"}},
    { M16_RR,    {"DSLLV"}},
    { RES,       {NULL}},
    { M16_RR_YX, {"DSRLV"}},
    { M16_RR_YX, {"DSRAV"}},

    { M16_RR,    {"MULT"}},
    { M16_RR,    {"MULTU"}},
    { M16_RR,    {"DIV"}},
    { M16_RR,    {"DIVU"}},
    { M16_RR,    {"DMULT"}},
    { M16_RR,    {"DMULTU"}},
    { M16_RR,    {"DDIV"}},
    { M16_RR,    {"DDIVU"}}
} ;

static const struct iType M16ShiftTable[4] =
{
    { M16_DSHIFT, {"SLL"}},
    { M16_DSHIFT, {"DSLL"}},
    { M16_DSHIFT, {"SRL"}},
    { M16_DSHIFT, {"SRA"}}
} ;

static const struct iType M16RRIATable[2] =
{
    { M16_RRIA, {"ADDIU"}},
    { M16_RRIA, {"DADDIU"}}
};

static const struct iType M16RRRTable[4] =
{
    { M16_RRR, {"DADDU"}},
    { M16_RRR, {"ADDU"}},
    { M16_RRR, {"DSUBU"}},
    { M16_RRR, {"SUBU"}}
};

static const struct iType M16I8Table[8] =
{
    { M16_BI8,    {"BTEQZ"}},
    { M16_BI8,    {"BTNEZ"}},
    { M16_I8,     {"SWRASP"}},
    { M16_I8,     {"ADDJSP"}},
    { M16_SAVRES, {"SAVE/RESTORE"}},
    { M16_MOV32R, {"MOVE"}},
    { RES,        {NULL}},
    { M16_MOV32,  {"MOVE"}}
} ;

static const struct iType M16I64Table[8] =
{
      { M16_RI64, {"LDSP"}},
      { M16_RI64, {"SDSP"}},
      { M16_RI64, {"SDRASP"}},
      { M16_I64,  {"DADJSP"}},
      { M16_RI64, {"LDPC"}},
      { M16_RI64, {"DADDIU"}},
      { M16_RI64, {"DADDIUPC"}},
      { M16_RI64, {"DADDIUSP"}}
};

/////////////////////////////////////////
// Main decode table

/////////////////////////////////////////
static const struct iType MainTable[64] = {
  { FUN, {(char *)DecodeSpecial}},
  { FUN, {(char *)DecodeRegImm} },
  { J,   {"J"} },
  { J,   {"JAL"} },
  { FUN, {(char *)DecodeBEQ} }, // { STO, "BEQ" },
  { STO, {"BNE"} },
  { SO,  {"BLEZ"} },
  { SO,  {"BGTZ"} },

  { TSO, {"ADDI"} },
  { TSO, {"ADDIU"} },
  { TSO, {"SLTI"} },
  { TSO, {"SLTIU"} },
  { TSO, {"ANDI"} },
  { TSO, {"ORI "} },
  { TSO, {"XORI"} },
  { TO,  {"LUI"} },

  { FUN, {(char *)DecodeCopRs} },
  { FUN, {(char *)DecodeCopRs} },
  { FUN, {(char *)DecodeCopRs} },
  { FUN, {(char *)DecodeCop1X} },
  { STO, {"BEQL"} },
  { STO, {"BNEL"} },
  { SO,  {"BLEZL"} },
  { SO,  {"BGTZL"} },

  { TSO,     {"DADDI"}  },
  { TSO,     {"DADDIU"} },
  { TOS,     {"LDL"}    },
  { TOS,     {"LDR"}    },
  { FUN,     {(char *)DecodeSpec2} },
  { M16_JAL, {"JALX"} },
#ifdef MIPS_MDMX
  { FUN,     {(char *)DecodeMDMX} },
#else
  { RES,     {NULL} },
#endif // MIPS_MDMX
  { FUN,     {(char *)DecodeSpec3} },
  { TOS,     {"LB"} },
  { TOS,     {"LH"} },
  { TOS,     {"LWL"} },
  { TOS,     {"LW"} },
  { TOS,     {"LBU"} },
  { TOS,     {"LHU"} },
  { TOS,     {"LWR"} },
  { TOS,     {"LWU"} },

  { TOS, {"SB"} },
  { TOS, {"SH"} },
  { TOS, {"SWL"} },
  { TOS, {"SW"} },
  { TOS, {"SDL"} },
  { TOS, {"SDR"} },
  { TOS, {"SWR"} },
  { TOS, {"CACHE"} },

  { TOS,   {"LL"} },
  { C1TOB, {"LWC1"} },
  { TOS,   {"LWC2"} },
  { TOS,   {"PREF"} },
  { TOS,   {"LLD"}  },
  { C1TOB, {"LDC1"} },
  { TOS,   {"LDC2"} },
  { TOS,   {"LD"}   },

  { TOS,   {"SC"} },
  { C1TOB, {"SWC1"} },
  { TOS,   {"SWC2"} },
  { TOS,   {"SWC3"} },
  { TOS,   {"SCD"}  },
  { C1TOB, {"SDC1"} },
  { TOS,   {"SDC2"} },
  { TOS,   {"SD"}   }

};

static const struct iType SpecialTable[64] = {
  { FUN,    {(char*)DecodeSLL} }, //{ DTA, "SLL" },
  { FUN,    {(char*)DecodeMovCond} },
  { FUN,    {(char*)DecodeSRL} },
  { DTA,    {"SRA"} },
  { DTS,    {"SLLV"} },
  { RESSPC, {NULL} },
  { FUN,    {(char*)DecodeSRLV} },
  { DTS,    {"SRAV"} },

  { FUN,    {(char *)DecodeJR} },
  { FUN,    {(char *)DecodeJALR} },
  { DST,    {"MOVZ"} },
  { DST,    {"MOVN"} },
  { O,      {"SYSCALL"} },
  { BRK,    {"BREAK"} },
  { RESSPC, {NULL} },
  { O,      {"SYNC"} },

  { MFAC,    {"MFHI"} },
  { MTAC,    {"MTHI"} },
  { FUN,    {(char*)DecodeMFLO} },
  { FUN,    {(char*)DecodeMTLO} },
  { DTS,    {"DSLLV"} },
  { RESSPC, {NULL} },
  { FUN,    {(char*)DecodeDSRLV} },
  { DTS,    {"DSRAV"} },

  { ST,  {"MULT"} },
  { FUN, {(char*)DecodeMULTU} },
  { ST,  {"DIV"} },
  { ST,  {"DIVU"} },
  { ST,  {"DMULT"}  },
  { ST,  {"DMULTU"} },
  { ST,  {"DDIV"}   },
  { ST,  {"DDIVU"}  },

  { DST, {"ADD"} },
  { DST, {"ADDU"} },
  { DST, {"SUB"} },
  { DST, {"SUBU"} },
  { DST, {"AND"} },
  { DST, {"OR"} },
  { DST, {"XOR"} },
  { DST, {"NOR"} },

  { RESSPC, {NULL} },
  { RESSPC, {NULL} },
  { DST,    {"SLT"} },
  { DST,    {"SLTU"} },
  { DST,    {"DADD"}  },
  { DST,    {"DADDU"} },
  { DST,    {"DSUB"}  },
  { DST,    {"DSUBU"} },

  { ST,     {"TGE"} },
  { ST,     {"TGEU"} },
  { ST,     {"TLT"} },
  { ST,     {"TLTU"} },
  { ST,     {"TEQ"} },
  { RESSPC, {NULL} },
  { ST,     {"TNE"} },
  { RESSPC, {NULL} },

  { DTA,    {"DSLL"}  },
  { RESSPC, {NULL} },
  { FUN,    {(char*)DecodeDSRL} },
  { DTA,    {"DSRA"}  },
  { DTA,    {"DSLL32"}},
  { RESSPC, {NULL} },
  { FUN,    {(char*)DecodeDSRL32} },
  { DTA,    {"DSRA32"}}
};

static const struct iType Spec2Table[64] = {
  { ST,      {"MADD"} },
  { FUN,     {(char*)DecodeMADDU} },
  { DST,     {"MUL"} },
  { RESSPC2, {NULL} },
  { ST,      {"MSUB"}  },
  { ST,      {"MSUBU"} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },

  { DIB,     {"LXS"} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },

  { DS,  {"CNZ"} },
  { DS,  {"CNO"} },
  { D,   {"SWPMFH"} },
  { D,   {"SWPMFL"} },
  { ST,  {"SWPMT"} },
  { ST,  {"SWP"} },
  { ST,  {"SWPACC"} },
  { DST, {"SWPGPR"} },

  { UDISPC2, {"UDI"} },
  { UDISPC2, {"UDI"} },
  { UDISPC2, {"UDI"} },
  { UDISPC2, {"UDI"} },
  { UDISPC2, {"UDI"} },
  { UDISPC2, {"UDI"} },
  { UDISPC2, {"UDI"} },
  { UDISPC2, {"UDI"} },

  { DorTS,   {"CLZ"} },
  { DorTS,   {"CLO"} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { DorTS,   {"DCLZ"} },
  { DorTS,   {"DCLO"} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },

  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },

  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },

  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { RESSPC2, {NULL} },
  { O, {"SDBBP"} }
};

static const struct iType Spec3Table[64] = {
  { TSPS,    {"EXT"} },
  { TSPS,    {"DEXTM"} },
  { TSPS,    {"DEXTU"} },
  { TSPS,    {"DEXT"} },
  { TSPS,    {"INS"} },
  { TSPS,    {"DINSM"} },
  { TSPS,    {"DINSU"} },
  { TSPS,    {"DINS"} },

  { DST,     {"FORK"} },
  { DS,      {"YIELD"} },
  { FUN,     {(char *)DecodeLX} },
  { RESSPC3, {NULL} },
  { TS,      {"INSV"} },
  { TS,      {"DINSV"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { FUN,     {(char *)DecodeADDUQB} },
  { FUN,     {(char *)DecodeCMPEQQB} },
  { FUN,     {(char *)DecodeABSQPH} },
  { FUN,     {(char *)DecodeSHLLQB} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { FUN,     {(char *)DecodeADDUHQB} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { FUN,     {(char *)DecodeBSHFL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { FUN,     {(char *)DecodeDBSHFL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { FUN,     {(char *)DecodeDPAQWPH} },
  { FUN,     {(char *)DecodeAPPEND} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { FUN,     {(char *)DecodeDPAQWQH} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { FUN,     {(char *)DecodeEXTRW} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { TD,      {"RDHWR"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} }
};

static const struct iType AdduQBTable[32] = {
  { DST,     {"ADDU.QB"} },
  { DST,     {"SUBU.QB"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"ADDU_S.QB"} },
  { DST,     {"SUBU_S.QB"} },
  { DST,     {"MULEU_S.PH.QBL"} },
  { DST,     {"MULEU_S.PH.QBR"} },

  { DST,     {"ADDU.PH"} },
  { DST,     {"SUBU.PH"} },
  { DST,     {"ADDQ.PH"} },
  { DST,     {"SUBQ.PH"} },
  { DST,     {"ADDU_S.PH"} },
  { DST,     {"SUBU_S.PH"} },
  { DST,     {"ADDQ_S.PH"} },
  { DST,     {"SUBQ_S.PH"} },

  { DST,     {"ADDSC"} },
  { DST,     {"ADDWC"} },
  { DST,     {"MODSUB"} },
  { RESSPC3, {NULL} },
  { DST,     {"RADDU.W.QB"} },
  { RESSPC3, {NULL} },
  { DST,     {"ADDQ_S.W"} },
  { DST,     {"SUBQ_S.W"} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"MULEQ_S.W.PHL"} },
  { DST,     {"MULEQ_S.W.PHR"} },
  { DST,     {"MULQ_S.PH"} },
  { DST,     {"MULQ_RS.PH"} }
};

#if 0
static const struct iType AdduOBTable[32] = {
  { DST,     {"ADDU.OB"} },
  { DST,     {"SUBU.OB"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"ADDU_S.OB"} },
  { DST,     {"SUBU_S.OB"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"ADDQ.QH"} },
  { DST,     {"SUBQ.QH"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"ADDQ_S.QH"} },
  { DST,     {"SUBQ_S.QH"} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"ADDQ.PW"} },
  { DST,     {"SUBQ.PW"} },
  { DST,     {"RADDU.L.OB"} },
  { RESSPC3, {NULL} },
  { DST,     {"ADDQ_S.PW"} },
  { DST,     {"SUBQ_S.PW"} },

  { DST,     {"MULEQ.PW.QHL"} },
  { DST,     {"MULEQ.PW.QHR"} },
  { RESSPC3, {NULL} },
  { DST,     {"MULQ.QH"} },
  { DST,     {"MULEQ_S.PW.QHL"} },
  { DST,     {"MULEQ_S.PW.QHR"} },
  { RESSPC3, {NULL} },
  { DST,     {"MULQ_RS.QH"} }
};
#endif

static const struct iType CmpEqQBTable[32] = {
  { ST,      {"CMPU.EQ.QB"} },
  { ST,      {"CMPU.LT.QB"} },
  { ST,      {"CMPU.LE.QB"} },
  { DST,     {"PICK.QB"} },
  { DST,     {"CMPGU.EQ.QB"} },
  { DST,     {"CMPGU.LT.QB"} },
  { DST,     {"CMPGU.LE.QB"} },
  { RESSPC3, {NULL} },

  { ST,      {"CMP.EQ.PH"} },
  { ST,      {"CMP.LT.PH"} },
  { ST,      {"CMP.LE.PH"} },
  { DST,     {"PICK.PH"} },
  { DST,     {"PRECRQ.QB.PH"} },
  { DST,     {"PRECR.QB.PH"} },
  { DST,     {"PACKRL.PH"} },
  { DST,     {"PRECRQU_S.QB.PH"} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"PRECRQ.PH.W"} },
  { DST,     {"PRECRQ_RS.PH.W"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { DST,     {"CMPGDU.EQ.QB"} },
  { DST,     {"CMPGDU.LT.QB"} },
  { DST,     {"CMPGDU.LE.QB"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"PRECR_SRA.PH.W"} },
  { DST,     {"PRECR_SRA_R.PH.W"} }
};

#if 0
static const struct iType CmpEqOBTable[32] = {
  { ST,      {"CMPU.EQ.OB"} },
  { ST,      {"CMPU.LT.OB"} },
  { ST,      {"CMPU.LE.OB"} },
  { DST,     {"PICK.OB"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { ST,      {"CMP.EQ.QH"} },
  { ST,      {"CMP.LT.QH"} },
  { ST,      {"CMP.LE.QH"} },
  { DST,     {"PICK.QH"} },
  { DST,     {"PRECRQ.OB.QH"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"PRECRQU_S.OB.QH"} },

  { ST,      {"CMP.EQ.PW"} },
  { ST,      {"CMP.LT.PW"} },
  { ST,      {"CMP.LE.PW"} },
  { DST,     {"PICK.PW"} },
  { DST,     {"PRECRQ.QH.PW"} },
  { DST,     {"PRECRQ_RS.QH.PW"} },
  { RESSPC3, {NULL} },
  { DST,     {"PRECRQU_S.QH.PW"} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"PRECRQ.PW.L"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"PRECRQU_S.PW.L"} }
};
#endif

static const struct iType AbsqPHTable[32] = {
  { RESSPC3, {NULL} },
  { DT,      {"ABSQ_S.QB"} },
  { DI,      {"REPL.QB"} },
  { DT,      {"REPLV.QB"} },
  { DT,      {"PRECEQU.PH.QBL"} },
  { DT,      {"PRECEQU.PH.QBR"} },
  { DT,      {"PRECEQU.PH.QBLA"} },
  { DT,      {"PRECEQU.PH.QBRA"} },

  { DT,      {"ABSQ.PH"} },
  { DT,      {"ABSQ_S.PH"} },
  { DI,      {"REPL.PH"} },
  { DT,      {"REPLV.PH"} },
  { DT,      {"PRECEQ.W.PHL"} },
  { DT,      {"PRECEQ.W.PHR"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { RESSPC3, {NULL} },
  { DT,      {"ABSQ_S.W"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DT,      {"BITREV"} },
  { DT,      {"PRECEU.PH.QBL"} },
  { DT,      {"PRECEU.PH.QBR"} },
  { DT,      {"PRECEU.PH.QBLA"} },
  { DT,      {"PRECEU.PH.QBRA"} }
};

#if 0
static const struct iType AbsqQHTable[32] = {
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DI,      {"REPL.OB"} },
  { DT,      {"REPLV.OB"} },
  { DT,      {"PRECEQU.QH.OBL"} },
  { DT,      {"PRECEQU.QH.OBR"} },
  { DT,      {"PRECEQU.QH.OBLA"} },
  { DT,      {"PRECEQU.QH.OBRA"} },

  { DT,      {"ABSQ.QH"} },
  { DT,      {"ABSQ_S.QH"} },
  { DI,      {"REPL.QH"} },
  { DT,      {"REPLV.QH"} },
  { DT,      {"PRECEQ.PW.QHL"} },
  { DT,      {"PRECEQ.PW.QHR"} },
  { DT,      {"PRECEQ.PW.QHLA"} },
  { DT,      {"PRECEQ.PW.QHRA"} },

  { DT,      {"ABSQ.PW"} },
  { DT,      {"ABSQ_S.PW"} },
  { DI,      {"REPL.PW"} },
  { DT,      {"REPLV.PW"} },
  { DT,      {"PRECEQ.L.PWL"} },
  { DT,      {"PRECEQ.L.PWR"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} }
};
#endif

static const struct iType ShllQBTable[32] = {
  { DTSA,    {"SHLL.QB"} },
  { DTSA,    {"SHRL.QB"} },
  { DTS,     {"SHLLV.QB"} },
  { DTS,     {"SHRLV.QB"} },
  { DTSA,    {"SHRA.QB"} },
  { DTSA,    {"SHRA_R.QB"} },
  { DTS,     {"SHRAV.QB"} },
  { DTS,     {"SHRAV_R.QB"} },

  { DTSA,    {"SHLL.PH"} },
  { DTSA,    {"SHRA.PH"} },
  { DTS,     {"SHLLV.PH"} },
  { DTS,     {"SHRAV.PH"} },
  { DTSA,    {"SHLL_S.PH"} },
  { DTSA,    {"SHRA_R.PH"} },
  { DTS,     {"SHLLV_S.PH"} },
  { DTS,     {"SHRAV_R.PH"} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DTSA,    {"SHLL_S.W"} },
  { DTSA,    {"SHRA_R.W"} },
  { DTS,     {"SHLLV_S.W"} },
  { DTS,     {"SHRAV_R.W"} },

  { RESSPC3, {NULL} },
  { DTSA,    {"SHRL.PH"} },
  { RESSPC3, {NULL} },
  { DTS,     {"SHRLV.PH"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} }
};

#if 0
static const struct iType ShllOBTable[32] = {
  { DTSA,    {"SHLL.OB"} },
  { DTSA,    {"SHRL.OB"} },
  { DTS,     {"SHLLV.OB"} },
  { DTS,     {"SHRLV.OB"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { DTSA,    {"SHLL.QH"} },
  { DTSA,    {"SHRA.QH"} },
  { DTS,     {"SHLLV.QH"} },
  { DTS,     {"SHRAV.QH"} },
  { DTSA,    {"SHLL_S.QH"} },
  { DTSA,    {"SHRA_R.QH"} },
  { DTS,     {"SHLLV_S.QH"} },
  { DTS,     {"SHRAV_R.QH"} },

  { DTSA,    {"SHLL.PW"} },
  { DTSA,    {"SHRA.PW"} },
  { DTS,     {"SHLLV.PW"} },
  { DTS,     {"SHRAV.PW"} },
  { DTSA,    {"SHLL_S.PW"} },
  { DTSA,    {"SHRA_R.PW"} },
  { DTS,     {"SHLLV_S.PW"} },
  { DTS,     {"SHRAV_R.PW"} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} }
};
#endif

static const struct iType DpaqWPHTable[32] = {
  { ACST,    {"DPA.W.PH"} },
  { ACST,    {"DPS.W.PH"} },
  { ACST,    {"MULSA.W.PH"} },
  { ACST,    {"DPAU.H.QBL"} },
  { ACST,    {"DPAQ_S.W.PH"} },
  { ACST,    {"DPSQ_S.W.PH"} },
  { ACST,    {"MULSAQ_S.W.PH"} },
  { ACST,    {"DPAU.H.QBR"} },

  { ACST,    {"DPAX.W.PH"} },
  { ACST,    {"DPSX.W.PH"} },
  { RESSPC3, {NULL} },
  { ACST,    {"DPSU.H.QBL"} },
  { ACST,    {"DPAQ_SA.L.W"} },
  { ACST,    {"DPSQ_SA.L.W"} },
  { RESSPC3, {NULL} },
  { ACST,    {"DPSU.H.QBR"} },

  { ACST,    {"MAQ_SA.W.PHL"} },
  { RESSPC3, {NULL} },
  { ACST,    {"MAQ_SA.W.PHR"} },
  { RESSPC3, {NULL} },
  { ACST,    {"MAQ_S.W.PHL"} },
  { RESSPC3, {NULL} },
  { ACST,    {"MAQ_S.W.PHR"} },
  { RESSPC3, {NULL} },

  { ACST,    {"DPAQX_S.W.PH"} },
  { ACST,    {"DPSQX_S.W.PH"} },
  { ACST,    {"DPAQX_SA.W.PH"} },
  { ACST,    {"DPSQX_SA.W.PH"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} }
};

static const struct iType DpaqWQHTable[32] = {
  { ACST,    {"DPAQ.W.QH"} },
  { ACST,    {"DPSQ.W.QH"} },
  { ACST,    {"MULSAQ.W.QH"} },
  { RESSPC3, {NULL} },
  { ACST,    {"DPAQ_S.W.QH"} },
  { ACST,    {"DPSQ_S.W.QH"} },
  { ACST,    {"MULSAQ_S.W.QH"} },
  { RESSPC3, {NULL} },

  { ACST,    {"DPAQ.L.PW"} },
  { ACST,    {"DPSQ.L.PW"} },
  { ACST,    {"MULSAQ.L.PW"} },
  { RESSPC3, {NULL} },
  { ACST,    {"DPAQ_SA.L.PW"} },
  { ACST,    {"DPSQ_SA.L.PW"} },
  { ACST,    {"MULSAQ_S.L.PW"} },
  { RESSPC3, {NULL} },

  { ACST,    {"MAQ.W.QHLL"} },
  { ACST,    {"MAQ.W.QHLR"} },
  { ACST,    {"MAQ.W.QHRL"} },
  { ACST,    {"MAQ.W.QHRR"} },
  { ACST,    {"MAQ_S.W.QHLL"} },
  { ACST,    {"MAQ_S.W.QHLR"} },
  { ACST,    {"MAQ_S.W.QHRL"} },
  { ACST,    {"MAQ_S.W.QHRR"} },

  { ACST,    {"MAQ.L.PWL"} },
  { ST,      {"DMADD"} },
  { ACST,    {"MAQ.L.PWR"} },
  { ST,      {"DMSUB"} },
  { ACST,    {"MAQ_S.L.PWL"} },
  { ST,      {"DMADDU"} },
  { ACST,    {"MAQ_S.L.PWR"} },
  { ST,      {"DMSUBU"} }
};

static struct iType ExtrWTable[32] = {
  { TACSA,   {"EXTR.W"} },
  { TACS,    {"EXTRV.W"} },
  { TACSA,   {"EXTP"} },
  { TACS,    {"EXTPV"} },
  { TACSA,   {"EXTR_R.W"} },
  { TACS,    {"EXTRV_R.W"} },
  { TACSA,   {"EXTR_RS.W"} },
  { TACS,    {"EXTRV_RS.W"} },

  { TACSA,   {"EXTL.W"} },
  { TACS,    {"EXTLV.W"} },
  { TACSA,   {"EXTPDP"} },
  { TACS,    {"EXTPDPV"} },
  { TACSA,   {"EXTL_S.W"} },
  { TACS,    {"EXTLV_S.W"} },
  { TACSA,   {"EXTR_S.H"} },
  { TACS,    {"EXTRV_S.H"} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DTI,     {"RDDSP"} },
  { SDI,     {"WRDSP"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { ACSA,    {"SHILO"} },
  { ACS,     {"SHILOV"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { SAC,     {"MTHLIP"} }
};

#if 0
static const struct iType DExtrWTable[32] = {
  { TACSA,   {"DEXTR.W"} },
  { TACS,    {"DEXTRV.W"} },
  { TACSA,   {"DEXTP"} },
  { TACS,    {"DEXTPV"} },
  { TACSA,   {"DEXTR_R.W"} },
  { TACS,    {"DEXTRV_R.W"} },
  { TACSA,   {"DEXTR_RS.W"} },
  { TACS,    {"DEXTRV_RS.W"} },

  { TACSA,   {"DEXTL.W"} },
  { TACS,    {"DEXTLV.W"} },
  { TACSA,   {"DEXTPDP"} },
  { TACS,    {"DEXTPDPV"} },
  { TACSA,   {"DEXTL_S.W"} },
  { TACS,    {"DEXTLV_S.W"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { TACSA,   {"DEXTL.L"} },
  { TACS,    {"DEXTLV.L"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { TACSA,   {"DEXTR_R.L"} },
  { TACS,    {"DEXTRV_R.L"} },
  { TACSA,   {"DEXTR_RS.L"} },
  { TACS,    {"DEXTRV_RS.L"} },

  { TACSA,   {"DEXTL.L"} },
  { TACS,    {"DEXTLV.L"} },
  { ACSA,    {"DSHILO"} },
  { ACS,     {"DSHILOV"} },
  { TACSA,   {"DEXTL_S.L"} },
  { TACS,    {"DEXTLV_S.L"} },
  { RESSPC3, {NULL} },
  { AC,      {"DMTHLIP"} }
};
#endif

static const struct iType AdduhQBTable[32] = {

  { DST,     {"ADDUH.QB"} },
  { DST,     {"SUBUH.QB"} },
  { DST,     {"ADDUH_R.QB"} },
  { DST,     {"SUBUH_R.QB"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { DST,     {"ADDQH.PH"} },
  { DST,     {"SUBQH.PH"} },
  { DST,     {"ADDQH_R.PH"} },
  { DST,     {"SUBQH_R.PH"} },
  { DST,     {"MUL.PH"} },
  { RESSPC3, {NULL} },
  { DST,     {"MUL_S.PH"} },
  { RESSPC3, {NULL} },

  { DST,     {"ADDQH.W"} },
  { DST,     {"SUBQH.W"} },
  { DST,     {"ADDQH_R.W"} },
  { DST,     {"SUBQH_R.W"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { DST,     {"MULQ_S.W"} },
  { DST,     {"MULQ_RS.W"} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} }

};

static const struct iType AppendTable[32] = {

  { TSSA,    {"APPEND"} },
  { TSSA,    {"PREPEND"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { TSSA,    {"BALIGN"} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },

  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} },
  { RESSPC3, {NULL} }

};

static const struct iType RegImmAtomicTable[2] = {
  { SOB, {"ACLR"} },
  { SOB, {"ASET"} }
};

static const struct iType RegImmTable[32] = {

  { SO, {"BLTZ"} },
  { SO, {"BGEZ"} },
  { SO, {"BLTZL"} },
  { SO, {"BGEZL"} },
  { RESREG, {NULL} },
  { RESREG, {NULL} },
  { RESREG, {NULL} },
  { FUN, {(char *)DecodeRegImmAtomic} },

  { SI, {"TGEI"} },
  { SI, {"TGEIU"} },
  { SI, {"TLTI"} },
  { SI, {"TLTIU"} },
  { SI, {"TEQI"} },
  { RESREG, {NULL} },
  { SI, {"TNEI"} },
  { RESREG, {NULL} },

  { SO, {"BLTZAL"} },
  { SO, {"BGEZAL"} },
  { SO, {"BLTZALL"} },
  { SO, {"BGEZALL"} },
  { RESREG, {NULL} },
  { RESREG, {NULL} },
  { RESREG, {NULL} },
  { RESREG, {NULL} },

  { RESREG, {NULL} },
  { RESREG, {NULL} },
  { RESREG, {NULL} },
  { RESREG, {NULL} },
  { B,  {"BPOSGE32"} },
  { B,  {"BPOSGE64"} },
  { RESREG, {NULL} },
  { OS, {"SYNCI"} }

};

static const struct iType CopRsTable[32] = {

  { CTD, {"MFCz"} },
  { CTD, {"DMFCz"} },
  { CTD, {"CFCz"} },
  { CTD, {"MFHCz"} },
  { CTD, {"MTCz"} },
  { CTD, {"DMTCz"} },
  { CTD, {"CTCz"} },
  { CTD, {"MTHCz"} },

  // The following 8 entry need to futher decode for cop0, 1 and 2
  { FUN,    {(char *)DecodeCopRt} },
  { C1BCCO, {"BC1ANY2"} },
  { FUN,    {(char *)DecodeCopRsOp} },//{ C1BCCO, {"BC1ANY4"} },
  { FUN,    {(char *)DecodeMFMC0} },
  { FUN,    {(char *)DecodeMTTR} },
  { RESCRS, {NULL} },
  { DT,     {"WRPGPR"} },
  { RESCRS, {NULL} },

  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },

  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} },
  { FUN, {(char *)DecodeCopOp} }
};

static const struct iType SLLTable[4] = {
  { O, {"NOP"} },
  { O, {"SSNOP"} },
  { O, {"EHB"} },
  { DTA, {"SLL"} }
};

static const struct iType BEQTable[2] = {
  { B, {"B"} },
  { STO, {"BEQ"} }
};

static const struct CopZeroEntry {
  uint32_t      fnum;
  struct iType  it;
} CopZeroTable[] = {
  { 0x1, { O, {"TLBR"} } },
  { 0x2, { O, {"TLBWI"} } },
  { 0x6, { O, {"TLBWR"} } },
  { 0x8, { O, {"TLBP"} } },
  { 0x10, { O, {"RFE"} } },
  { 0x18, { O, {"ERET"} } },
  { 0x19, { O, {"IACK"} } },
  { 0x1f, { O, {"DERET"} } },
  { 0x20, { O, {"WAIT"} } },
  { 0x38, { O, {"IRET"} } }
};

static const struct iType Cop1RsTable[64] =
{
  { C1FDST, {"ADD"} },
  { C1FDST, {"SUB"} },
  { C1FDST, {"MUL"} },
  { C1FDST, {"DIV"} },
  { C1FDS, {"SQRT"} },
  { C1FDS, {"ABS"}  },
  { C1FDS, {"MOV"} },
  { C1FDS, {"NEG"} },

  { C1FDS, {"ROUND.L"} },
  { C1FDS, {"TRUNC.L"} },
  { C1FDS, {"CEIL.L"} },
  { C1FDS, {"FLOOR.L"} },
  { C1FDS, {"ROUND.W"} },
  { C1FDS, {"TRUNC.W"} },
  { C1FDS, {"CEIL.W"} },
  { C1FDS, {"FLOOR.W"} },

  { RESCRS, {NULL} },
  { FUN, {(char *)DecodeMovCond} },
  { C1FDSGT, {"MOVZ"} },
  { C1FDSGT, {"MOVN"} },
  { RESCRS, {NULL} },
  { C1FDS, {"RECIP"} },
  { C1FDS, {"RSQRT"} },
  { RESCRS, {NULL} },

  { C1DST, {"ADDR.PS"} },
  { RESCRS, {NULL} },
  { C1DST, {"MULR.PS"} },
  { RESCRS, {NULL} },
  { C1FDST, {"RECIP2"} },
  { C1FDS,  {"RECIP1"} },
  { C1FDS,  {"RSQRT1"} },
  { C1FDST, {"RSQRT2"} },

  { C1CVTS, {"CVT.S"} },
  { C1FDS, {"CVT.D"} },
  { RESCRS, {NULL} },
  { RESCRS, {NULL} },
  { C1CVTW, {"CVT"} },
  { C1FDS, {"CVT.L"} },
  { C1CVTPS, {"CVT.PS"} },
  { RESCRS, {NULL} },

  { C1DS,   {"CVT.S.PL"} },
  { RESCRS, {NULL} },
  { RESCRS, {NULL} },
  { RESCRS, {NULL} },
  { C1DST,  {"PLL.PS"} },
  { C1DST,  {"PLU.PS"} },
  { C1DST,  {"PUL.PS"} },
  { C1DST,  {"PUU.PS"} },

  { C1FCCST, {"F"} },
  { C1FCCST, {"UN"} },
  { C1FCCST, {"EQ"} },
  { C1FCCST, {"UEQ"} },
  { C1FCCST, {"OLT"} },
  { C1FCCST, {"ULT"} },
  { C1FCCST, {"OLE"} },
  { C1FCCST, {"ULE"} },

  { C1FCCST, {"SF"} },
  { C1FCCST, {"NGLE"} },
  { C1FCCST, {"SEQ"} },
  { C1FCCST, {"NGL"} },
  { C1FCCST, {"LT"} },
  { C1FCCST, {"NGE"} },
  { C1FCCST, {"LE"} },
  { C1FCCST, {"NGT"} }

};

static const struct iType Cop1XTable[64] =
{
  { C1DIB, {"LWXC1"} },
  { C1DIB, {"LDXC1"} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { C1DIB, {"LUXC1"} },
  { RES, {NULL} },
  { RES, {NULL} },

  { C1SIB, {"SWXC1"} },
  { C1SIB, {"SDXC1"} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { C1SIB, {"SUXC1"} },
  { RES, {NULL} },
  { C1HIB, {"PREFX"} },

  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },

  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { C1DSTS, {"ALNV.PS"} },
  { RES, {NULL} },

  { C1DRST, {"MADD.S"} },
  { C1DRST, {"MADD.D"} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { C1DRST, {"MADD.PS"} },
  { RES, {NULL} },

  { C1DRST, {"MSUB.S"} },
  { C1DRST, {"MSUB.D"} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { C1DRST, {"MSUB.PS"} },
  { RES, {NULL} },

  { C1DRST, {"NMADD.S"} },
  { C1DRST, {"NMADD.D"} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { C1DRST, {"NMADD.PS"} },
  { RES, {NULL} },

  { C1DRST, {"NMSUB.S"} },
  { C1DRST, {"NMSUB.D"} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { RES, {NULL} },
  { C1DRST, {"NMSUB.PS"} },
  { RES, {NULL} }

};

#ifdef MIPS_MDMX

// MDMX Opcodes

static const struct iType MDMXTable[64] = {
  { MDMX, {"MSGN"} },
  { MDMX_C, {"C.EQ"} },
  { MDMX, {"PICKF"} },
  { MDMX, {"PICKT"} },
  { MDMX_C, {"C.LT"}  },
  { MDMX_C, {"C.LE"} },
  { MDMX, {"MIN"} },
  { MDMX, {"MAX"} },

  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { MDMX, {"SUB"} },
  { MDMX, {"ADD"} },
  { MDMX, {"AND"} },
  { MDMX, {"XOR"} },
  { MDMX, {"OR"} },
  { MDMX, {"NOR"} },

  { MDMX, {"SLL"} },
  { RESMDMX, {NULL} },
  { MDMX, {"SRL"} },
  { MDMX, {"SRA"} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },

  { MDMX_ALNI, {"ALNI.OB"} },
  { MDMX_ALNV, {"ALNV.OB"} },
  { MDMX_ALNI, {"ALNI.QH"} },
  { MDMX_ALNV, {"ALNV.QH"} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { MDMX_SHFL, {"SHFL"} },

  { MDMX_R, {"RZU"} },
  { MDMX_R, {"RNAU"} },
  { MDMX_R, {"RNEU"} },
  { RESSPC2, {NULL} },
  { MDMX_R, {"RZS"} },
  { MDMX_R, {"RNAS"} },
  { MDMX_R, {"RNES"} },
  { RESMDMX, {NULL} },

  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },

  { MDMX, {"MUL"} },
  { RESMDMX, {NULL} },
  { MDMX_SAC, {"MULS"}},
  { MDMX_AC, {"MUL"} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { MDMX_AC, {"SUB"} },
  { MDMX_AC, {"ADD"} },

  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { RESMDMX, {NULL} },
  { MDMX_WAC, {"WAC"} },
  { MDMX_RAC, {"RAC"} }

};

#endif /* MIPS_MDMX */

static const char    *RegName[32] = {
    "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra"
};

static const char* Cop0Name[32][8] =
{
  /* 0  */ { "Index",     "MVPControl",    "MVPConf0",     "MVPConf1",
             "MVPConf2"                                                  },
  /* 1  */ { "Random",    "VPEControl",    "VPEConf0",     "VPEConf1",
             "YQMask",    "VPESchedule",   "VPEScheFBack", "VPEOpt"      },
  /* 2  */ { "EntryLo0",  "TCStatus",      "TCBind",       "TCRestart",
             "TCHalt",    "TCContext",     "TCSchedule",   "TCScheFBack" },
  /* 3  */ { "EntryLo1",  NULL,            NULL,           NULL,
             NULL,        NULL,            NULL,           "TCOpt"       },
  /* 4  */ { "Context",   "ContextConfig", "UserLocal"                   },
  /* 5  */ { "PageMask",  "PageGrain"                                    },
  /* 6  */ { "Wired",     "SRSConf0",      "SRSConf1",     "SRSConf2",
             "SRSConf3",  "SRSConf4"                                     },
  /* 7  */ { "HWREna"                                                    },
  /* 8  */ { "BadVAddr"                                                  },
  /* 9  */ { "Count"                                                     },
  /* 10 */ { "EntryHi"                                                   },
  /* 11 */ { "Compare" },
  /* 12 */ { "Status",    "IntCtl",        "SRSCtl",       "SRSMap",
             "View_IPL",  "SRSMap2"                                      },
  /* 13 */ { "Cause",     NULL,            NULL,           NULL,
             "View_RIPL"                                                 },
  /* 14 */ { "EPC",       NULL,            NULL,           NULL,
             NULL,        NULL,            "EPC6",         "EPC7"        },
  /* 15 */ { "PRId",      "EBase",         "CDMMBase"                    },
  /* 16 */ { "Config",    "Config1",       "Config2",      "Config3",
             "Config4",   "Config5",       "Config6",      "Config7"     },
  /* 17 */ { "LLAddr" },
  /* 18 */ { "WatchLo",   "WatchLo1",      "WatchLo2",     "WatchLo3",
             "WatchLo4",  "WatchLo5",      "WatchLo6",     "WatchLo7"    },
  /* 29 */ { "WatchHi",   "WatchHi1",      "WatchHi2",     "WatchHi3",
             "WatchHi4",  "WatchHi5",      "WatchHi6",     "WatchHi7"    },
  /* 20 */ { "XContext"                                                  },
  /* 21 */ { NULL                                                        },
  /* 22 */ { "SEC",       "SEC1"                                         },
  /* 23 */ { "Debug",     "TraceControl",  "TraceControl2", "UserTraceData",
             "TraceIBPC", "TraceDBPC",     "Debug2"                      },
  /* 24 */ { "DEPC",      NULL,            "TraceControl3", "UserTraceData2",
             NULL,        NULL,            "DEPC6",         "DEPC7"      },
  /* 25 */ { "PerfCtrl0", "PerfCnt0",      "PerfCtrl1",     "PerfCnt1",
             "PerfCtrl2", "PerfCnt2",      "PerfCtrl3",     "PerfCnt3"   },
  /* 26 */ { "ErrCtl",    "IErrCtl"                                      },
  /* 27 */ { "CacheErr"                                                  },
  /* 28 */ { "ITagLo",    "IDataLo",       "DTagLo",        "DDataLo",
             "L23TagLo",  "L23DataLo"                                    },
  /* 29 */ { "ITagHi",    "IDataHi",       "DTagHi",        "DDataHi",
             "L23TagHi",  "L23DataHi"                                    },

  /* 30 */ { "ErrorEPC",  NULL,            NULL,            NULL,
             NULL,        NULL,            "ErrorEPC6",     "ErrorEPC7"  },
  /* 31 */ { "DESAVE"                                                    },
};

/*
        Access the Special Table and return ptr to entry
*/
const struct iType    *DecodeSpecial(uint32_t opcode)
{
  //fprintf (stdout, "spec=%x\n", opcode & BITMASK5_0);
  uint32_t      idx = opcode & BITMASK5_0;
  const struct iType  *p = SpecialTable+idx;
  if (p->fmt == FUN)
    return (*(p->n.f))(opcode);
  else
    return p;
}

/*
        Access the Spec2 Table and return ptr to entry
*/
const struct iType    *DecodeSpec2(uint32_t opcode)
{
  uint32_t      idx = opcode & BITMASK5_0;
  const struct iType  *p = Spec2Table+idx;
  if (p->fmt == FUN)
    return (*(p->n.f))(opcode);
  else
    return p;
}

/*
        Access the Spec3 Table and return ptr to entry
*/
const struct iType    *DecodeSpec3(uint32_t opcode)
{
  uint32_t      idx = opcode & BITMASK5_0;
  const struct iType  *p = Spec3Table+idx;
  if (p->fmt == FUN)
    return (*(p->n.f))(opcode);
  else
    return p;
}

const struct iType    *DecodeLX(uint32_t opcode)
{
    static struct iType lx_variant[9] =
    {
        { DIB,     {"LWX"}  },
        { RESSPC3, {NULL}   },
        { RESSPC3, {NULL}   },
        { RESSPC3, {NULL}   },
        { DIB,     {"LHX"}  },
        { RESSPC3, {NULL}   },
        { DIB,     {"LBUX"}   },
        { RESSPC3, {NULL}   },

        { DIB,     {"LDX"}  }
    };

    uint32_t idx = (opcode >> 6) & BITMASK4_0;

    switch (idx)
    {
        case 0:
        case 4:
        case 6:
        case 8:
            return &lx_variant[idx];
        default:
            return &lx_variant[1];
    }
}

const struct iType    *DecodeADDUQB(uint32_t opcode)
{
    const struct iType *p = AdduQBTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

#if 0
const struct iType    *DecodeADDUOB(uint32_t opcode)
{
    const struct iType *p = AdduOBTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}
#endif

const struct iType    *DecodeCMPEQQB(uint32_t opcode)
{
    const struct iType *p = CmpEqQBTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

#if 0
const struct iType    *DecodeCMPEQOB(uint32_t opcode)
{
    const struct iType *p = CmpEqOBTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}
#endif

const struct iType    *DecodeABSQPH(uint32_t opcode)
{
    const struct iType *p = AbsqPHTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

#if 0
const struct iType    *DecodeABSQQH(uint32_t opcode)
{
    const struct iType *p = AbsqQHTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}
#endif

const struct iType    *DecodeSHLLQB(uint32_t opcode)
{
    const struct iType *p = ShllQBTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

#if 0
const struct iType    *DecodeSHLLOB(uint32_t opcode)
{
    const struct iType *p = ShllOBTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}
#endif

const struct iType    *DecodeDPAQWPH(uint32_t opcode)
{
    const struct iType *p = DpaqWPHTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

const struct iType    *DecodeDPAQWQH(uint32_t opcode)
{
    const struct iType *p = DpaqWQHTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

const struct iType    *DecodeEXTRW(uint32_t opcode)
{
    const struct iType *p = ExtrWTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

#if 0
const struct iType    *DecodeDEXTRW(uint32_t opcode)
{
    const struct iType *p = DExtrWTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}
#endif

const struct iType    *DecodeADDUHQB(uint32_t opcode)
{
    const struct iType *p = AdduhQBTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

const struct iType    *DecodeAPPEND(uint32_t opcode)
{
    const struct iType *p = AppendTable+((opcode >> 6) & BITMASK4_0);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}


/*
        Access the RegImm Table and return ptr to entry
*/
const struct iType    *DecodeRegImm(uint32_t opcode)
{
  //fprintf (stdout, "rt=%x\n", (opcode>>16) & BITMASK4_0);
  const struct iType  *p =  RegImmTable+((opcode >> 16) & BITMASK4_0);

  if (p->fmt == FUN)
      return (*(p->n.f))(opcode);
  else
      return p;
}

const struct iType* DecodeRegImmAtomic(uint32_t opcode)
{
    const struct iType  *p =  RegImmAtomicTable+((opcode >> 15) & 1);

    if (p->fmt == FUN)
        return (*(p->n.f))(opcode);
    else
        return p;
}

/*
        Access the CopRs Table and return ptr to entry
*/
const struct iType    *DecodeCopRs(uint32_t opcode)
{
  const struct iType  *p = CopRsTable+((opcode >> 21) & BITMASK4_0);
  //fprintf (stdout, "(cop)rs=%x\n",  (opcode>>21) & BITMASK4_0);

  if (p->fmt == FUN)
    return (*(p->n.f))(opcode);
  else
    return p;
}

/*
        Access the CopRt Table and return ptr to entry
*/
const struct iType    *DecodeCopRt(uint32_t opcode)
{
    static const struct iType copx_rt_res = { RESCRS, {NULL} };

    static const struct iType copx_rt_variant[4] = { { CO, {"BCzF"} },
                                               { CO, {"BCzT"} },
                                               { CO, {"BCzFL"}  },
                                               { CO, {"BCzTL"}  } };

    switch ((opcode >> 26) & 3)
    {
        case 0: // cop0
            return DecodeMFTR(opcode);
        case 1: // cop1
        case 2: // cop2
        case 3: // cop3
            return copx_rt_variant+((opcode>>16) & 0x3);
        default:
            break;
    }

    return &copx_rt_res;
}

const struct iType  *DecodeCopRsOp(uint32_t opcode)
{
   static const struct iType GenCopOp = { CF, {"COPz"} };

   static const struct iType UC0_Op  = { DT, {"RDPGPR"} };

   static const struct iType BC1ANY4 = { C1BCCO, {"BC1ANY4"} };

   switch ((opcode >> 26) & 3)
   {
   case 0:
      {
         // cop0
         return &UC0_Op;
      }

   case 1:
      // cop1
      return &BC1ANY4;

   default:
      break;
   }

   return  &GenCopOp;

}

const struct iType  *DecodeMFMC0(uint32_t opcode)
{
    static const struct iType mfmc0_res = { RESCRS, {NULL} };

    static const struct iType VPE_Op[]  = { { T, { "DVPE" } },
                                            { T, { "EVPE" } } };

    static const struct iType MT_Op[]   = { { T, { "DMT" } },
                                            { T, { "EMT" } } };

    static const struct iType UC0_Op[]  = { { T, { "DI" } },
                                            { T, { "EI" } } };

    uint32_t pos = (opcode & (0x1f << 6)) >> 6;
    uint32_t sc  = (opcode >> 5) & 1;
    uint32_t sel = opcode & BITMASK2_0;

    // cop0
    switch ((opcode >> 11) & BITMASK4_0)
    {
        case 0:
            if ( sel == 1 && pos == 0 ) return &VPE_Op[sc];
            break;
        case 1:
            if ( sel == 1 && pos == 15 ) return &MT_Op[sc];
            break;
        case 12: // Status (DI/EI)
            if ( sel == 0 && pos == 0 ) return &UC0_Op[sc];
            break;
        default:
            break;
    }

    return &mfmc0_res;
}

const struct iType *DecodeMFTR(uint32_t opcode)
{
    static const struct iType mftr_op[10] = { { RESCRS, {NULL} },      // 0
                                              { CFDT,   {"MFTC0"} },   // 1
                                              { DT,     {"MFTGPR"} },  // 2
                                              { MFTAC,  {"MFTLO"} },   // 3
                                              { MFTAC,  {"MFTHI"} },   // 4
                                              { MFTAC,  {"MFTACX"} },  // 5
                                              { D,      {"MFTDSP"} },  // 6
                                              { CFDT,   {"MFTCz"} },   // 7
                                              { CFDT,   {"MFTHCz"} },  // 8
                                              { CFDT,   {"CFTCz"} } }; // 9

    uint32_t rt  = (opcode >> 16) & BITMASK4_0;

    if ( ( (opcode >> 5) & 1 ) == 0 ) // u == 0
        // CP0
        return &mftr_op[1];
    else // u == 1
    {
        switch (opcode & BITMASK2_0) // sel
        {
            case 0: return &mftr_op[2]; // GPR
            case 1:
                switch (rt%4)
                {
                    case 0:
                        if (rt != 16)
                            return &mftr_op[3]; // LO
                        else
                            return &mftr_op[6]; // DSP
                    case 1: return &mftr_op[4]; // HI
                    case 2: return &mftr_op[5]; // ACX
                    default: break;
                }
            case 2:
            case 4:
                if ( (opcode >> 4) & 1 ) // h
                    return &mftr_op[8]; // HCz
                else
                    return &mftr_op[7]; // Cz
            case 3:
            case 5:
                return &mftr_op[9]; // CFTCz
            default:
                break;
        }
    }

    return &mftr_op[0];
}

const struct iType *DecodeMTTR(uint32_t opcode)
{
    static const struct iType mttr_op[10] = { { RESCRS, {NULL} },     // 0
                                              { CTTD,  {"MTTCz"} },   // 1
                                              { TD,    {"MTTGPR"} },  // 2
                                              { MTTAC, {"MTTLO"} },   // 3
                                              { MTTAC, {"MTTHI"} },   // 4
                                              { MTTAC, {"MTTACX"} },  // 5
                                              { T,     {"MTTDSP"} },  // 6
                                              { CTTD,  {"MTTCz"} },   // 7
                                              { CTTD,  {"MTTHCz"} },  // 8
                                              { CTTD,  {"CTTCz"} } }; // 9

    uint32_t rd  = (opcode >> 11) & BITMASK4_0;

    if ( ( (opcode >> 5) & 1 ) == 0 ) // u == 0
        // CP0
        return &mttr_op[1];
    else // u == 1
    {
        switch (opcode & BITMASK2_0) // sel
        {
            case 0: return &mttr_op[2]; // GPR
            case 1:
                switch (rd%4)
                {
                    case 0:
                        if (rd != 16)
                            return &mttr_op[3]; // LO
                        else
                            return &mttr_op[6]; // DSP
                    case 1: return &mttr_op[4]; // HI
                    case 2: return &mttr_op[5]; // ACX
                    default: break;
                }
            case 2:
            case 4:
                if ( (opcode >> 4) & 1 ) // h
                    return &mttr_op[8]; // MTTHCz
                else
                    return &mttr_op[7]; // MTTCz
            case 3:
            case 5:
                return &mttr_op[9]; // CTTCz
            default:
                break;
        }
    }

    return &mttr_op[0];
}

/*
        Decode CoProcessor Operation
*/
const struct iType    *DecodeCopOp(uint32_t opcode)
{
  static const struct iType   GenCopOp = { CF, {"COPz"} };

  //fprintf (stdout, "(cop)op=%x\n", opcode & 0xffffff);

  switch ((opcode >> 26) & 3) {
  case 0:
  { // cop0


      uint32_t  op = opcode & 0x3f; //0xffffff
      unsigned int i;
      for (i = 0; i < (sizeof(CopZeroTable) / sizeof(struct CopZeroEntry)); i++)
          if (CopZeroTable[i].fnum == op)
              return &CopZeroTable[i].it;
      break;
  }
  case 1:
  { // cop1
      const struct iType      *p = Cop1RsTable+(opcode & BITMASK5_0);
      if (p->fmt == FUN)
          return (*(p->n.f))(opcode);
      else
          return p;
      break;
  }
  default:
      break;
  }

  return        &GenCopOp;
}


#ifdef MIPS_MDMX
/*
        Access the MDMX Table and return ptr to entry
*/
const struct iType    *DecodeMDMX(uint32_t opcode)
{
  //fprintf (stdout, "spec=%x\n", opcode & BITMASK5_0);
  uint32_t      idx = opcode & BITMASK5_0;
  const struct iType  *p = MDMXTable+idx;
  if (p->fmt == FUN)
    return (*(p->n.f))(opcode);
  else
    return p;
}

#endif // MIPS_MDMX


//////////////////////////////////////////////
const struct iType    *DecodeMovCond(uint32_t opcode)
{
  static const struct iType mov_t[2] =
  {
     { C1FDSCC, {"MOVT"} },
     { C1FDSCC, {"MOVF"} }
  };
  static const struct iType mov_ti[2] =
  {
     { C1GRDSCC, {"MOVT"} },
     { C1GRDSCC, {"MOVF"} }
  };

  uint32_t op = opcode >> 26;
  if (opcode & (1<<16))
     return (op ? &mov_t[0] : &mov_ti[0]);
  else
     return (op ? &mov_t[1] : &mov_ti[1]);
}


/*
        Access the Cop1X Table and return ptr to entry
*/
const struct iType    *DecodeCop1X(uint32_t opcode)
{
  return Cop1XTable+(opcode & BITMASK5_0);
}


/*
        Return SLL or NOP depending on opcode
*/
const struct iType    *DecodeSLL(uint32_t opcode)
{
    if (opcode == 0)
        return SLLTable+0;
    else if (opcode == 0x00000040)
        return SLLTable+1;
    else if (opcode == 0x000000c0)
        return SLLTable+2;
    else
        return SLLTable+3;
}

const struct iType  * DecodeJALR(uint32_t opcode)
{
    static const struct iType jalr_variant[2] = {
        { DS, {"JALR"} },
        { DS, {"JALR.HB"} }
    };

    if (opcode & (1 << 10))
        return &jalr_variant[1];
    else
        return &jalr_variant[0];
}

const struct iType  * DecodeJR(uint32_t opcode)
{
    static const struct iType jr_variant[2] = {
        { S, {"JR"} },
        { S, {"JR.HB"} }
    };

    if (opcode & (1 << 10))
        return &jr_variant[1];
    else
        return &jr_variant[0];
}

/*
        Return BEQ or B depending on operands
*/
const struct iType    *DecodeBEQ(uint32_t opcode)
{
  if (opcode & 0x03ff0000)
    return BEQTable+1;
  else
    return BEQTable+0;
}


/*
 * MIPS Crypto Instructions, which overload some existing opcodes
 */

const struct iType  * DecodeMADDU(uint32_t opcode)
{
    static const struct iType maddu_variant[4] = {
        { ST, {"MADDU"} },
        { ST, {"MADDP"} },
        { ST, {"PPERM"} },
        { RESSPC2, {NULL} }
    };

    switch ((opcode & 0x7c0) >> 6) {
        case 0x0:
            return &maddu_variant[0];
            break; // NOTREACHED
        case 0x11:
            return &maddu_variant[1];
            break; // NOTREACHED
        case 0x12:
            return &maddu_variant[2];
            break; // NOTREACHED
        default:
            return &maddu_variant[3];
            break; // NOTREACHED
    }
}

const struct iType  * DecodeMFLO(uint32_t opcode)
{
    static const struct iType mflo_variant[3] = {
        { MFAC,   {"MFLO"} },
        { D,      {"MFLHXU"} },
        { RESSPC, {NULL} }
    };

    switch ((opcode & 0x7c0) >> 6) {
        case 0x0:
            return &mflo_variant[0];
            break; // NOTREACHED
        case 0x1:
            return &mflo_variant[1];
            break; // NOTREACHED
        default:
            return &mflo_variant[2];
            break; // NOTREACHED
    }
}

const struct iType  * DecodeMTLO(uint32_t opcode)
{
    static const struct iType mtlo_variant[3] = {
        { MTAC,   {"MTLO"} },
        { D,      {"MTLHX"} },
        { RESSPC, {NULL} }
    };

    switch ((opcode & 0x7c0) >> 6) {
        case 0x0:
            return &mtlo_variant[0];
            break; // NOTREACHED
        case 0x1:
            return &mtlo_variant[1];
            break; // NOTREACHED
        default:
            return &mtlo_variant[2];
            break; // NOTREACHED
    }
}

const struct iType  * DecodeMULTU(uint32_t opcode)
{
    static const struct iType multu_variant[3] = {
        { ST, {"MULTU"} },
        { ST, {"MULTP"} },
        { RESSPC, {NULL} }
    };

    switch ((opcode & 0x7c0) >> 6) {
        case 0x0:
            return &multu_variant[0];
            break; // NOTREACHED
        case 0x11:
            return &multu_variant[1];
            break; // NOTREACHED
        default:
            return &multu_variant[3];
            break; // NOTREACHED
    }
}

const struct iType  * DecodeSRL(uint32_t opcode)
{
    static const struct iType srl_variant[3] = {
        { DTA, {"SRL"} },
        { DTA, {"ROTR"} },
        { RESSPC, {NULL} }
    };

    switch ((opcode & 0x03e00000) >> 21) {
        case 0x0:
            return &srl_variant[0];
            break; // NOTREACHED
        case 0x1:
            return &srl_variant[1];
            break; // NOTREACHED
        default:
            return &srl_variant[3];
            break; // NOTREACHED
    }
}

const struct iType  * DecodeSRLV(uint32_t opcode)
{
    static const struct iType srlv_variant[3] = {
        { DTS, {"SRLV"} },
        { DTS, {"ROTRV"} },
        { RESSPC, {NULL} }
    };

    switch ((opcode & 0x000007c0) >> 6) {
        case 0x0:
            return &srlv_variant[0];
            break; // NOTREACHED
        case 0x1:
            return &srlv_variant[1];
            break; // NOTREACHED
        default:
            return &srlv_variant[3];
            break; // NOTREACHED
    }
}


const struct iType  * DecodeDSRL(uint32_t opcode)
{
    static const struct iType dsrl_variant[3] = {
        { DTA, {"DSRL"} },
        { DTA, {"DROTR"} },
        { RESSPC, {NULL} }
    };

    switch ((opcode & 0x03e00000) >> 21) {
        case 0x0:
            return &dsrl_variant[0];
            break; // NOTREACHED
        case 0x1:
            return &dsrl_variant[1];
            break; // NOTREACHED
        default:
            return &dsrl_variant[3];
            break; // NOTREACHED
    }
}

const struct iType  * DecodeDSRL32(uint32_t opcode)
{
    static const struct iType dsrl32_variant[3] = {
        { DTA, {"DSRL32"} },
        { DTA, {"DROTR32"} },
        { RESSPC, {NULL} }
    };

    switch ((opcode & 0x03e00000) >> 21) {
        case 0x0:
            return &dsrl32_variant[0];
            break; // NOTREACHED
        case 0x1:
            return &dsrl32_variant[1];
            break; // NOTREACHED
        default:
            return &dsrl32_variant[3];
            break; // NOTREACHED
    }
}

const struct iType  * DecodeDSRLV(uint32_t opcode)
{
    static const struct iType dsrlv_variant[3] = {
        { DTS, {"DSRLV"} },
        { DTS, {"DROTRV"} },
        { RESSPC, {NULL} }
    };

    switch ((opcode & 0x000007c0) >> 6) {
        case 0x0:
            return &dsrlv_variant[0];
            break; // NOTREACHED
        case 0x1:
            return &dsrlv_variant[1];
            break; // NOTREACHED
        default:
            return &dsrlv_variant[3];
            break; // NOTREACHED
    }
}

const struct iType  * DecodeBSHFL(uint32_t opcode)
{
    static const struct iType bshfl_variant[4] = {
        { DT, {"WSBH"} },
        { DT, {"SEB"} },
        { DT, {"SEH"} },
        { DTA, {"DBSHFL"} }
    };

    switch (( opcode & (0x1f << 6)) >> 6) {
    case 0x02:
        return &bshfl_variant[0];
        break; // NOTREACHED
    case 0x10:
        return &bshfl_variant[1];
        break; // NOTREACHED
    case 0x18:
        return &bshfl_variant[2];
        break; // NOTREACHED
    default:
        return &bshfl_variant[3];
        break; // NOTREACHED
    }
}

const struct iType  * DecodeDBSHFL(uint32_t opcode)
{
    static const struct iType dbshfl_variant[3] = {
        { DT, {"DSBH"} },
        { DT, {"DSHD"} },
        { DTA, {"DBSHFL"} }
    };

    switch (( opcode & (0x1f << 6)) >> 6) {
    case 0x02:
        return &dbshfl_variant[0];
        break; // NOTREACHED
    case 0x5:
        return &dbshfl_variant[1];
        break; // NOTREACHED
    default:
        return &dbshfl_variant[2];
        break; // NOTREACHED
    }
}



/*
   Select floating point format
*/
static char *sel_fpfmt ( uint32_t op )
{
   static char *fmt_tab[9] =
   {
      "S", "D", "??", "??", "W", "L", "PS", "??", "???"
   };

   if ( (op >> 3) == 0x2)
      return fmt_tab [op & 0x7];
   return fmt_tab [8];
}

#ifdef MIPS_MDMX
/*
   Describe MDMX vector or scalar format
*/
void mdmx_fmtsel ( uint32_t op, char *dtype, char *vtpref, char *vtsuff )
{
   int nselbits;
   int fmtsel_field = (op >> 21) & 0x1f;

   if(!(fmtsel_field & 0x1)) {
        strcpy(dtype, ".OB");
        nselbits = 3;
   } else if ((fmtsel_field & 0x3) == 1) {
        strcpy(dtype, ".QH");
        nselbits = 2;
   } else if ((fmtsel_field & 0x7) == 3) {
        strcpy(dtype, ".PW");
        nselbits = 1;
   } else {
        strcpy(dtype, ".D");
        nselbits = 0;
   }

   if(!(fmtsel_field & 0x10)) {
        /* Vector/Scalar */
        strcpy(vtpref, "v");
        sprintf(vtsuff, "[%d]",
            (fmtsel_field >> (4 - nselbits)) & ((1 << nselbits) - 1));
   } else if ((fmtsel_field & 0x18) == 0x18) {
        /* Immdeiate Scalar */
        strcpy(vtpref, "");
        strcpy(vtsuff, "");
   } else {
        /* Vector */
        strcpy(vtpref, "v");
        strcpy(vtsuff, "");
   }
}

/*
 * MDMX SHFL shuffle format mnemonics
 */
static char *shflfmt[] = {
        ".?",
        ".MIXH",
        ".?",
        ".?",

        ".?",
        ".MIXL",
        ".UPSL",
        ".?",

        ".PACH",
        ".PACH",
        ".?",
        ".?",

        ".MIXH",
        ".?",
        ".MIXL",
        ".?",

        ".?",
        ".BFLA",
        ".?",
        ".?",

        ".?",
        ".?",
        ".?",
        ".?",

        ".?",
        ".REPA",
        ".?",
        ".?",

        ".?",
        ".REPB",
        ".?",
        ".?"
};

#endif // MIPS_MDMX

/*
        Main Routine
*/
const char* disasm_mips(uint32_t opcode, uint32_t pc)
{
  uint32_t      op = opcode >> 26;
  static char   retbuf[128];
  char *prb;
  uint32_t      cp, rs, rt, rd, sa, target, sel, cc, pos, size, imm, ac;
  int   i;

  char *fmt, *fmt1;
  uint32_t  fd, fs, ft, fr, fbase, findex, fhint, tmp;

#ifdef MIPS_MDMX
  char dtstr[4];
  char vtpre[2];
  char vtsuf[4];
  uint32_t vd, vs, vt;
#endif // MIPS_MDMX

  const struct iType  *p = MainTable+op;
  //fprintf (stdout, "op=%x   fmt=%d\n", op, p->fmt);

  if ( p->fmt == FUN) {
    p =  (*(p->n.f) )(opcode);
  }

  switch(p->fmt) {
  case DST:     // rd, rs, rt
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    rd = (opcode >> 11) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, %s/$%i, %s/$%i",
              p->n.name, RegName[rd], rd,
              RegName[rs], rs, RegName[rt], rt);
    else
      sprintf (retbuf, "%-5s $%i, $%i, $%i",
              p->n.name, rd, rs, rt);
    break;
  case DTS:     // rd, rs, rt
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    rd = (opcode >> 11) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, %s/$%i, %s/$%i",
              p->n.name, RegName[rd], rd,
              RegName[rt], rt, RegName[rs], rs);
    else
      sprintf (retbuf, "%-5s $%i, $%i, $%i",
              p->n.name, rd, rt, rs);
    break;

  case TSSA:     // rt, rs, sa
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    sa = (opcode >> 11) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, %s/$%i, %i",
              p->n.name, RegName[rt], rt,
              RegName[rs], rs, sa);
    else
      sprintf (retbuf, "%-5s $%i, $%i, %i",
              p->n.name, rt, rs, sa);
    break;

  case TSPS:  // rt, rs, pos, size
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    size = (opcode >> 11) & BITMASK4_0;
    pos = (opcode >> 6) & BITMASK4_0;

    switch (opcode & 7)
    {
    case 0:  // EXT
       size += 1;
       break;
    case 1:  // DEXTM
       size += 33;
       break;
    case 2:  // DEXTU
       size += 1;
       pos  += 32;
       break;
    case 3:  // DEXT
      size += 1;
      break;
    case 4:  // INS
       size = size - pos + 1;
       break;
    case 5:  // DINSM
       size = size - pos + 32 +1;
       break;
    case 6:  // DINSU
       size = size - pos  + 1;
       pos  += 32;
       break;
    }

    if (SymNames)
        sprintf (retbuf, "%-5s %s/$%i, %s/$%i, %d, %d",
                 p->n.name, RegName[rt], rt,
                 RegName[rs], rs, pos, size);
    else
        sprintf (retbuf, "%-5s $%i, $%i, %d, %d",
                 p->n.name, rt, rs, pos, size);
    break;
  case DIB: // rd index(base)
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    rd = (opcode >> 11) & BITMASK4_0;
    sprintf (retbuf, "%s $%i, $%i($%i)", p->n.name, rd, rt, rs);
    break;
  case TSO:     // rt, rs, offset
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, %s/$%i, 0x%x",
              p->n.name, RegName[rt], rt,
              RegName[rs], rs, opcode & 0xffff);
    else
      sprintf (retbuf, "%-5s $%i, $%i, 0x%x",
              p->n.name, rt, rs, opcode & 0xffff);
    break;
  case CO:      // cp offset
    cp = (opcode >> 26) & 0x3;
    cc = (opcode >> 18) & 7;

    target = (pc+4) + ((opcode & 0xffff)<<2);
    if (opcode & 0x8000)
      target += 0xfffc0000;

    sprintf (retbuf, "%s", p->n.name);
    int j;
    for(i=0, j = (int)strlen(retbuf); i<j; i++)
      if (retbuf[i] == 'z')
        retbuf[i] = '0' + cp;
    //sprintf (retbuf+i, " 0x%x", opcode & 0xffffff);
    sprintf (retbuf+i, " %x, 0x%x", cc, target);
    break;
  case STO:     // rs, rt, offset
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;

    target = (pc+4) + ((opcode & 0xffff)<<2);
    if (opcode & 0x8000)
      target += 0xfffc0000;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, %s/$%i, 0x%x",
              p->n.name, RegName[rs], rs,
              RegName[rt], rt, target);
    else
      sprintf (retbuf, "%-5s $%i, $%i, 0x%x",
              p->n.name, rs, rt, target);
    break;
  case SI:      // rs, immediate
    rs = (opcode >> 21) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, 0x%x",
              p->n.name, RegName[rs], rs, opcode & 0xffff);
    else
      sprintf (retbuf, "%-5s $%i, 0x%x",
              p->n.name, rs, opcode & 0xffff);
    break;
  case SO:      // rs, offset
    rs = (opcode >> 21) & BITMASK4_0;

    target = (pc+4) + ((opcode & 0xffff)<<2);
    if (opcode & 0x8000)
      target += 0xfffc0000;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, 0x%x",
              p->n.name, RegName[rs], rs, target);
    else
      sprintf (retbuf, "%-5s $%i, 0x%x",
              p->n.name, rs, target);
    break;
  case SOB:     // bit, offset(rs)
    rs = (opcode >> 21) & BITMASK4_0;
    rd = (opcode >> 12) & BITMASK2_0;

    if (SymNames)
        sprintf (retbuf, "%-5s %i, 0x%x(%s/$%d)",
                p->n.name, rd,
                opcode & 0xffff, RegName[rs], rs);
    else
        sprintf (retbuf, "%-5s %i, 0x%x($%d)",
                p->n.name, rd,
                opcode & 0xffff, rs);
    break;
  case O:               // Opcode Only
    sprintf (retbuf, "%-5s", p->n.name);
    break;
  case TOS:     // rt, offset(rs)
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, 0x%x(%s/$%d)",
              p->n.name, RegName[rt], rt,
              opcode & 0xffff, RegName[rs], rs);
    else
      sprintf (retbuf, "%-5s $%i, 0x%x($%d)",
              p->n.name, rt, opcode & 0xffff, rs);
    break;
  case OS:      // offset(rs)
    rs = (opcode >> 21) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s 0x%x(%s/$%d)",
              p->n.name, opcode & 0xffff, RegName[rs], rs);
    else
      sprintf (retbuf, "%-5s 0x%x($%d)",
              p->n.name, opcode & 0xffff, rs);
    break;
  case TO:      // rt, offset
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%i, 0x%x",
              p->n.name, RegName[rt], rt, opcode & 0xffff);
    else
      sprintf (retbuf, "%-5s $%i, 0x%x", p->n.name, rt, opcode & 0xffff);
    break;
  case B:       // brtarget
    target = (pc+4) + ((opcode & 0xffff)<<2);
    if (opcode & 0x8000)
      target += 0xfffc0000;

    sprintf (retbuf, "%-5s 0x%x", p->n.name, target);

    break;
  case CTD:     // cp rt, rd
  {
    const char* p_name = NULL;

    cp = (opcode >> 26) & 0x3;
    rt = (opcode >> 16) & BITMASK4_0;
    rd = (opcode >> 11) & BITMASK4_0;
    sel = (opcode  & 0x7);
    prb = retbuf;

    prb += sprintf (prb, "%s", p->n.name);
    for(i=0, j = (int)strlen(retbuf); i<j; i++)
    {
      if (retbuf[i] == 'z')
      {
        retbuf[i] = '0' + cp;
      }
    }

    *prb++ = ' ';

    if (SymNames)
    {
        prb += sprintf(prb, "%s/", RegName[rt]);
    }

    prb += sprintf(prb, "$%i, ", rt);

    if (Cop0SymNames && 0 == cp)
    {
        p_name = Cop0Name[rd][sel];
    }

    if (p_name != NULL)
    {
        prb += sprintf(prb, "%s", p_name);
    }
    else if (sel)
    {
        prb += sprintf(prb, "%i, %i", rd, sel);
    }
    else
    {
        prb += sprintf(prb, "%i", rd);
    }
    break;
  }
  case CF:
    cp = (opcode >> 26) & 0x3;
    sprintf (retbuf, "%s", p->n.name);
    for(i=0, j = (int)strlen(retbuf); i<j; i++)
      if (retbuf[i] == 'z')
        retbuf[i] = '0' + cp;
    sprintf (retbuf+i, " 0x%x", opcode & 0xffffff);
    break;
  case ST:      // rs, rt
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d, %s/$%d",
              p->n.name, RegName[rs], rs, RegName[rt], rt);
    else
      sprintf (retbuf, "%-5s $%d, $%d", p->n.name, rs, rt);
    break;
  case DorTS:   // rd if non-zero, else rt, rs
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    rd = (opcode >> 11) & BITMASK4_0;
    prb = retbuf;

    if (rd != rt) {
        if (SymNames)
            prb += sprintf (prb, "%-5s %s/$%d, %s/$%d, %s/$%d",
                           p->n.name, RegName[rd], rd, RegName[rt], rt, RegName[rs], rs);
        else
            prb += sprintf (prb, "%-5s $%d, $%d, $%d", p->n.name, rd, rt, rs);
        sprintf (prb, " WARNING: rt and rd fields are different");
    } else {
        if (SymNames)
            prb += sprintf (prb, "%-5s %s/$%d, %s/$%d",
                           p->n.name, RegName[rt], rt, RegName[rs], rs);
        else
            prb += sprintf (prb, "%-5s $%d, $%d", p->n.name, rt, rs);
    }
    break;

  case J:               // target
    target = ((pc+8) & 0xf0000000) | ((opcode & 0x3ffffff)<<2);
    sprintf (retbuf, "%-5s 0x%x", p->n.name, target);
    break;
  case S:               // rs
    rs = (opcode >> 21) & BITMASK4_0;
    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d",
              p->n.name, RegName[rs], rs);
    else
      sprintf (retbuf, "%-5s $%d", p->n.name, rs);
    break;
  case DS:      // rd, rs
    rs = (opcode >> 21) & BITMASK4_0;
    rd = (opcode >> 11) & BITMASK4_0;
    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d, %s/$%d",
              p->n.name, RegName[rd], rd, RegName[rs], rs);
    else
      sprintf (retbuf, "%-5s $%d, $%d", p->n.name, rd, rs);
    break;
  case D:               // rd
    rd = (opcode >> 11) & BITMASK4_0;
    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d",
              p->n.name, RegName[rd], rd);
    else
      sprintf (retbuf, "%-5s $%d", p->n.name, rd);
    break;
  case DTA:     // rd, rt, sa
    rd = (opcode >> 11) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    sa = (opcode >> 6) & BITMASK4_0;
    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d, %s/$%d, %d",
              p->n.name, RegName[rd], rd,
              RegName[rt], rt, sa);
    else
      sprintf (retbuf, "%-5s $%d, $%d, %d", p->n.name, rd, rt, sa);
    break;

  case T:      // rt
    rt = (opcode >> 16) & BITMASK4_0;

    if (SymNames)
        sprintf (retbuf, "%-5s %s/$%d", p->n.name, RegName[rt], rt);
    else
        sprintf (retbuf, "%-5s $%d", p->n.name, rt);
    break;

  case TD:      // rt, rd
    rd = (opcode >> 11) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d, %s/$%d",
              p->n.name, RegName[rt], rt,
              RegName[rd], rd);
    else
      sprintf (retbuf, "%-5s $%d, $%d", p->n.name, rt, rd);
    break;

  case DT:      // rd, rt
    rd = (opcode >> 11) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d, %s/$%d",
              p->n.name, RegName[rd], rd,
              RegName[rt], rt);
    else
      sprintf (retbuf, "%-5s $%d, $%d", p->n.name, rd, rt);
    break;

    // DSP ASE code start here
  case SDI:      // rs, rd
    rs = (opcode >> 21) & BITMASK4_0;
    rd = (opcode >> 11) & BITMASK9_0;

    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d, 0x%x",
              p->n.name, RegName[rs], rs, rd);
    else
      sprintf (retbuf, "%-5s $%d, 0x%x", p->n.name, rs, rd);
    break;


  case DTI:    // rd, rt
    rd = (opcode >> 11) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK9_0;
    if (SymNames)
      sprintf (retbuf, "%-5s %s/$%d, 0x%x",
              p->n.name, RegName[rd], rd, rt);
    else
      sprintf (retbuf, "%-5s $%d, 0x%x", p->n.name, rd, rt);
    break;


  case TS: // rt, rs
    rs = (opcode >> 21) & BITMASK4_0;
    rt = (opcode >> 16) & BITMASK4_0;
    if (SymNames)
        sprintf (retbuf, "%-5s %s/$%d, %s/$%d",
              p->n.name, RegName[rt], rt,
              RegName[rs], rs);
    else
        sprintf (retbuf, "%-5s $%d, $%d", p->n.name, rt, rs);
    break;

  case DI: // rd, immediate
      rd  = (opcode >> 11) & BITMASK4_0;
      imm = (opcode >> 16) & BITMASK9_0;

      if (SymNames)
          sprintf (retbuf, "%-5s %s/$%i, 0x%x",
                   p->n.name, RegName[rd], rd, imm);
      else
          sprintf (retbuf, "%-5s $%i, 0x%x",
                   p->n.name, rd, imm);
      break;

  case DTSA: // rd, rt, sa (sa is in the rs field)
      rd = (opcode >> 11) & BITMASK4_0;
      rt = (opcode >> 16) & BITMASK4_0;
      sa = (opcode >> 21) & BITMASK4_0;
      if (SymNames)
          sprintf (retbuf, "%-5s %s/$%d, %s/$%d, %d",
                   p->n.name, RegName[rd], rd,
                   RegName[rt], rt, sa);
      else
          sprintf (retbuf, "%-5s $%d, $%d, %d", p->n.name, rd, rt, sa);
      break;

  case ACST:
      ac = (opcode >> 11) & BITMASK1_0;
      rs = (opcode >> 21) & BITMASK4_0;
      rt = (opcode >> 16) & BITMASK4_0;

      if (SymNames)
          sprintf (retbuf, "%-5s %d, %s/$%d, %s/$%d",
                   p->n.name, ac, RegName[rs], rs,
                   RegName[rt], rt);
      else
          sprintf (retbuf, "%-5s %d, $%d, $%d", p->n.name, ac, rs, rt);
      break;

  case TACSA:
      rt = (opcode >> 16) & BITMASK4_0;
      ac = (opcode >> 11) & BITMASK1_0;
      sa = (opcode >> 21) & BITMASK4_0;

      if (SymNames)
          sprintf (retbuf, "%-5s %s/$%d, %d, %d",
                   p->n.name, RegName[rt], rt, ac, sa);
      else
          sprintf (retbuf, "%-5s $%d, %d, %d", p->n.name, rt, ac, sa);
      break;

  case TACS:
      rt = (opcode >> 16) & BITMASK4_0;
      ac = (opcode >> 11) & BITMASK1_0;
      rs = (opcode >> 21) & BITMASK4_0;

      if (SymNames)
          sprintf (retbuf, "%-5s %s/$%d, %d, %s/$%d",
                   p->n.name, RegName[rt], rt, ac,
                   RegName[rs], rs);
      else
          sprintf (retbuf, "%-5s $%d, %d, $%d", p->n.name, rt, ac, rs);
      break;

  case SAC:
      ac = (opcode >> 11) & BITMASK1_0;
      rs = (opcode >> 21) & BITMASK4_0;

      if (SymNames)
          sprintf (retbuf, "%-5s %s/$%d, %d",
                   p->n.name, RegName[rs], rs, ac);
      else
          sprintf (retbuf, "%-5s $%d, %d", p->n.name, rs, ac);
      break;


  case ACS:
      ac = (opcode >> 11) & BITMASK1_0;
      rs = (opcode >> 21) & BITMASK4_0;

      if (SymNames)
          sprintf (retbuf, "%-5s %d, %s/$%d",
                   p->n.name, ac, RegName[rs], rs);
      else
          sprintf (retbuf, "%-5s %d, $%d", p->n.name, ac, rs);
      break;

  case ACSA:
      ac = (opcode >> 11) & BITMASK1_0;
      sa = (opcode >> 20) & BITMASK5_0;

      // sa is a 6 bit signed number -32 to 31
      if ( sa & 0x20 ) sa -= 64;

      sprintf (retbuf, "%-5s %d, %d", p->n.name, ac, sa);
      break;

  case AC:
      ac = (opcode >> 11) & BITMASK1_0;
      sprintf (retbuf, "%-5s %d", p->n.name, ac);
      break;

  case MFAC:
      rd = (opcode >> 11) & BITMASK4_0;
      ac = (opcode >> 21) & BITMASK1_0;

      if (ac)
      {
          if (SymNames)
              sprintf (retbuf, "%-5s %s/$%d, %d",
                       p->n.name, RegName[rd], rd, ac);
          else
              sprintf (retbuf, "%-5s $%d, %d", p->n.name, rd, ac);
      }
      else
      {
          if (SymNames)
              sprintf (retbuf, "%-5s %s/$%d",
                       p->n.name, RegName[rd], rd);
          else
              sprintf (retbuf, "%-5s $%d", p->n.name, rd);
      }
      break;

  case MTAC:
      ac = (opcode >> 11) & BITMASK1_0;
      rs = (opcode >> 21) & BITMASK4_0;

      if (ac)
      {
          if (SymNames)
              sprintf (retbuf, "%-5s %s/$%d, %d",
                       p->n.name, RegName[rs], rs, ac);
          else
              sprintf (retbuf, "%-5s $%d, %d", p->n.name, rs, ac);
      }
      else
      {
          if (SymNames)
              sprintf (retbuf, "%-5s %s/$%d",
                       p->n.name, RegName[rs], rs);
          else
              sprintf (retbuf, "%-5s $%d", p->n.name, rs);
      }
      break;

  // MT code start here
  case MFTAC:
      ac = ((opcode >> 16) & BITMASK4_0)/4;
      rd = (opcode >> 11) & BITMASK4_0;

      if (ac)
      {
          if (SymNames)
              sprintf (retbuf, "%-5s %s/$%d, %d",
                       p->n.name, RegName[rd], rd, ac);
          else
              sprintf (retbuf, "%-5s $%d, %d", p->n.name, rd, ac);
      }
      else
      {
          if (SymNames)
              sprintf (retbuf, "%-5s %s/$%d",
                       p->n.name, RegName[rd], rd);
          else
              sprintf (retbuf, "%-5s $%d", p->n.name, rd);
      }
      break;

  case MTTAC:
      ac = ((opcode >> 11) & BITMASK4_0)/4;
      rt = (opcode >> 16) & BITMASK4_0;

      if (ac)
      {
          if (SymNames)
              sprintf (retbuf, "%-5s %s/$%d, %d",
                       p->n.name, RegName[rt], rt, ac);
          else
              sprintf (retbuf, "%-5s $%d, %d", p->n.name, rt, ac);
      }
      else
      {
          if (SymNames)
              sprintf (retbuf, "%-5s %s/$%d",
                       p->n.name, RegName[rt], rt);
          else
              sprintf (retbuf, "%-5s $%d", p->n.name, rt);
      }
      break;

  case CFDT:     // cp rt, rd
  {
      const char* p_name = NULL;

      sel = opcode & BITMASK2_0;
      cp = (opcode & 0x20) ? sel/2 : 0;

      rt = (opcode >> 16) & BITMASK4_0;
      rd = (opcode >> 11) & BITMASK4_0;

      prb = retbuf;

      prb += sprintf (prb, "%s", p->n.name);

      for(i=0, j = (int)strlen(retbuf); i<j; i++)
          if (retbuf[i] == 'z')
              retbuf[i] = '0' + cp;

      *prb++ = ' ';

      if (SymNames)
        prb += sprintf(prb, "%s/", RegName[rd]);

      prb += sprintf(prb, "$%i, ", rd);

      if (Cop0SymNames && 0 == cp)
          p_name = Cop0Name[rt][sel];

      if (p_name != NULL)
          prb += sprintf(prb, "%s", p_name);
      else if ( opcode & 0x30) // U == 1
          prb += sprintf(prb, "%i", rt);
      else
          prb += sprintf(prb, "%i, %i", rt, sel);
      break;
  }
  case CTTD:     // cp rt, rd
  {
      const char* p_name = NULL;
      sel = opcode & BITMASK2_0;
      cp = (opcode & 0x20) ? sel/2 : 0;

      rt = (opcode >> 16) & BITMASK4_0;
      rd = (opcode >> 11) & BITMASK4_0;

      prb = retbuf;

      prb += sprintf (prb, "%s", p->n.name);

      for(i=0, j = (int)strlen(retbuf); i<j; i++)
          if (retbuf[i] == 'z')
              retbuf[i] = '0' + cp;

      *prb++ = ' ';

      if (SymNames)
          prb += sprintf(prb, "%s/", RegName[rt]);

      prb += sprintf(prb, "$%i, ", rt);

      if (Cop0SymNames && 0 == cp)
          p_name = Cop0Name[rd][sel];

      if (p_name != NULL)
          prb += sprintf(prb, "%s", p_name);
      else if ( opcode & 0x30 ) // u = 1
          prb += sprintf(prb, "%i", rd);
      else
          prb += sprintf(prb, "%i, %i", rd, sel);
      break;
  }
      // COP1 code start here
  case C1FDST: // cp1 .fmt fd fs ft
    fmt = sel_fpfmt ((opcode >> 21) & BITMASK4_0);
    fd  = (opcode >> 6) & BITMASK4_0;
    fs  = (opcode >> 11) & BITMASK4_0;
    ft  = (opcode >> 16) & BITMASK4_0;
    sprintf (retbuf, "%s.%s f%i, f%i,f%i", p->n.name, fmt, fd, fs, ft);
    break;

  case C1FDSGT: // cp1 .fmt fd fs rt
    fmt = sel_fpfmt ((opcode >> 21) & BITMASK4_0);
    fd  = (opcode >> 6) & BITMASK4_0;
    fs  = (opcode >> 11) & BITMASK4_0;
    rt  = (opcode >> 16) & BITMASK4_0;
    sprintf (retbuf, "%s.%s f%i, f%i,$%i", p->n.name, fmt, fd, fs, rt);
    break;


  case C1DST: // cp1 fd fs ft
    fd  = (opcode >> 6) & BITMASK4_0;
    fs  = (opcode >> 11) & BITMASK4_0;
    ft  = (opcode >> 16) & BITMASK4_0;
    sprintf (retbuf, "%s f%i, f%i,f%i", p->n.name, fd, fs, ft);
    break;

  case C1FDS: // cp1 .fmt fd fs
    fmt = sel_fpfmt ((opcode >> 21) & BITMASK4_0);
    fd  = (opcode >> 6) & BITMASK4_0;
    fs  = (opcode >> 11) & BITMASK4_0;
    sprintf (retbuf, "%s.%s f%i, f%i", p->n.name, fmt, fd, fs);
    break;

  case C1CVTS: // cp1 .fmt fd fs    for CVT.S instruction
    tmp = (opcode >> 21) & BITMASK4_0;
    if (((tmp & 0x7) == 0x6) && (tmp >> 3) == 0x2) // PS table
         fmt = "PU";
    else
         fmt = sel_fpfmt (tmp);
    fd  = (opcode >> 6) & BITMASK4_0;
    fs  = (opcode >> 11) & BITMASK4_0;
    sprintf (retbuf, "%s.%s f%i, f%i", p->n.name, fmt, fd, fs);
    break;

  case C1CVTW: // cp1 .fmt fd fs    for CVT.S instruction
    tmp = (opcode >> 21) & BITMASK4_0;
    if (((tmp & 0x7) == 0x6) && (tmp >> 3) == 0x2) // PS table
         fmt1 = "PW";
    else
         fmt1 = "W" ;
    fmt = sel_fpfmt (tmp);
    fd  = (opcode >> 6) & BITMASK4_0;
    fs  = (opcode >> 11) & BITMASK4_0;
    sprintf (retbuf, "%s.%s.%s f%i, f%i", p->n.name, fmt1, fmt, fd, fs);
    break;

  case C1CVTPS: // cp1 .fmt fd fs    for CVT.PS instruction
  {
    char ft_str[8];
    tmp = (opcode >> 21) & BITMASK4_0;
    if (((tmp & 0x7) == 0x4) && (tmp >> 3) == 0x2) // W or L table
    {
         fmt = "PW";
         ft_str[0] = 0;
    }
    else
    {
         fmt = sel_fpfmt (tmp);
         ft  = (opcode >> 16) & BITMASK4_0;
         sprintf(ft_str, ", f%i", ft);
    }
    fd  = (opcode >> 6) & BITMASK4_0;
    fs  = (opcode >> 11) & BITMASK4_0;
    sprintf (retbuf, "%s.%s f%i, f%i%s", p->n.name, fmt, fd, fs, ft_str);
  }
  break;

  case C1DS: // cp1 fd fs
    fd  = (opcode >> 6) & BITMASK4_0;
    fs  = (opcode >> 11) & BITMASK4_0;
    sprintf (retbuf, "%s f%i, f%i", p->n.name, fd, fs);
    break;


  case C1FCCST: // cp1 .fmt cc fs ft
  {
    fmt = sel_fpfmt ((opcode >> 21) & BITMASK4_0);
    fs  = (opcode >> 11) & BITMASK4_0;
    ft  = (opcode >> 16) & BITMASK4_0;
    cc  = (opcode >> 8)  & 0x7;
    sprintf (retbuf, "%s%s.%s %i, f%i, f%i",
            (((opcode >> 6) & 3) == 1) ? "CABS." : "C.",
            p->n.name, fmt, cc, fs, ft);
    break;
  }
  case C1FDSCC: // cp1 .fmt fd fs cc
  {
     fmt = sel_fpfmt ((opcode >> 21) & BITMASK4_0);
     fd  = (opcode >> 6) & BITMASK4_0;
     fs  = (opcode >> 11) & BITMASK4_0;
     cc  = (opcode >> 18)  & 0x7;
     sprintf (retbuf, "%s.%s f%i, f%i, %i",
             p->n.name, fmt, fd, fs, cc);
     break;
  }
  case C1GRDSCC: // cp1  rd rs cc
  {
     fmt = sel_fpfmt ((opcode >> 21) & BITMASK4_0);
     rd  = (opcode >> 11) & BITMASK4_0;
     rs  = (opcode >> 21) & BITMASK4_0;
     cc  = (opcode >> 18)  & 0x7;
     sprintf (retbuf, "%s $%i, $%i, %i",
             p->n.name, rd, rs, cc);
     break;
  }
  case C1DIB: // cp1 fd findex(fbase)
    fd     = (opcode >> 6) & BITMASK4_0;
    findex = (opcode >> 16) & BITMASK4_0;
    fbase  = (opcode >> 21) & BITMASK4_0;
    sprintf (retbuf, "%s f%i, $%i($%i)", p->n.name, fd, findex, fbase);
    break;


  case C1SIB: // cp1 fs findex(fbase)
    fs     = (opcode >> 11) & BITMASK4_0;
    findex = (opcode >> 16) & BITMASK4_0;
    fbase  = (opcode >> 21) & BITMASK4_0;
    sprintf (retbuf, "%s f%i, $%i($%i)", p->n.name, fs, findex, fbase);
    break;

  case C1TOB:   // ft, offset(fbase)
    fbase = (opcode >> 21) & BITMASK4_0;
    ft = (opcode >> 16) & BITMASK4_0;

    sprintf (retbuf, "%s f%i, 0x%x($%i)", p->n.name, ft, opcode & 0xffff, fbase);
    break;


  case C1HIB: // cp1 fhint findex(fbase)
    fhint = (opcode >> 11) & BITMASK4_0;
    findex = (opcode >> 16) & BITMASK4_0;
    fbase  = (opcode >> 21) & BITMASK4_0;
    sprintf (retbuf, "%s %i, $%i($%i)", p->n.name, fhint, findex, fbase);
    break;

  case C1DSTS:  // cp1 fd fs ft rs
    fd = (opcode >> 6) & BITMASK4_0;
    fs = (opcode >> 11) & BITMASK4_0;
    ft = (opcode >> 16) & BITMASK4_0;
    rs = (opcode >> 21) & BITMASK4_0;
    sprintf (retbuf, "%s f%i, f%i, f%i, $%i", p->n.name, fd, fs, ft, rs);
    break;

  case C1DRST: // cp1 fd, fr, fs, ft
    fd = (opcode >> 6) & BITMASK4_0;
    fs = (opcode >> 11) & BITMASK4_0;
    ft = (opcode >> 16) & BITMASK4_0;
    fr = (opcode >> 21) & BITMASK4_0;
    sprintf (retbuf, "%s f%i, f%i, f%i, f%i", p->n.name, fd, fr, fs, ft);
    break;

  case C1BCCO:
     cc  = (opcode >> 18)  & 0x7;

     target = (pc+4) + ((opcode & 0xffff)<<2);
     if (opcode & 0x8000)
         target += 0xfffc0000;

     sprintf (retbuf, "%s%s %i, 0x%x",
             p->n.name, ((opcode >> 16) & 1) ? "T" : "F",
             cc, target);
     break;

  case UDISPC2:
  {
      int src_ac = (opcode >> 9) & BITMASK1_0;
      int dst_ac = (opcode >> 7) & BITMASK1_0;
      rs = (opcode >> 21) & BITMASK4_0;
      rt = (opcode >> 16) & BITMASK4_0;
      rd = (opcode >> 11) & BITMASK4_0;

      switch ( opcode & 0xf )
      {
          case 0x8:
              sprintf(retbuf, "SWPHI $%d, %d", rd, src_ac);
              break;
          case 0x9:
              sprintf(retbuf, "SWPLO $%d, %d", rd, src_ac);
              break;
          case 0xa:
              sprintf(retbuf, "SWPMTHILO %d, $%d, $%d", dst_ac, rs, rt);
              break;
          case 0xb:
              sprintf(retbuf, "SWPHILO %d, $%d, $%d", dst_ac, rs, rt);
              break;
          case 0xc:
              sprintf(retbuf, "SWPACCHILO %d, $%d, $%d, %d", dst_ac, rs, rt, src_ac);
              break;
          case 0xd:
              sprintf(retbuf, "SWPGPRHILO $%d, $%d, $%d, %d", rd, rs, rt, src_ac);
              break;
          default:
              sprintf (retbuf, "UDI Spec2(0%x)", opcode & BITMASK5_0);
    }
    break;
  }

  case BRK:
    sprintf (retbuf, "%-5s 0x%x", p->n.name, (opcode>>6) & 0xfffff);
    break;
  case RES:
    sprintf (retbuf, "UNK Main(0%x)", opcode >> 26);
    break;
  case RESSPC:
    sprintf (retbuf, "UNK Spec(0%x)", opcode & BITMASK5_0);
    break;
  case RESSPC2:
    sprintf (retbuf, "UNK Spec2(0%x)", opcode & BITMASK5_0);
    break;
  case RESSPC3:
    sprintf (retbuf, "UNK Spec3(0%x)", opcode & BITMASK5_0);
    break;
  case RESREG:
    sprintf (retbuf, "UNK RegImm(0%x)", (opcode >> 16) & BITMASK4_0);
    break;
  case RESCRS:
    sprintf (retbuf, "UNK Copz rs(0%x)", (opcode >> 21) & 0xf);
    break;
  case RESCRT:
    sprintf (retbuf, "UNK Copz rt(0%x)", (opcode >> 16) & BITMASK4_0);
    break;

  case M16_JAL:
    target = opcode & 0x3ffffff;

    //opcode&BITMASK15_0 | ((opcode >> 21) & BITMASK4_0) << 16 |
    //          ((opcode >> 16) & BITMASK4_0) << 21;

    target = ((pc + 4) & ~(uint32_t)0xfffffff) | ((target << 2) & (uint32_t)0xfffffff);
    sprintf (retbuf, "%-5s 0x%x", p->n.name, target);
    break;

#ifdef MIPS_MDMX
  case MDMX:
    mdmx_fmtsel(opcode, dtstr, vtpre, vtsuf);
    vd  = (opcode >> 6) & BITMASK4_0;
    vs  = (opcode >> 11) & BITMASK4_0;
    vt  = (opcode >> 16) & BITMASK4_0;
    sprintf (retbuf, "%s%s v%i, v%i, %s%i%s",
                         p->n.name, dtstr, vd, vs, vtpre, vt, vtsuf);
    break;

  case MDMX_AC:
    mdmx_fmtsel(opcode, dtstr, vtpre, vtsuf);
    vd  = (opcode >> 6) & BITMASK4_0;
    vs  = (opcode >> 11) & BITMASK4_0;
    vt  = (opcode >> 16) & BITMASK4_0;
    /* VD isn't really a register number, but the MSB indicates load vs. acc */
    if(!(vd & 0x10)) {
        sprintf (retbuf, "%sA%s v%i, %s%i%s",
                         p->n.name, dtstr, vs, vtpre, vt, vtsuf);
    } else {
        sprintf (retbuf, "%sL%s v%i, %s%i%s",
                         p->n.name, dtstr, vs, vtpre, vt, vtsuf);
    }
    break;

  case MDMX_SAC:
    /* Annoying special case due to irregular multiply-subtract mnemonic */
    mdmx_fmtsel(opcode, dtstr, vtpre, vtsuf);
    vd  = (opcode >> 6) & BITMASK4_0;
    vs  = (opcode >> 11) & BITMASK4_0;
    vt  = (opcode >> 16) & BITMASK4_0;
    /* VD isn't really a register number, but the MSB indicates load vs. acc */
    if(!(vd & 0x10)) {
        sprintf (retbuf, "%s%s v%i, %s%i%s",
                         p->n.name, dtstr, vs, vtpre, vt, vtsuf);
    } else {
        sprintf (retbuf, "%sL%s v%i, %s%i%s",
                         p->n.name, dtstr, vs, vtpre, vt, vtsuf);
    }
    break;

  case MDMX_ALNI:
    vd  = (opcode >> 6) & BITMASK4_0;
    vs  = (opcode >> 11) & BITMASK4_0;
    vt  = (opcode >> 16) & BITMASK4_0;
    sprintf (retbuf, "%s v%i, v%i, v%i, %i",
                         p->n.name, vd, vs, vt, ((opcode >> 21) & 0x7));
    break;

  case MDMX_ALNV:
    vd  = (opcode >> 6) & BITMASK4_0;
    vs  = (opcode >> 11) & BITMASK4_0;
    vt  = (opcode >> 16) & BITMASK4_0;
    sprintf (retbuf, "%s v%i, v%i, v%i, $%i",
                         p->n.name, vd, vs, vt, ((opcode >> 21) & 0x1f));
    break;

  case MDMX_C:
    mdmx_fmtsel(opcode, dtstr, vtpre, vtsuf);
    vd  = (opcode >> 6) & BITMASK4_0;
    vs  = (opcode >> 11) & BITMASK4_0;
    vt  = (opcode >> 16) & BITMASK4_0;
    if (vd != 0) {
        /* Hypothetical case of GPR used as condition code target */
        sprintf (retbuf, "%s%s v%i, v%i, %s%i%s",
                         p->n.name, dtstr, vd, vs, vtpre, vt, vtsuf);
    } else {
        sprintf (retbuf, "%s%s v%i, %s%i%s",
                         p->n.name, dtstr, vs, vtpre, vt, vtsuf);
    }
    break;

  case MDMX_R:
    mdmx_fmtsel(opcode, dtstr, vtpre, vtsuf);
    vd  = (opcode >> 6) & BITMASK4_0;
    vt  = (opcode >> 16) & BITMASK4_0;
    sprintf (retbuf, "%s%s v%i, %s%i%s",
                         p->n.name, dtstr, vd, vtpre, vt, vtsuf);
    break;

  case MDMX_RAC:
    mdmx_fmtsel(opcode, dtstr, vtpre, vtsuf);
    vd  = (opcode >> 6) & BITMASK4_0;
    switch((opcode >> 24) & 0x3) {
    case 0:
        sprintf (retbuf, "%sL%s v%i", p->n.name, dtstr, vd);
        break;
    case 1:
        sprintf (retbuf, "%sM%s v%i", p->n.name, dtstr, vd);
        break;
    case 2:
        sprintf (retbuf, "%sH%s v%i", p->n.name, dtstr, vd);
        break;
    default:
        sprintf (retbuf, "%s?%s v%i", p->n.name, dtstr, vd);
        break;
    }
    break;

  case MDMX_WAC:
    mdmx_fmtsel(opcode, dtstr, vtpre, vtsuf);
    vs  = (opcode >> 11) & BITMASK4_0;
    vt  = (opcode >> 16) & BITMASK4_0;
    switch((opcode >> 24) & 0x3) {
    case 0:
        sprintf (retbuf, "%sL%s v%i, v%i", p->n.name, dtstr, vs, vt);
        break;
    case 2:
        sprintf (retbuf, "%sH%s v%i", p->n.name, dtstr, vs);
        break;
    default:
        sprintf (retbuf, "%s?%s v%i", p->n.name, dtstr, vs);
        break;
    }
    break;

  case MDMX_SHFL:
    {
        /* Format code isn't the same as for others - shuffle op is encoded */
        uint32_t shflcode = (opcode >> 21) & BITMASK4_0;
        mdmx_fmtsel(opcode, dtstr, vtpre, vtsuf);
        vd  = (opcode >> 6) & BITMASK4_0;
        vs  = (opcode >> 11) & BITMASK4_0;
        vt  = (opcode >> 16) & BITMASK4_0;
        sprintf (retbuf, "%s%s%s v%i, v%i, v%i",
                         p->n.name, shflfmt[shflcode], dtstr, vd, vs, vt);
    break;
    }


#endif // MIPS_MDMX


  default:
    sprintf (retbuf, "?DW? 0x%x", opcode);
    break;

  };

  return retbuf;
}

//-----------------------------mips16------------------------------------

//////////////////////////////////////////////////////////////
#define Xlat(i) ( (i)<2 ? (i)+16: (i) )

#define DECODE_NEXT(TABLE, INDEX) \
     const struct iType  *p = (TABLE)+(INDEX);                     \
     if (p->fmt == FUN)                                            \
       return (*(p->n.f))(opcode);                                 \
     else                                                          \
       return p;

const struct iType  * DecodeMIPS16_SHIFT(uint32_t opcode)
{
   uint32_t  idx = opcode & 0x3;
   DECODE_NEXT(M16ShiftTable, idx);
}

const struct iType  * DecodeMIPS16_RRIA(uint32_t opcode)
{
   uint32_t  idx = (opcode >> 4) & 1;
   DECODE_NEXT(M16RRIATable, idx);
}

const struct iType  * DecodeMIPS16_RRR(uint32_t opcode)
{
   uint32_t  idx = opcode & 0x3;
   DECODE_NEXT(M16RRRTable, idx);
}

const struct iType  * DecodeMIPS16_I8(uint32_t opcode)
{
   uint32_t  idx = (opcode >> 8) & 0x7;
   DECODE_NEXT(M16I8Table, idx);
}

const struct iType  * DecodeMIPS16_I64(uint32_t opcode)
{
   uint32_t  idx = (opcode >> 8) & 0x7;
   DECODE_NEXT(M16I64Table, idx);
}

const struct iType  * DecodeMIPS16_RR(uint32_t opcode)
{
   uint32_t  idx = opcode & BITMASK4_0;
   DECODE_NEXT(M16RRTable, idx);
}

/////////////////////////////////////////////////////////////////
const char* disasm_mips16(uint32_t opcode, uint32_t pc)
{
  static char  retbuf[256] = "mips16  ";
  char *cbuf = retbuf + 8;

  uint32_t rx, ry, rz, r32, sa, ra, ac, immediate, target, func, sign_bit, offset;
  uint32_t framesize, s0, s1, xsregs, aregs;

  char *name;

  uint32_t op = MIPS16_INSTR_INDEX(opcode);
  const struct iType *p = M16Table + op;

  if ( p->fmt == FUN) // call decoder on the nested level
  {
    p =  (*(p->n.f) )(opcode);
  }

  switch (p->fmt)
  {
  case M16_R:
  {
     rx=(opcode>>8)&BITMASK2_0;

     sprintf (cbuf, "%-5s $%i", p->n.name, Xlat(rx));
     break;
  }

  case M16_RR:
  {
     rx=(opcode>>8)&BITMASK2_0;
     ry=(opcode>>5)&BITMASK2_0;

     sprintf (cbuf, "%-5s $%i, $%i", p->n.name, Xlat(rx), Xlat(ry));
     break;
  }

  case M16_RR_YX:
  {
     rx=(opcode>>8)&BITMASK2_0;
     ry=(opcode>>5)&BITMASK2_0;

     sprintf (cbuf, "%-5s $%i, $%i", p->n.name, Xlat(ry), Xlat(rx));
     break;
  }

  case M16_CNVT:
  {
     rx=(opcode>>8)&BITMASK2_0;
     ry=(opcode>>5)&BITMASK2_0;

     switch (ry)
     {
     case 0: name = "ZEB";
             break;
     case 1: name = "ZEH";
             break;
     case 2: name = "ZEW";
             break;
     case 4: name = "SEB";
             break;
     case 5: name = "SEH";
             break;
     case 6: name = "SEW";
             break;
     default:
             name = p->n.name;
     }

     sprintf (cbuf, "%s $%i", name, Xlat(rx));
     break;
  }

  case M16_RRR:
  {
      if (IS_EXTENDED(opcode)) {
          // ASMACRO
          int select = (opcode >> 24) & 0x7 ;
          int p0 = (opcode >> 0) & 0x1f ;
          int p1 = (opcode >> 5) & 0x7 ;
          int p2 = (opcode >> 8) & 0x7 ;
          int p3 = (opcode >> 16) & 0x1f ;
          int p4 = (opcode >> 21) & 0x7 ;
          sprintf (cbuf, "ASMACRO 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", select, p0, p1, p2, p3, p4);
      } else {
          rx=(opcode>>8)&BITMASK2_0;
          ry=(opcode>>5)&BITMASK2_0;
          rz=(opcode>>2)&BITMASK2_0;

          sprintf (cbuf, "%-5s $%i, $%i, $%i", p->n.name, Xlat(rz), Xlat(rx), Xlat(ry));
      }
      break;
  }

  case M16_RI:
  case M16_BRI:
  {
      rx=(opcode>>8)&BITMASK2_0;
      immediate=opcode & BITMASK7_0;
      if (IS_EXTENDED(opcode))
        immediate |= ((opcode>>21)&BITMASK5_0)<<5 | ((opcode>>16)&BITMASK4_0)<<11;

      if (p->fmt == M16_BRI)
      {
         sign_bit = 8;
         offset   = 2;
         if (IS_EXTENDED(opcode))
         {
            sign_bit = 16;
            offset   = 4;
         }

         SIGNEXTEND(immediate, sign_bit );
         immediate = (immediate<<1) +  offset + pc;
      }

      sprintf (cbuf, "%-5s $%i, 0x%x", p->n.name, Xlat(rx), immediate);

      break;
  }

  case M16_RI64:
  {
      ry=(opcode>>5)&BITMASK2_0;
      immediate=opcode & BITMASK4_0;
      if (IS_EXTENDED(opcode))
         immediate |= ((opcode>>21)&BITMASK5_0)<<5 | ((opcode>>16)&BITMASK4_0)<<11;

      sprintf (cbuf, "%-5s $%i, 0x%x", p->n.name, Xlat(ry), immediate);
      break;
  }

  case M16_RRI:
  {
      rx=(opcode>>8)&BITMASK2_0;
      ry=(opcode>>5)&BITMASK2_0;
      immediate=opcode & BITMASK4_0;
      if (IS_EXTENDED(opcode))
         immediate |= ((opcode>>21)&BITMASK5_0)<<5 | ((opcode>>16)&BITMASK4_0)<<11;

      sprintf (cbuf, "%-5s $%i, $%i, 0x%x", p->n.name, Xlat(rx), Xlat(ry), immediate);
      break;
  }

  case M16_RRI_YOX:
  {
      rx=(opcode>>8)&BITMASK2_0;
      ry=(opcode>>5)&BITMASK2_0;
      immediate=opcode & BITMASK4_0;
      if (IS_EXTENDED(opcode))
         immediate |= ((opcode>>21)&BITMASK5_0)<<5 | ((opcode>>16)&BITMASK4_0)<<11;

      sprintf (cbuf, "%-5s $%i, 0x%x($%i)", p->n.name, Xlat(ry), immediate, Xlat(rx));
      break;
  }

  case M16_RRIA:
  {
     rx=(opcode>>8)&BITMASK2_0;
     ry=(opcode>>5)&BITMASK2_0;
     immediate=opcode & BITMASK3_0;
     if (IS_EXTENDED(opcode))
        immediate |= ((opcode>>20)&BITMASK6_0)<<4 | ((opcode>>16)&BITMASK3_0)<<11;

      sprintf (cbuf, "%-5s $%i, $%i, 0x%x", p->n.name, Xlat(ry), Xlat(rx), immediate);
      break;
  }

  case M16_I:
  {
      immediate=opcode & BITMASK10_0;
      if (IS_EXTENDED(opcode))
      immediate = (immediate & BITMASK4_0) | ((opcode>>21)&BITMASK5_0)<<5 |
                  ((opcode>>16)&BITMASK4_0)<<11;
      sign_bit = 11;
      offset   = 2;
      if (IS_EXTENDED(opcode))
      {
           sign_bit = 16;
           offset   = 4;
      }

      SIGNEXTEND(immediate, sign_bit);
      immediate = (immediate<<1) + offset + pc;
      sprintf (cbuf, "%-5s 0x%x", p->n.name, immediate);
      break;
  }

  case M16_I64:
  case M16_I8:
  case M16_BI8:
  {
      func = (opcode>>8)&BITMASK2_0;
      immediate=opcode & BITMASK7_0;
      if (IS_EXTENDED(opcode))
      immediate = (immediate & BITMASK4_0) | ((opcode>>21)&BITMASK5_0)<<5 |
                ((opcode>>16)&BITMASK4_0)<<11;
      if (p->fmt == M16_BI8)
      {
         sign_bit = 8;
         offset   = 2;
         if (IS_EXTENDED(opcode))
         {
            sign_bit = 16;
            offset   = 4;
         }

          SIGNEXTEND(immediate, sign_bit );
          immediate = (immediate<<1) +  offset + pc;
      }

      sprintf (cbuf, "%-5s 0x%x", p->n.name, immediate);
      break;
  }

  case M16_SAVRES:
  {
      framesize=opcode & BITMASK3_0;
      ra = (opcode & (1<<6)) != 0;
      s0 = (opcode & (1<<5)) != 0;
      s1 = (opcode & (1<<4)) != 0;
      xsregs = 0;
      aregs  = 0;
      if (opcode & (1<<7)) // save
         name = "SAVE";
      else // restore
         name = "RESTORE";

      char abuf[64];
      if (IS_EXTENDED(opcode))
      {
           framesize = framesize  | (((opcode>>20) &BITMASK3_0 )<<4) ;
           xsregs    = ( opcode >> 24 ) & BITMASK2_0;
           aregs     = ( opcode >> 16 ) & BITMASK3_0;

           sprintf ( abuf, ", xsregs(%i), aregs(%i)", xsregs, aregs );
      }

      sprintf (cbuf, "%-5s fsize(%i), ra(%i), s0(%i), s1(%i)", name, framesize, ra, s0, s1);
      if (IS_EXTENDED(opcode))
          strcat(cbuf, abuf );

      break;
  }

  case M16_JAL:
  {
      target = (opcode & BITMASK15_0) | ((opcode >> 21) & BITMASK4_0) << 16 |
                ((opcode >> 16) & BITMASK4_0) << 21;
      if ((opcode & (1<<26)) == 0)
         name = p->n.name;
      else
         name = "JALX";
      target = ((pc + 4) & ~(uint32_t)0xfffffff) | ((target << 2) & (uint32_t)0xfffffff);
      sprintf (cbuf, "%-5s 0x%x", name, target);
      break;
  }

  case M16_JALR:
  {
      rx=(opcode>>8)&BITMASK2_0;
      ra=(opcode>>5)&BITMASK2_0;

      switch (ra)
      {
         case 0: sprintf (cbuf, "JR $%i", Xlat(rx));
                 break;
         case 1: sprintf (cbuf, "JR $31");
                 break;
         case 2: sprintf (cbuf, "JALR $31, $%i", Xlat(rx));
                 break;
         case 4: sprintf (cbuf, "JRC $%i", Xlat(rx));
                 break;
         case 5: sprintf (cbuf, "JRC $31");
                 break;
         case 6: sprintf (cbuf, "JALRC $31, $%i", Xlat(rx));
                 break;

         default:
                 sprintf (cbuf, "%-5s $%i, $%i", p->n.name, Xlat(rx), ra);
      }
      break;
  }

  case M16_SR:
  {
      ry=(opcode>>5)&BITMASK2_0;
      sa=(opcode>>8)&BITMASK2_0;
      if (IS_EXTENDED(opcode))
         sa = ((opcode >> 22) & BITMASK4_0) | (((opcode >> 21) & 1) << 5);
      else
         sa = sa == 0 ? 8 : sa;

      sprintf (cbuf, "%-5s $%i, %i", p->n.name, Xlat(ry), sa);
      break;
  }

  case M16_DSHIFT:
  {
      rx=(opcode>>8)&BITMASK2_0;
      ry=(opcode>>5)&BITMASK2_0;
      sa=(opcode>>2)&BITMASK2_0;
      if (IS_EXTENDED(opcode))
        sa = ((opcode >> 22) & BITMASK4_0) ;

      sprintf (cbuf, "%-5s $%i, $%i, %i", p->n.name, Xlat(rx), Xlat(ry), sa);
      break;
  }

  case M16_MOV32R:
  {
      rz=opcode & BITMASK2_0;
      r32=((opcode>>5)&BITMASK2_0) | (opcode & (BITMASK1_0 << 3));

      if (r32 == 0)
         sprintf (cbuf, "NOP");
      else
         sprintf (cbuf, "%-5s $%i, $%i", p->n.name, r32, Xlat(rz));

      break;
  }

  case M16_MOV32:
  {
      ry=(opcode>>5) & BITMASK2_0;
      r32=opcode & BITMASK4_0;

      sprintf (cbuf, "%-5s $%i, $%i", p->n.name, Xlat(ry), r32);

      break;
  }

  case M16_RAC:
  {
      rx=(opcode>>8) & BITMASK2_0;
      ac=opcode & BITMASK1_0;

      if (ac)
          sprintf (cbuf, "%-5s $%i, %d", p->n.name, Xlat(rx), ac);
      else
          sprintf (cbuf, "%-5s $%i", p->n.name, Xlat(rx));
      break;
  }

  case BRK:
  {
      sprintf (cbuf, "%-5s 0x%x", p->n.name, (opcode>>5) & BITMASK5_0);
      break;
  }

  default:
  {
      sprintf (cbuf, "UNK 0x%x", opcode );
      break;
  }
  }

  // print additional attributes
  //strcat(cbuf, " M16");
  if( IS_EXTENDED(opcode)) strcat(cbuf, " Extended" );

  return retbuf;
}
