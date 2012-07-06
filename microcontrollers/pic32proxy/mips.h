/*
 * Instructions and registers of MIPS32 processor.
 *
 * Copyright (C) 2011 Serge Vakulenko
 *
 * This file is part of PIC32PROXY project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */

/*
 * Opcodes for some mips instructions
 */
#define	MIPS_JR		0x8		/* jr rs; */
#define	MIPS_JAL	(0x3 << 26)	/* jal 0; jump and link */
#define	MIPS_ADD	0x20		/* add rd, rs, rt */
#define	MIPS_ADDI	(0x8 << 26)	/* addi rt << 16, rs << 21, imed */
#define	MIPS_ADDIU	(0x8 << 26)	/* addiu rt << 16, rs << 21, imed */
#define MIPS_MFHI	0x10		/* mfhi, (rd << 11) */
#define MIPS_MFLO	0x12		/* mflo, (rd << 11) */
#define MIPS_MTHI	0x11		/* mthi, (rd << 21) */
#define MIPS_MTLO	0x13		/* mtlo, (rd << 21) */
#define MIPS_SLL	0x0		/* sll dest << 11, src << 16, sa << 6 */
#define MIPS_SRL	(0x0 | 0x3)	/* srl dest << 11, src << 16, sa << 6 */
#define MIPS_NOP	MIPS_SLL	/* nop (SLL, r0, r0, 0) */
#define	MIPS_OR		37		/* add rd, rs, rt */
#define MIPS_ORI	(0xd << 26)	/* ori rt << 16, rs << 21, imed */
#define MIPS_MFC0	(0x10 << 26)	/* mfc0 rt << 16, rd << 11, sel */
#define MIPS_MTC0	((0x10<<26)|(4<<21))/* mtc0 rt << 16, rd << 11, sel */
#define MIPS_TLBP	0x42000008	/* tlbp */
#define MIPS_TLBR	0X42000001	/* tlbr */
#define MIPS_TLBWI	0X42000002	/* tlbwi */
#define MIPS_TLBWR	0X42000006	/* tlbwr */
#define MIPS_LUI	(15 << 26)	/* lui (r << 16) imm */
#define MIPS_SW		(0x2b << 26)	/* sw (rbase << 21) (rto << 16) offt */
#define MIPS_LW		(0x23 << 26)	/* lw (rbase << 21) (rto << 16) offt */
#define MIPS_SWC1	(0x39 << 26)	/* swc1 (rbase << 21) (rto << 16) offt */
#define MIPS_LWC1	(0x31 << 26)	/* lwc1 (rbase << 21) (rto << 16) offt */
#define MIPS_MTC1	((0x11<<26)|(4<<21))/* mtc0 rt << 16, rd << 11, sel */
#define MIPS_CTC1	((0x11<<26)|(6<<21))/* ctc1 rt << 16, fs << 11 */
#define MIPS_CFC1	((0x11<<26)|(2<<21))/* cfc1 rt << 16, fs << 11 */

#define MIPS32_OP_BEQ	0x04
#define MIPS32_OP_BNE	0x05
#define MIPS32_OP_ADDI	0x08
#define MIPS32_OP_AND	0x24
#define MIPS32_OP_COP0	0x10
#define MIPS32_OP_JR	0x08
#define MIPS32_OP_LUI	0x0F
#define MIPS32_OP_LW	0x23
#define MIPS32_OP_LBU	0x24
#define MIPS32_OP_LHU	0x25
#define MIPS32_OP_MFHI	0x10
#define MIPS32_OP_MTHI	0x11
#define MIPS32_OP_MFLO	0x12
#define MIPS32_OP_MTLO	0x13
#define MIPS32_OP_SB	0x28
#define MIPS32_OP_SH	0x29
#define MIPS32_OP_SW	0x2B
#define MIPS32_OP_ORI	0x0D
#define MIPS32_OP_XOR	0x26
#define MIPS32_OP_SRL	0x03

#define MIPS32_COP0_MF	0x00
#define MIPS32_COP0_MT	0x04

#define MIPS32_R_INST(opcode, rs, rt, rd, shamt, funct)	\
                        (((opcode) << 26) | ((rs) << 21) | ((rt) << 16) | \
                         ((rd) << 11)| ((shamt) << 6) | (funct))

#define MIPS32_I_INST(opcode, rs, rt, immd)	\
                        (((opcode) << 26) | ((rs) << 21) | ((rt) << 16) | (immd))

#define MIPS32_J_INST(opcode, addr)	\
                        (((opcode) << 26) | (addr))

#define MIPS32_NOP					0
#define MIPS32_ADDI(tar, src, val)	MIPS32_I_INST(MIPS32_OP_ADDI, src, tar, val)
#define MIPS32_AND(reg, off, val)	MIPS32_R_INST(0, off, val, reg, 0, MIPS32_OP_AND)
#define MIPS32_B(off)			MIPS32_BEQ(0, 0, off)
#define MIPS32_BEQ(src,tar,off)		MIPS32_I_INST(MIPS32_OP_BEQ, src, tar, off)
#define MIPS32_BNE(src,tar,off)		MIPS32_I_INST(MIPS32_OP_BNE, src, tar, off)
#define MIPS32_JR(reg)			MIPS32_R_INST(0, reg, 0, 0, 0, MIPS32_OP_JR)
#define MIPS32_MFC0(gpr, cpr, sel)	MIPS32_R_INST(MIPS32_OP_COP0, MIPS32_COP0_MF, gpr, cpr, 0, sel)
#define MIPS32_MTC0(gpr,cpr, sel)	MIPS32_R_INST(MIPS32_OP_COP0, MIPS32_COP0_MT, gpr, cpr, 0, sel)
#define MIPS32_LBU(reg, off, base)	MIPS32_I_INST(MIPS32_OP_LBU, base, reg, off)
#define MIPS32_LHU(reg, off, base)	MIPS32_I_INST(MIPS32_OP_LHU, base, reg, off)
#define MIPS32_LUI(reg, val)		MIPS32_I_INST(MIPS32_OP_LUI, 0, reg, val)
#define MIPS32_LW(reg, off, base)	MIPS32_I_INST(MIPS32_OP_LW, base, reg, off)
#define MIPS32_MFLO(reg)		MIPS32_R_INST(0, 0, 0, reg, 0, MIPS32_OP_MFLO)
#define MIPS32_MFHI(reg)		MIPS32_R_INST(0, 0, 0, reg, 0, MIPS32_OP_MFHI)
#define MIPS32_MTLO(reg)		MIPS32_R_INST(0, reg, 0, 0, 0, MIPS32_OP_MTLO)
#define MIPS32_MTHI(reg)		MIPS32_R_INST(0, reg, 0, 0, 0, MIPS32_OP_MTHI)
#define MIPS32_ORI(tar, src, val)	MIPS32_I_INST(MIPS32_OP_ORI, src, tar, val)
#define MIPS32_SB(reg, off, base)	MIPS32_I_INST(MIPS32_OP_SB, base, reg, off)
#define MIPS32_SH(reg, off, base)	MIPS32_I_INST(MIPS32_OP_SH, base, reg, off)
#define MIPS32_SW(reg, off, base)	MIPS32_I_INST(MIPS32_OP_SW, base, reg, off)
#define MIPS32_XOR(reg, val1, val2)	MIPS32_R_INST(0, val1, val2, reg, 0, MIPS32_OP_XOR)
#define MIPS32_SRL(reg, src, off)	MIPS32_R_INST(0, 0, src, reg, off, MIPS32_OP_SRL)

/* ejtag specific instructions */
#define MIPS32_DRET			0x4200001F
#define MIPS32_SDBBP			0x7000003F
#define MIPS16_SDBBP			0xE801

#define MIPS32_PRACC_FASTDATA_AREA	0xFF200000
#define MIPS32_PRACC_FASTDATA_SIZE	16
#define MIPS32_PRACC_TEXT		0xFF200200
#define MIPS32_PRACC_STACK		0xFF204000
#define MIPS32_PRACC_PARAM_IN		0xFF201000
#define MIPS32_PRACC_PARAM_IN_SIZE	0x1000
#define MIPS32_PRACC_PARAM_OUT		(MIPS32_PRACC_PARAM_IN + MIPS32_PRACC_PARAM_IN_SIZE)
#define MIPS32_PRACC_PARAM_OUT_SIZE	0x1000

#define UPPER16(uint32_t) 		(uint32_t >> 16)
#define LOWER16(uint32_t) 		(uint32_t & 0xFFFF)
#define NEG16(v) 			(((~(v)) + 1) & 0xFFFF)

/*
 * CP0 defines
 */
#define CP0_INDEX	0
#define CP0_RANDOM	1
#define CP0_ENTRYLO0	2
#define CP0_ENTRYLO1	3
#define CP0_CONTEXT	4
#define CP0_PAGEMASK	5
#define CP0_WIRED	6
#define CP0_7		7
#define CP0_BADVADDR	8
#define CP0_COUNT	9
#define CP0_ENTRYHI	10
#define CP0_COMPARE	11
#define CP0_STATUS	12
#define CP0_CAUSE	13
#define CP0_EPC		14
#define CP0_PRID	15
#define CP0_CONFIG	16
#define CP0_LLADDR	17
#define CP0_WATCHLO	18
#define CP0_WATCHHI	19
#define CP0_20		20
#define CP0_21		21
#define CP0_22		22
#define CP0_DEBUG	23
#define CP0_DEPC	24
#define CP0_PERFCNT	25
#define CP0_ERRCTL	26
#define CP0_CACHEERR	27
#define CP0_TAGLO	28
#define CP0_TAGHI	29
#define CP0_ERROREPC	30
#define CP0_DESAVE	31

/*
 * Status register.
 */
#define ST_IE		0x00000001	/* Interrupt enable */
#define ST_EXL		0x00000002	/* Exception level */
#define ST_ERL		0x00000004	/* Error level */
#define ST_UM		0x00000010	/* User mode */
#define ST_IM_SW0	0x00000100	/* Software interrupt mask 0 */
#define ST_IM_SW1	0x00000200	/* Software interrupt mask 1 */
#define ST_IM_IRQ0	0x00000400	/* External interrupt mask 0 */
#define ST_IM_IRQ1	0x00000800	/* External interrupt mask 1 */
#define ST_IM_IRQ2	0x00001000	/* External interrupt mask 2 */
#define ST_IM_IRQ3	0x00002000	/* External interrupt mask 3 */
#define ST_IM_IRQ4	0x00004000	/* External interrupt mask 3 */
#define ST_IM_IRQ5	0x00008000	/* External interrupt mask 3 */
#define ST_NMI		0x00080000	/* Reset caused by NMI */
#define ST_TS		0x00200000	/* TLB shutdown */
#define ST_BEV		0x00400000	/* Boot exception vectors */

#define ST_CU0		0x10000000	/* Enable coprocessor 0 */
#define ST_CU1		0x20000000	/* Enable coprocessor 1 (FPU) */

/*
 * Coprocessor 1 (FPU) registers.
 */
#define CP1_FIR		0	/* implementation and revision */
#define CP1_FCCR	25	/* condition codes */
#define CP1_FEXR	26	/* exceptions */
#define CP1_FENR	28	/* enables */
#define CP1_FCSR	31	/* control/status */
