/*
 * Hardware register defines for Microchip PIC32MZ microcontrollers.
 *
 * Copyright (C) 2013 Serge Vakulenko, <serge@vak.ru>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#ifndef _IO_PIC32MZ_H
#define _IO_PIC32MZ_H

/*--------------------------------------
 * Coprocessor 0 registers.
 */
#define C0_INDEX            0   /* Index into the TLB array */
#define C0_RANDOM           1   /* Randomly generated index into the TLB array */
#define C0_ENTRYLO0         2   /* Low-order portion of the TLB entry for
                                 * even-numbered virtual pages */
#define C0_ENTRYLO1         3   /* Low-order portion of the TLB entry for
                                 * odd-numbered virtual pages */
#define C0_CONTEXT          4   /* Pointer to the page table entry in memory */
#define C0_USERLOCAL        4   /* (1) User information that can be written by
                                 * privileged software and read via the RDHWR instruction */
#define C0_PAGEMASK         5   /* Variable page sizes in TLB entries */
#define C0_PAGEGRAIN        5   /* (1) Support of 1 KB pages in the TLB */
#define C0_WIRED            6   /* Controls the number of fixed TLB entries */
#define C0_HWRENA           7   /* Enables access via the RDHWR instruction
                                 * to selected hardware registers in Non-privileged mode */
#define C0_BADVADDR         8   /* Reports the address for the most recent
                                 * address-related exception */
#define C0_COUNT            9   /* Processor cycle count */
#define C0_ENTRYHI          10  /* High-order portion of the TLB entry */
#define C0_COMPARE          11  /* Core timer interrupt control */
#define C0_STATUS           12  /* Processor status and control */
#define C0_INTCTL           12  /* (1) Interrupt control of vector spacing */
#define C0_SRSCTL           12  /* (2) Shadow register set control */
#define C0_SRSMAP           12  /* (3) Shadow register mapping control */
#define C0_VIEW_IPL         12  /* (4) Allows the Priority Level to be read/written
                                 * without extracting or inserting that bit from/to the Status register */
#define C0_SRSMAP2          12  /* (5) Contains two 4-bit fields that provide
                                 * the mapping from a vector number to
                                 * the shadow set number to use when servicing such an interrupt */
#define C0_CAUSE            13  /* Describes the cause of the last exception */
#define C0_NESTEDEXC        13  /* (1) Contains the error and exception level
                                 * status bit values that existed prior to the current exception */
#define C0_VIEW_RIPL        13  /* (2) Enables read access to the RIPL bit that
                                 * is available in the Cause register */
#define C0_EPC              14  /* Program counter at last exception */
#define C0_NESTEDEPC        14  /* (1) Contains the exception program counter
                                 * that existed prior to the current exception */
#define C0_PRID             15  /* Processor identification and revision */
#define C0_EBASE            15  /* (1) Exception base address of exception vectors */
#define C0_CDMMBASE         15  /* (2) Common device memory map base */
#define C0_CONFIG           16  /* Configuration register */
#define C0_CONFIG1          16  /* (1) Configuration register 1 */
#define C0_CONFIG2          16  /* (2) Configuration register 2 */
#define C0_CONFIG3          16  /* (3) Configuration register 3 */
#define C0_CONFIG4          16  /* (4) Configuration register 4 */
#define C0_CONFIG5          16  /* (5) Configuration register 5 */
#define C0_CONFIG7          16  /* (7) Configuration register 7 */
#define C0_LLADDR           17  /* Load link address */
#define C0_WATCHLO          18  /* Low-order watchpoint address */
#define C0_WATCHHI          19  /* High-order watchpoint address */
#define C0_DEBUG            23  /* EJTAG debug register */
#define C0_TRACECONTROL     23  /* (1) EJTAG trace control */
#define C0_TRACECONTROL2    23  /* (2) EJTAG trace control 2 */
#define C0_USERTRACEDATA1   23  /* (3) EJTAG user trace data 1 register */
#define C0_TRACEBPC         23  /* (4) EJTAG trace breakpoint register */
#define C0_DEBUG2           23  /* (5) Debug control/exception status 1 */
#define C0_DEPC             24  /* Program counter at last debug exception */
#define C0_USERTRACEDATA2   24  /* (1) EJTAG user trace data 2 register */
#define C0_PERFCTL0         25  /* Performance counter 0 control */
#define C0_PERFCNT0         25  /* (1) Performance counter 0 */
#define C0_PERFCTL1         25  /* (2) Performance counter 1 control */
#define C0_PERFCNT1         25  /* (3) Performance counter 1 */
#define C0_ERRCTL           26  /* Software test enable of way-select and data
                                 * RAM arrays for I-Cache and D-Cache */
#define C0_TAGLO            28  /* Low-order portion of cache tag interface */
#define C0_DATALO           28  /* (1) Low-order portion of cache tag interface */
#define C0_ERROREPC         30  /* Program counter at last error exception */
#define C0_DESAVE           31  /* Debug exception save */

/*
 * Status register.
 */
#define ST_CU0		0x10000000	/* Access to coprocessor 0 allowed (in user mode) */
#define ST_RP		0x08000000	/* Enable reduced power mode */
#define ST_RE		0x02000000	/* Reverse endianness (in user mode) */
#define ST_BEV		0x00400000	/* Exception vectors: bootstrap */
#define ST_SR		0x00100000	/* Soft reset */
#define ST_NMI		0x00080000	/* NMI reset */
#define ST_IPL(x)	((x) << 10)	/* Current interrupt priority level */
#define ST_UM		0x00000010	/* User mode */
#define ST_ERL		0x00000004	/* Error level */
#define ST_EXL		0x00000002	/* Exception level */
#define ST_IE		0x00000001	/* Interrupt enable */

/*
 * Сause register.
 */
#define CA_BD		0x80000000	/* Exception occured in delay slot */
#define CA_TI		0x40000000	/* Timer interrupt is pending */
#define CA_CE		0x30000000	/* Coprocessor exception */
#define CA_DC		0x08000000	/* Disable COUNT register */
#define CA_IV		0x00800000	/* Use special interrupt vector 0x200 */
#define CA_RIPL(r)	((r)>>10 & 63)	/* Requested interrupt priority level */
#define CA_IP1		0x00020000	/* Request software interrupt 1 */
#define CA_IP0		0x00010000	/* Request software interrupt 0 */
#define CA_EXC_CODE	0x0000007c	/* Exception code */

#define CA_Int		0		/* Interrupt */
#define CA_AdEL		(4 << 2)	/* Address error, load or instruction fetch */
#define CA_AdES		(5 << 2)	/* Address error, store */
#define CA_IBE		(6 << 2)	/* Bus error, instruction fetch */
#define CA_DBE		(7 << 2)	/* Bus error, load or store */
#define CA_Sys		(8 << 2)	/* Syscall */
#define CA_Bp		(9 << 2)	/* Breakpoint */
#define CA_RI		(10 << 2)	/* Reserved instruction */
#define CA_CPU		(11 << 2)	/* Coprocessor unusable */
#define CA_Ov		(12 << 2)	/* Arithmetic overflow */
#define CA_Tr		(13 << 2)	/* Trap */

#define DB_DBD          (1 << 31)       /* Debug exception in a branch delay slot */
#define DB_DM           (1 << 30)       /* Debug mode */
#define DB_NODCR        (1 << 29)       /* No dseg present */
#define DB_LSNM         (1 << 28)       /* Load/stores in dseg go to main memory */
#define DB_DOZE         (1 << 27)       /* Processor was in low-power mode */
#define DB_HALT         (1 << 26)       /* Internal system bus clock was running */
#define DB_COUNTDM      (1 << 25)       /* Count register is running in Debug mode */
#define DB_IBUSEP       (1 << 24)       /* Instruction fetch bus error exception */
#define DB_DBUSEP       (1 << 21)       /* Data access bus error exception */
#define DB_IEXI         (1 << 20)       /* Imprecise error exception */
#define DB_VER          (7 << 15)       /* EJTAG version number */
#define DB_DEXCCODE     (0x1f << 10)    /* Cause of exception in Debug mode */
#define DB_SST          (1 << 8)        /* Single step exception enabled */
#define DB_DIBImpr      (1 << 6)        /* Imprecise debug instruction break */
#define DB_DINT         (1 << 5)        /* Debug interrupt exception */
#define DB_DIB          (1 << 4)        /* Debug instruction break exception */
#define DB_DDBS         (1 << 3)        /* Debug data break exception on store */
#define DB_DDBL         (1 << 2)        /* Debug data break exception on load */
#define DB_DBP          (1 << 1)        /* Debug software breakpoint exception */
#define DB_DSS          (1 << 0)        /* Debug single-step exception */

/*
 * Read C0 coprocessor register.
 */
#define mfc0(reg, sel) ({ int __value;				\
	asm volatile (						\
	"mfc0	%0, $%1, %2"					\
	: "=r" (__value) : "K" (reg), "K" (sel));		\
	__value; })

/*
 * Write coprocessor 0 register.
 */
#define mtc0(reg, sel, value) asm volatile (                    \
	"mtc0	%z0, $%1, %2"                                   \
	: : "r" ((unsigned) (value)), "K" (reg), "K" (sel))

/*--------------------------------------
 * Configuration registers.
 */
#define PIC32_DEVCFG(cfg0, cfg1, cfg2, cfg3) \
    unsigned __DEVCFG0 __attribute__ ((section (".config0"))) = (cfg0) ^ 0x7fffffff; \
    unsigned __DEVCFG1 __attribute__ ((section (".config1"))) = (cfg1) | DEVCFG1_UNUSED; \
    unsigned __DEVCFG2 __attribute__ ((section (".config2"))) = (cfg2) | DEVCFG2_UNUSED; \
    unsigned __DEVCFG3 __attribute__ ((section (".config3"))) = (cfg3) | DEVCFG3_UNUSED

/*
 * Config0 register at 1fc0ffcc, inverted.
 */
#define DEVCFG0_DEBUG_ENABLE    0x00000002 /* Enable background debugger */
#define DEVCFG0_JTAG_DISABLE    0x00000004 /* Disable JTAG port */
#define DEVCFG0_ICESEL_PGE2     0x00000008 /* Use PGC2/PGD2 (default PGC1/PGD1) */
#define DEVCFG0_TRC_DISABLE     0x00000020 /* Disable Trace port */
#define DEVCFG0_MICROMIPS       0x00000040 /* Boot in microMIPS mode */
#define DEVCFG0_ECC_MASK        0x00000300 /* Flash ECC mode mask */
#define DEVCFG0_ECC_ENABLE      0x00000300 /* Enable Flash ECC */
#define DEVCFG0_DECC_ENABLE     0x00000200 /* Enable Dynamic Flash ECC */
#define DEVCFG0_ECC_DIS_LOCK    0x00000100 /* Disable ECC, lock ECCCON */
#define DEVCFG0_FSLEEP          0x00000400 /* Flash power down controlled by VREGS bit */
#define DEVCFG0_DBGPER0         0x00001000 /* In Debug mode, deny CPU access to
                                            * Permission Group 0 permission regions */
#define DEVCFG0_DBGPER1         0x00002000 /* In Debug mode, deny CPU access to
                                            * Permission Group 1 permission regions */
#define DEVCFG0_DBGPER2         0x00004000 /* In Debug mode, deny CPU access to
                                            * Permission Group 2 permission regions */
#define DEVCFG0_EJTAG_REDUCED   0x40000000 /* Reduced EJTAG functionality */

/*
 * Config1 register at 1fc0ffc8.
 */
#define DEVCFG1_UNUSED          0x00003800
#define DEVCFG1_FNOSC_MASK      0x00000007 /* Oscillator selection */
#define DEVCFG1_FNOSC_SPLL      0x00000001 /* SPLL */
#define DEVCFG1_FNOSC_POSC      0x00000002 /* Primary oscillator XT, HS, EC */
#define DEVCFG1_FNOSC_SOSC      0x00000004 /* Secondary oscillator */
#define DEVCFG1_FNOSC_LPRC      0x00000005 /* Low-power RC */
#define DEVCFG1_FNOSC_FRCDIV    0x00000007 /* Fast RC with divide-by-N */
#define DEVCFG1_DMTINV_MASK     0x00000038 /* Deadman Timer Count Window Interval */
#define DEVCFG1_DMTINV_1_2      0x00000008 /* 1/2 of counter value */
#define DEVCFG1_DMTINV_3_4      0x00000010 /* 3/4 of counter value */
#define DEVCFG1_DMTINV_7_8      0x00000018 /* 7/8 of counter value */
#define DEVCFG1_DMTINV_15_16    0x00000020 /* 15/16 of counter value */
#define DEVCFG1_DMTINV_31_32    0x00000028 /* 31/32 of counter value */
#define DEVCFG1_DMTINV_63_64    0x00000030 /* 63/64 of counter value */
#define DEVCFG1_DMTINV_127_128  0x00000038 /* 127/128 of counter value */
#define DEVCFG1_FSOSCEN         0x00000040 /* Secondary oscillator enable */
#define DEVCFG1_IESO            0x00000080 /* Internal-external switch over enable */
#define DEVCFG1_POSCMOD_MASK    0x00000300 /* Primary oscillator config */
#define DEVCFG1_POSCMOD_EXT     0x00000000 /* External mode */
#define DEVCFG1_POSCMOD_HS      0x00000200 /* HS oscillator */
#define DEVCFG1_POSCMOD_DISABLE 0x00000300 /* Disabled */
#define DEVCFG1_CLKO_DISABLE    0x00000400 /* Disable CLKO output */
#define DEVCFG1_FCKS_ENABLE     0x00004000 /* Enable clock switching */
#define DEVCFG1_FCKM_ENABLE     0x00008000 /* Enable fail-safe clock monitoring */
#define DEVCFG1_WDTPS_MASK      0x001f0000 /* Watchdog postscale */
#define DEVCFG1_WDTPS_1         0x00000000 /* 1:1 */
#define DEVCFG1_WDTPS_2         0x00010000 /* 1:2 */
#define DEVCFG1_WDTPS_4         0x00020000 /* 1:4 */
#define DEVCFG1_WDTPS_8         0x00030000 /* 1:8 */
#define DEVCFG1_WDTPS_16        0x00040000 /* 1:16 */
#define DEVCFG1_WDTPS_32        0x00050000 /* 1:32 */
#define DEVCFG1_WDTPS_64        0x00060000 /* 1:64 */
#define DEVCFG1_WDTPS_128       0x00070000 /* 1:128 */
#define DEVCFG1_WDTPS_256       0x00080000 /* 1:256 */
#define DEVCFG1_WDTPS_512       0x00090000 /* 1:512 */
#define DEVCFG1_WDTPS_1024      0x000a0000 /* 1:1024 */
#define DEVCFG1_WDTPS_2048      0x000b0000 /* 1:2048 */
#define DEVCFG1_WDTPS_4096      0x000c0000 /* 1:4096 */
#define DEVCFG1_WDTPS_8192      0x000d0000 /* 1:8192 */
#define DEVCFG1_WDTPS_16384     0x000e0000 /* 1:16384 */
#define DEVCFG1_WDTPS_32768     0x000f0000 /* 1:32768 */
#define DEVCFG1_WDTPS_65536     0x00100000 /* 1:65536 */
#define DEVCFG1_WDTPS_131072    0x00110000 /* 1:131072 */
#define DEVCFG1_WDTPS_262144    0x00120000 /* 1:262144 */
#define DEVCFG1_WDTPS_524288    0x00130000 /* 1:524288 */
#define DEVCFG1_WDTPS_1048576   0x00140000 /* 1:1048576 */
#define DEVCFG1_WDTSPGM         0x00200000 /* Watchdog stops during Flash programming */
#define DEVCFG1_WINDIS          0x00400000 /* Watchdog is in non-Window mode */
#define DEVCFG1_FWDTEN          0x00800000 /* Watchdog enable */
#define DEVCFG1_FWDTWINSZ_75    0x00000000 /* Watchdog window size is 75% */
#define DEVCFG1_FWDTWINSZ_50    0x01000000 /* 50% */
#define DEVCFG1_FWDTWINSZ_375   0x02000000 /* 37.5% */
#define DEVCFG1_FWDTWINSZ_25    0x03000000 /* 25% */
#define DEVCFG1_DMTCNT(n)       ((n)<<26)  /* Deadman Timer Count Select */
#define DEVCFG1_FDMTEN          0x80000000 /* Deadman Timer enable */

/*
 * Config2 register at 1fc0ffc4.
 */
#define DEVCFG2_UNUSED          0x3ff88008
#define DEVCFG2_FPLLIDIV_MASK   0x00000007 /* PLL input divider */
#define DEVCFG2_FPLLIDIV_1      0x00000000 /* 1x */
#define DEVCFG2_FPLLIDIV_2      0x00000001 /* 2x */
#define DEVCFG2_FPLLIDIV_3      0x00000002 /* 3x */
#define DEVCFG2_FPLLIDIV_4      0x00000003 /* 4x */
#define DEVCFG2_FPLLIDIV_5      0x00000004 /* 5x */
#define DEVCFG2_FPLLIDIV_6      0x00000005 /* 6x */
#define DEVCFG2_FPLLIDIV_7      0x00000006 /* 7x */
#define DEVCFG2_FPLLIDIV_8      0x00000007 /* 8x */
#define DEVCFG2_FPLLRNG_MASK    0x00000070 /* PLL input frequency range */
#define DEVCFG2_FPLLRNG_5_10    0x00000010 /* 5-10 MHz */
#define DEVCFG2_FPLLRNG_8_16    0x00000020 /* 8-16 MHz */
#define DEVCFG2_FPLLRNG_13_26   0x00000030 /* 13-26 MHz */
#define DEVCFG2_FPLLRNG_21_42   0x00000040 /* 21-42 MHz */
#define DEVCFG2_FPLLRNG_34_64   0x00000050 /* 34-64 MHz */
#define DEVCFG2_FPLLICLK_FRC    0x00000080 /* Select FRC as input to PLL */
#define DEVCFG2_FPLLMULT(n)     (((n)-1)<<8) /* PLL Feedback Divider */
#define DEVCFG2_FPLLODIV_MASK   0x00070000 /* Default PLL output divisor */
#define DEVCFG2_FPLLODIV_2      0x00010000 /* 2x */
#define DEVCFG2_FPLLODIV_4      0x00020000 /* 4x */
#define DEVCFG2_FPLLODIV_8      0x00030000 /* 8x */
#define DEVCFG2_FPLLODIV_16     0x00040000 /* 16x */
#define DEVCFG2_FPLLODIV_32     0x00050000 /* 32x */
#define DEVCFG2_UPLLFSEL_24     0x40000000 /* USB PLL input clock is 24 MHz (default 12 MHz) */
#define DEVCFG2_UPLLEN          0x80000000 /* Enable USB PLL */

/*
 * Config3 register at 1fc0ffc0.
 */
#define DEVCFG3_UNUSED          0x84ff0000
#define DEVCFG3_USERID_MASK     0x0000ffff /* User-defined ID */
#define DEVCFG3_USERID(x)       ((x) & 0xffff)
#define DEVCFG3_FMIIEN          0x01000000 /* Ethernet MII enable */
#define DEVCFG3_FETHIO          0x02000000 /* Default Ethernet pins */
#define DEVCFG3_PGL1WAY         0x08000000 /* Permission Group Lock - only 1 reconfig */
#define DEVCFG3_PMDL1WAY        0x10000000 /* Peripheral Module Disable - only 1 reconfig */
#define DEVCFG3_IOL1WAY         0x20000000 /* Peripheral Pin Select - only 1 reconfig */
#define DEVCFG3_FUSBIDIO        0x40000000 /* USBID pin: controlled by USB */

/*--------------------------------------
 * Peripheral registers.
 */
#define PIC32_R(a)		*(volatile unsigned*)(0xBF800000 + (a))

/*--------------------------------------
 * Port A-K registers.
 */
#define ANSELA		PIC32_R (0x60000) /* Port A: analog select */
#define ANSELACLR	PIC32_R (0x60004)
#define ANSELASET	PIC32_R (0x60008)
#define ANSELAINV	PIC32_R (0x6000C)
#define TRISA		PIC32_R (0x60010) /* Port A: mask of inputs */
#define TRISACLR	PIC32_R (0x60014)
#define TRISASET	PIC32_R (0x60018)
#define TRISAINV	PIC32_R (0x6001C)
#define PORTA		PIC32_R (0x60020) /* Port A: read inputs, write outputs */
#define PORTACLR	PIC32_R (0x60024)
#define PORTASET	PIC32_R (0x60028)
#define PORTAINV	PIC32_R (0x6002C)
#define LATA		PIC32_R (0x60030) /* Port A: read/write outputs */
#define LATACLR		PIC32_R (0x60034)
#define LATASET		PIC32_R (0x60038)
#define LATAINV		PIC32_R (0x6003C)
#define ODCA		PIC32_R (0x60040) /* Port A: open drain configuration */
#define ODCACLR		PIC32_R (0x60044)
#define ODCASET		PIC32_R (0x60048)
#define ODCAINV		PIC32_R (0x6004C)
#define CNPUA		PIC32_R (0x60050) /* Port A: input pin pull-up enable */
#define CNPUACLR	PIC32_R (0x60054)
#define CNPUASET	PIC32_R (0x60058)
#define CNPUAINV	PIC32_R (0x6005C)
#define CNPDA		PIC32_R (0x60060) /* Port A: input pin pull-down enable */
#define CNPDACLR	PIC32_R (0x60064)
#define CNPDASET	PIC32_R (0x60068)
#define CNPDAINV	PIC32_R (0x6006C)
#define CNCONA		PIC32_R (0x60070) /* Port A: interrupt-on-change control */
#define CNCONACLR	PIC32_R (0x60074)
#define CNCONASET	PIC32_R (0x60078)
#define CNCONAINV	PIC32_R (0x6007C)
#define CNENA		PIC32_R (0x60080) /* Port A: input change interrupt enable */
#define CNENACLR	PIC32_R (0x60084)
#define CNENASET	PIC32_R (0x60088)
#define CNENAINV	PIC32_R (0x6008C)
#define CNSTATA		PIC32_R (0x60090) /* Port A: status */
#define CNSTATACLR	PIC32_R (0x60094)
#define CNSTATASET	PIC32_R (0x60098)
#define CNSTATAINV	PIC32_R (0x6009C)

#define ANSELB		PIC32_R (0x60100) /* Port B: analog select */
#define ANSELBCLR	PIC32_R (0x60104)
#define ANSELBSET	PIC32_R (0x60108)
#define ANSELBINV	PIC32_R (0x6010C)
#define TRISB		PIC32_R (0x60110) /* Port B: mask of inputs */
#define TRISBCLR	PIC32_R (0x60114)
#define TRISBSET	PIC32_R (0x60118)
#define TRISBINV	PIC32_R (0x6011C)
#define PORTB		PIC32_R (0x60120) /* Port B: read inputs, write outputs */
#define PORTBCLR	PIC32_R (0x60124)
#define PORTBSET	PIC32_R (0x60128)
#define PORTBINV	PIC32_R (0x6012C)
#define LATB		PIC32_R (0x60130) /* Port B: read/write outputs */
#define LATBCLR		PIC32_R (0x60134)
#define LATBSET		PIC32_R (0x60138)
#define LATBINV		PIC32_R (0x6013C)
#define ODCB		PIC32_R (0x60140) /* Port B: open drain configuration */
#define ODCBCLR		PIC32_R (0x60144)
#define ODCBSET		PIC32_R (0x60148)
#define ODCBINV		PIC32_R (0x6014C)
#define CNPUB		PIC32_R (0x60150) /* Port B: input pin pull-up enable */
#define CNPUBCLR	PIC32_R (0x60154)
#define CNPUBSET	PIC32_R (0x60158)
#define CNPUBINV	PIC32_R (0x6015C)
#define CNPDB		PIC32_R (0x60160) /* Port B: input pin pull-down enable */
#define CNPDBCLR	PIC32_R (0x60164)
#define CNPDBSET	PIC32_R (0x60168)
#define CNPDBINV	PIC32_R (0x6016C)
#define CNCONB		PIC32_R (0x60170) /* Port B: interrupt-on-change control */
#define CNCONBCLR	PIC32_R (0x60174)
#define CNCONBSET	PIC32_R (0x60178)
#define CNCONBINV	PIC32_R (0x6017C)
#define CNENB		PIC32_R (0x60180) /* Port B: input change interrupt enable */
#define CNENBCLR	PIC32_R (0x60184)
#define CNENBSET	PIC32_R (0x60188)
#define CNENBINV	PIC32_R (0x6018C)
#define CNSTATB		PIC32_R (0x60190) /* Port B: status */
#define CNSTATBCLR	PIC32_R (0x60194)
#define CNSTATBSET	PIC32_R (0x60198)
#define CNSTATBINV	PIC32_R (0x6019C)

#define ANSELC		PIC32_R (0x60200) /* Port C: analog select */
#define ANSELCCLR	PIC32_R (0x60204)
#define ANSELCSET	PIC32_R (0x60208)
#define ANSELCINV	PIC32_R (0x6020C)
#define TRISC		PIC32_R (0x60210) /* Port C: mask of inputs */
#define TRISCCLR	PIC32_R (0x60214)
#define TRISCSET	PIC32_R (0x60218)
#define TRISCINV	PIC32_R (0x6021C)
#define PORTC		PIC32_R (0x60220) /* Port C: read inputs, write outputs */
#define PORTCCLR	PIC32_R (0x60224)
#define PORTCSET	PIC32_R (0x60228)
#define PORTCINV	PIC32_R (0x6022C)
#define LATC		PIC32_R (0x60230) /* Port C: read/write outputs */
#define LATCCLR		PIC32_R (0x60234)
#define LATCSET		PIC32_R (0x60238)
#define LATCINV		PIC32_R (0x6023C)
#define ODCC		PIC32_R (0x60240) /* Port C: open drain configuration */
#define ODCCCLR		PIC32_R (0x60244)
#define ODCCSET		PIC32_R (0x60248)
#define ODCCINV		PIC32_R (0x6024C)
#define CNPUC		PIC32_R (0x60250) /* Port C: input pin pull-up enable */
#define CNPUCCLR	PIC32_R (0x60254)
#define CNPUCSET	PIC32_R (0x60258)
#define CNPUCINV	PIC32_R (0x6025C)
#define CNPDC		PIC32_R (0x60260) /* Port C: input pin pull-down enable */
#define CNPDCCLR	PIC32_R (0x60264)
#define CNPDCSET	PIC32_R (0x60268)
#define CNPDCINV	PIC32_R (0x6026C)
#define CNCONC		PIC32_R (0x60270) /* Port C: interrupt-on-change control */
#define CNCONCCLR	PIC32_R (0x60274)
#define CNCONCSET	PIC32_R (0x60278)
#define CNCONCINV	PIC32_R (0x6027C)
#define CNENC		PIC32_R (0x60280) /* Port C: input change interrupt enable */
#define CNENCCLR	PIC32_R (0x60284)
#define CNENCSET	PIC32_R (0x60288)
#define CNENCINV	PIC32_R (0x6028C)
#define CNSTATC		PIC32_R (0x60290) /* Port C: status */
#define CNSTATCCLR	PIC32_R (0x60294)
#define CNSTATCSET	PIC32_R (0x60298)
#define CNSTATCINV	PIC32_R (0x6029C)

#define ANSELD		PIC32_R (0x60300) /* Port D: analog select */
#define ANSELDCLR	PIC32_R (0x60304)
#define ANSELDSET	PIC32_R (0x60308)
#define ANSELDINV	PIC32_R (0x6030C)
#define TRISD		PIC32_R (0x60310) /* Port D: mask of inputs */
#define TRISDCLR	PIC32_R (0x60314)
#define TRISDSET	PIC32_R (0x60318)
#define TRISDINV	PIC32_R (0x6031C)
#define PORTD		PIC32_R (0x60320) /* Port D: read inputs, write outputs */
#define PORTDCLR	PIC32_R (0x60324)
#define PORTDSET	PIC32_R (0x60328)
#define PORTDINV	PIC32_R (0x6032C)
#define LATD		PIC32_R (0x60330) /* Port D: read/write outputs */
#define LATDCLR		PIC32_R (0x60334)
#define LATDSET		PIC32_R (0x60338)
#define LATDINV		PIC32_R (0x6033C)
#define ODCD		PIC32_R (0x60340) /* Port D: open drain configuration */
#define ODCDCLR		PIC32_R (0x60344)
#define ODCDSET		PIC32_R (0x60348)
#define ODCDINV		PIC32_R (0x6034C)
#define CNPUD		PIC32_R (0x60350) /* Port D: input pin pull-up enable */
#define CNPUDCLR	PIC32_R (0x60354)
#define CNPUDSET	PIC32_R (0x60358)
#define CNPUDINV	PIC32_R (0x6035C)
#define CNPDD		PIC32_R (0x60360) /* Port D: input pin pull-down enable */
#define CNPDDCLR	PIC32_R (0x60364)
#define CNPDDSET	PIC32_R (0x60368)
#define CNPDDINV	PIC32_R (0x6036C)
#define CNCOND		PIC32_R (0x60370) /* Port D: interrupt-on-change control */
#define CNCONDCLR	PIC32_R (0x60374)
#define CNCONDSET	PIC32_R (0x60378)
#define CNCONDINV	PIC32_R (0x6037C)
#define CNEND		PIC32_R (0x60380) /* Port D: input change interrupt enable */
#define CNENDCLR	PIC32_R (0x60384)
#define CNENDSET	PIC32_R (0x60388)
#define CNENDINV	PIC32_R (0x6038C)
#define CNSTATD		PIC32_R (0x60390) /* Port D: status */
#define CNSTATDCLR	PIC32_R (0x60394)
#define CNSTATDSET	PIC32_R (0x60398)
#define CNSTATDINV	PIC32_R (0x6039C)

#define ANSELE		PIC32_R (0x60400) /* Port E: analog select */
#define ANSELECLR	PIC32_R (0x60404)
#define ANSELESET	PIC32_R (0x60408)
#define ANSELEINV	PIC32_R (0x6040C)
#define TRISE		PIC32_R (0x60410) /* Port E: mask of inputs */
#define TRISECLR	PIC32_R (0x60414)
#define TRISESET	PIC32_R (0x60418)
#define TRISEINV	PIC32_R (0x6041C)
#define PORTE		PIC32_R (0x60420) /* Port E: read inputs, write outputs */
#define PORTECLR	PIC32_R (0x60424)
#define PORTESET	PIC32_R (0x60428)
#define PORTEINV	PIC32_R (0x6042C)
#define LATE		PIC32_R (0x60430) /* Port E: read/write outputs */
#define LATECLR		PIC32_R (0x60434)
#define LATESET		PIC32_R (0x60438)
#define LATEINV		PIC32_R (0x6043C)
#define ODCE		PIC32_R (0x60440) /* Port E: open drain configuration */
#define ODCECLR		PIC32_R (0x60444)
#define ODCESET		PIC32_R (0x60448)
#define ODCEINV		PIC32_R (0x6044C)
#define CNPUE		PIC32_R (0x60450) /* Port E: input pin pull-up enable */
#define CNPUECLR	PIC32_R (0x60454)
#define CNPUESET	PIC32_R (0x60458)
#define CNPUEINV	PIC32_R (0x6045C)
#define CNPDE		PIC32_R (0x60460) /* Port E: input pin pull-down enable */
#define CNPDECLR	PIC32_R (0x60464)
#define CNPDESET	PIC32_R (0x60468)
#define CNPDEINV	PIC32_R (0x6046C)
#define CNCONE		PIC32_R (0x60470) /* Port E: interrupt-on-change control */
#define CNCONECLR	PIC32_R (0x60474)
#define CNCONESET	PIC32_R (0x60478)
#define CNCONEINV	PIC32_R (0x6047C)
#define CNENE		PIC32_R (0x60480) /* Port E: input change interrupt enable */
#define CNENECLR	PIC32_R (0x60484)
#define CNENESET	PIC32_R (0x60488)
#define CNENEINV	PIC32_R (0x6048C)
#define CNSTATE		PIC32_R (0x60490) /* Port E: status */
#define CNSTATECLR	PIC32_R (0x60494)
#define CNSTATESET	PIC32_R (0x60498)
#define CNSTATEINV	PIC32_R (0x6049C)

#define ANSELF		PIC32_R (0x60500) /* Port F: analog select */
#define ANSELFCLR	PIC32_R (0x60504)
#define ANSELFSET	PIC32_R (0x60508)
#define ANSELFINV	PIC32_R (0x6050C)
#define TRISF		PIC32_R (0x60510) /* Port F: mask of inputs */
#define TRISFCLR	PIC32_R (0x60514)
#define TRISFSET	PIC32_R (0x60518)
#define TRISFINV	PIC32_R (0x6051C)
#define PORTF		PIC32_R (0x60520) /* Port F: read inputs, write outputs */
#define PORTFCLR	PIC32_R (0x60524)
#define PORTFSET	PIC32_R (0x60528)
#define PORTFINV	PIC32_R (0x6052C)
#define LATF		PIC32_R (0x60530) /* Port F: read/write outputs */
#define LATFCLR		PIC32_R (0x60534)
#define LATFSET		PIC32_R (0x60538)
#define LATFINV		PIC32_R (0x6053C)
#define ODCF		PIC32_R (0x60540) /* Port F: open drain configuration */
#define ODCFCLR		PIC32_R (0x60544)
#define ODCFSET		PIC32_R (0x60548)
#define ODCFINV		PIC32_R (0x6054C)
#define CNPUF		PIC32_R (0x60550) /* Port F: input pin pull-up enable */
#define CNPUFCLR	PIC32_R (0x60554)
#define CNPUFSET	PIC32_R (0x60558)
#define CNPUFINV	PIC32_R (0x6055C)
#define CNPDF		PIC32_R (0x60560) /* Port F: input pin pull-down enable */
#define CNPDFCLR	PIC32_R (0x60564)
#define CNPDFSET	PIC32_R (0x60568)
#define CNPDFINV	PIC32_R (0x6056C)
#define CNCONF		PIC32_R (0x60570) /* Port F: interrupt-on-change control */
#define CNCONFCLR	PIC32_R (0x60574)
#define CNCONFSET	PIC32_R (0x60578)
#define CNCONFINV	PIC32_R (0x6057C)
#define CNENF		PIC32_R (0x60580) /* Port F: input change interrupt enable */
#define CNENFCLR	PIC32_R (0x60584)
#define CNENFSET	PIC32_R (0x60588)
#define CNENFINV	PIC32_R (0x6058C)
#define CNSTATF		PIC32_R (0x60590) /* Port F: status */
#define CNSTATFCLR	PIC32_R (0x60594)
#define CNSTATFSET	PIC32_R (0x60598)
#define CNSTATFINV	PIC32_R (0x6059C)

#define ANSELG		PIC32_R (0x60600) /* Port G: analog select */
#define ANSELGCLR	PIC32_R (0x60604)
#define ANSELGSET	PIC32_R (0x60608)
#define ANSELGINV	PIC32_R (0x6060C)
#define TRISG		PIC32_R (0x60610) /* Port G: mask of inputs */
#define TRISGCLR	PIC32_R (0x60614)
#define TRISGSET	PIC32_R (0x60618)
#define TRISGINV	PIC32_R (0x6061C)
#define PORTG		PIC32_R (0x60620) /* Port G: read inputs, write outputs */
#define PORTGCLR	PIC32_R (0x60624)
#define PORTGSET	PIC32_R (0x60628)
#define PORTGINV	PIC32_R (0x6062C)
#define LATG		PIC32_R (0x60630) /* Port G: read/write outputs */
#define LATGCLR		PIC32_R (0x60634)
#define LATGSET		PIC32_R (0x60638)
#define LATGINV		PIC32_R (0x6063C)
#define ODCG		PIC32_R (0x60640) /* Port G: open drain configuration */
#define ODCGCLR		PIC32_R (0x60644)
#define ODCGSET		PIC32_R (0x60648)
#define ODCGINV		PIC32_R (0x6064C)
#define CNPUG		PIC32_R (0x60650) /* Port G: input pin pull-up enable */
#define CNPUGCLR	PIC32_R (0x60654)
#define CNPUGSET	PIC32_R (0x60658)
#define CNPUGINV	PIC32_R (0x6065C)
#define CNPDG		PIC32_R (0x60660) /* Port G: input pin pull-down enable */
#define CNPDGCLR	PIC32_R (0x60664)
#define CNPDGSET	PIC32_R (0x60668)
#define CNPDGINV	PIC32_R (0x6066C)
#define CNCONG		PIC32_R (0x60670) /* Port G: interrupt-on-change control */
#define CNCONGCLR	PIC32_R (0x60674)
#define CNCONGSET	PIC32_R (0x60678)
#define CNCONGINV	PIC32_R (0x6067C)
#define CNENG		PIC32_R (0x60680) /* Port G: input change interrupt enable */
#define CNENGCLR	PIC32_R (0x60684)
#define CNENGSET	PIC32_R (0x60688)
#define CNENGINV	PIC32_R (0x6068C)
#define CNSTATG		PIC32_R (0x60690) /* Port G: status */
#define CNSTATGCLR	PIC32_R (0x60694)
#define CNSTATGSET	PIC32_R (0x60698)
#define CNSTATGINV	PIC32_R (0x6069C)

#define ANSELH		PIC32_R (0x60700) /* Port H: analog select */
#define ANSELHCLR	PIC32_R (0x60704)
#define ANSELHSET	PIC32_R (0x60708)
#define ANSELHINV	PIC32_R (0x6070C)
#define TRISH		PIC32_R (0x60710) /* Port H: mask of inputs */
#define TRISHCLR	PIC32_R (0x60714)
#define TRISHSET	PIC32_R (0x60718)
#define TRISHINV	PIC32_R (0x6071C)
#define PORTH		PIC32_R (0x60720) /* Port H: read inputs, write outputs */
#define PORTHCLR	PIC32_R (0x60724)
#define PORTHSET	PIC32_R (0x60728)
#define PORTHINV	PIC32_R (0x6072C)
#define LATH		PIC32_R (0x60730) /* Port H: read/write outputs */
#define LATHCLR		PIC32_R (0x60734)
#define LATHSET		PIC32_R (0x60738)
#define LATHINV		PIC32_R (0x6073C)
#define ODCH		PIC32_R (0x60740) /* Port H: open drain configuration */
#define ODCHCLR		PIC32_R (0x60744)
#define ODCHSET		PIC32_R (0x60748)
#define ODCHINV		PIC32_R (0x6074C)
#define CNPUH		PIC32_R (0x60750) /* Port H: input pin pull-up enable */
#define CNPUHCLR	PIC32_R (0x60754)
#define CNPUHSET	PIC32_R (0x60758)
#define CNPUHINV	PIC32_R (0x6075C)
#define CNPDH		PIC32_R (0x60760) /* Port H: input pin pull-down enable */
#define CNPDHCLR	PIC32_R (0x60764)
#define CNPDHSET	PIC32_R (0x60768)
#define CNPDHINV	PIC32_R (0x6076C)
#define CNCONH		PIC32_R (0x60770) /* Port H: interrupt-on-change control */
#define CNCONHCLR	PIC32_R (0x60774)
#define CNCONHSET	PIC32_R (0x60778)
#define CNCONHINV	PIC32_R (0x6077C)
#define CNENH		PIC32_R (0x60780) /* Port H: input change interrupt enable */
#define CNENHCLR	PIC32_R (0x60784)
#define CNENHSET	PIC32_R (0x60788)
#define CNENHINV	PIC32_R (0x6078C)
#define CNSTATH		PIC32_R (0x60790) /* Port H: status */
#define CNSTATHCLR	PIC32_R (0x60794)
#define CNSTATHSET	PIC32_R (0x60798)
#define CNSTATHINV	PIC32_R (0x6079C)

#define ANSELJ		PIC32_R (0x60800) /* Port J: analog select */
#define ANSELJCLR	PIC32_R (0x60804)
#define ANSELJSET	PIC32_R (0x60808)
#define ANSELJINV	PIC32_R (0x6080C)
#define TRISJ		PIC32_R (0x60810) /* Port J: mask of inputs */
#define TRISJCLR	PIC32_R (0x60814)
#define TRISJSET	PIC32_R (0x60818)
#define TRISJINV	PIC32_R (0x6081C)
#define PORTJ		PIC32_R (0x60820) /* Port J: read inputs, write outputs */
#define PORTJCLR	PIC32_R (0x60824)
#define PORTJSET	PIC32_R (0x60828)
#define PORTJINV	PIC32_R (0x6082C)
#define LATJ		PIC32_R (0x60830) /* Port J: read/write outputs */
#define LATJCLR		PIC32_R (0x60834)
#define LATJSET		PIC32_R (0x60838)
#define LATJINV		PIC32_R (0x6083C)
#define ODCJ		PIC32_R (0x60840) /* Port J: open drain configuration */
#define ODCJCLR		PIC32_R (0x60844)
#define ODCJSET		PIC32_R (0x60848)
#define ODCJINV		PIC32_R (0x6084C)
#define CNPUJ		PIC32_R (0x60850) /* Port J: input pin pull-up enable */
#define CNPUJCLR	PIC32_R (0x60854)
#define CNPUJSET	PIC32_R (0x60858)
#define CNPUJINV	PIC32_R (0x6085C)
#define CNPDJ		PIC32_R (0x60860) /* Port J: input pin pull-down enable */
#define CNPDJCLR	PIC32_R (0x60864)
#define CNPDJSET	PIC32_R (0x60868)
#define CNPDJINV	PIC32_R (0x6086C)
#define CNCONJ		PIC32_R (0x60870) /* Port J: interrupt-on-change control */
#define CNCONJCLR	PIC32_R (0x60874)
#define CNCONJSET	PIC32_R (0x60878)
#define CNCONJINV	PIC32_R (0x6087C)
#define CNENJ		PIC32_R (0x60880) /* Port J: input change interrupt enable */
#define CNENJCLR	PIC32_R (0x60884)
#define CNENJSET	PIC32_R (0x60888)
#define CNENJINV	PIC32_R (0x6088C)
#define CNSTATJ		PIC32_R (0x60890) /* Port J: status */
#define CNSTATJCLR	PIC32_R (0x60894)
#define CNSTATJSET	PIC32_R (0x60898)
#define CNSTATJINV	PIC32_R (0x6089C)

#define TRISK		PIC32_R (0x60910) /* Port K: mask of inputs */
#define TRISKCLR	PIC32_R (0x60914)
#define TRISKSET	PIC32_R (0x60918)
#define TRISKINV	PIC32_R (0x6091C)
#define PORTK		PIC32_R (0x60920) /* Port K: read inputs, write outputs */
#define PORTKCLR	PIC32_R (0x60924)
#define PORTKSET	PIC32_R (0x60928)
#define PORTKINV	PIC32_R (0x6092C)
#define LATK		PIC32_R (0x60930) /* Port K: read/write outputs */
#define LATKCLR		PIC32_R (0x60934)
#define LATKSET		PIC32_R (0x60938)
#define LATKINV		PIC32_R (0x6093C)
#define ODCK		PIC32_R (0x60940) /* Port K: open drain configuration */
#define ODCKCLR		PIC32_R (0x60944)
#define ODCKSET		PIC32_R (0x60948)
#define ODCKINV		PIC32_R (0x6094C)
#define CNPUK		PIC32_R (0x60950) /* Port K: input pin pull-up enable */
#define CNPUKCLR	PIC32_R (0x60954)
#define CNPUKSET	PIC32_R (0x60958)
#define CNPUKINV	PIC32_R (0x6095C)
#define CNPDK		PIC32_R (0x60960) /* Port K: input pin pull-down enable */
#define CNPDKCLR	PIC32_R (0x60964)
#define CNPDKSET	PIC32_R (0x60968)
#define CNPDKINV	PIC32_R (0x6096C)
#define CNCONK		PIC32_R (0x60970) /* Port K: interrupt-on-change control */
#define CNCONKCLR	PIC32_R (0x60974)
#define CNCONKSET	PIC32_R (0x60978)
#define CNCONKINV	PIC32_R (0x6097C)
#define CNENK		PIC32_R (0x60980) /* Port K: input change interrupt enable */
#define CNENKCLR	PIC32_R (0x60984)
#define CNENKSET	PIC32_R (0x60988)
#define CNENKINV	PIC32_R (0x6098C)
#define CNSTATK		PIC32_R (0x60990) /* Port K: status */
#define CNSTATKCLR	PIC32_R (0x60994)
#define CNSTATKSET	PIC32_R (0x60998)
#define CNSTATKINV	PIC32_R (0x6099C)

/*--------------------------------------
 * Parallel master port registers.
 */
#define PMCON		PIC32_R (0x2E000) /* Control */
#define PMCONCLR	PIC32_R (0x2E004)
#define PMCONSET	PIC32_R (0x2E008)
#define PMCONINV	PIC32_R (0x2E00C)
#define PMMODE		PIC32_R (0x2E010) /* Mode */
#define PMMODECLR	PIC32_R (0x2E014)
#define PMMODESET	PIC32_R (0x2E018)
#define PMMODEINV	PIC32_R (0x2E01C)
#define PMADDR		PIC32_R (0x2E020) /* Address */
#define PMADDRCLR	PIC32_R (0x2E024)
#define PMADDRSET	PIC32_R (0x2E028)
#define PMADDRINV	PIC32_R (0x2E02C)
#define PMDOUT		PIC32_R (0x2E030) /* Data output */
#define PMDOUTCLR	PIC32_R (0x2E034)
#define PMDOUTSET	PIC32_R (0x2E038)
#define PMDOUTINV	PIC32_R (0x2E03C)
#define PMDIN		PIC32_R (0x2E040) /* Data input */
#define PMDINCLR	PIC32_R (0x2E044)
#define PMDINSET	PIC32_R (0x2E048)
#define PMDININV	PIC32_R (0x2E04C)
#define PMAEN		PIC32_R (0x2E050) /* Pin enable */
#define PMAENCLR	PIC32_R (0x2E054)
#define PMAENSET	PIC32_R (0x2E058)
#define PMAENINV	PIC32_R (0x2E05C)
#define PMSTAT		PIC32_R (0x2E060) /* Status (slave only) */
#define PMSTATCLR	PIC32_R (0x2E064)
#define PMSTATSET	PIC32_R (0x2E068)
#define PMSTATINV	PIC32_R (0x2E06C)

/*
 * PMP Control register.
 */
#define PIC32_PMCON_RDSP	0x0001 /* Read strobe polarity active-high */
#define PIC32_PMCON_WRSP	0x0002 /* Write strobe polarity active-high */
#define PIC32_PMCON_CS1P	0x0008 /* Chip select 0 polarity active-high */
#define PIC32_PMCON_CS2P	0x0010 /* Chip select 1 polarity active-high */
#define PIC32_PMCON_ALP		0x0020 /* Address latch polarity active-high */
#define PIC32_PMCON_CSF		0x00C0 /* Chip select function bitmask: */
#define PIC32_PMCON_CSF_NONE	0x0000 /* PMCS2 and PMCS1 as A[15:14] */
#define PIC32_PMCON_CSF_CS2	0x0040 /* PMCS2 as chip select */
#define PIC32_PMCON_CSF_CS21	0x0080 /* PMCS2 and PMCS1 as chip select */
#define PIC32_PMCON_PTRDEN	0x0100 /* Read/write strobe port enable */
#define PIC32_PMCON_PTWREN	0x0200 /* Write enable strobe port enable */
#define PIC32_PMCON_PMPTTL	0x0400 /* TTL input buffer select */
#define PIC32_PMCON_ADRMUX	0x1800 /* Address/data mux selection bitmask: */
#define PIC32_PMCON_ADRMUX_NONE	0x0000 /* Address and data separate */
#define PIC32_PMCON_ADRMUX_AD	0x0800 /* Lower address on PMD[7:0], high on PMA[15:8] */
#define PIC32_PMCON_ADRMUX_D8	0x1000 /* All address on PMD[7:0] */
#define PIC32_PMCON_ADRMUX_D16	0x1800 /* All address on PMD[15:0] */
#define PIC32_PMCON_SIDL	0x2000 /* Stop in idle */
#define PIC32_PMCON_FRZ		0x4000 /* Freeze in debug exception */
#define PIC32_PMCON_ON		0x8000 /* Parallel master port enable */

/*
 * PMP Mode register.
 */
#define PIC32_PMMODE_WAITE(x)	((x)<<0) /* Wait states: data hold after RW strobe */
#define PIC32_PMMODE_WAITM(x)	((x)<<2) /* Wait states: data RW strobe */
#define PIC32_PMMODE_WAITB(x)	((x)<<6) /* Wait states: data setup to RW strobe */
#define PIC32_PMMODE_MODE	0x0300	/* Mode select bitmask: */
#define PIC32_PMMODE_MODE_SLAVE	0x0000	/* Legacy slave */
#define PIC32_PMMODE_MODE_SLENH	0x0100	/* Enhanced slave */
#define PIC32_PMMODE_MODE_MAST2	0x0200	/* Master mode 2 */
#define PIC32_PMMODE_MODE_MAST1	0x0300	/* Master mode 1 */
#define PIC32_PMMODE_MODE16	0x0400	/* 16-bit mode */
#define PIC32_PMMODE_INCM	0x1800	/* Address increment mode bitmask: */
#define PIC32_PMMODE_INCM_NONE	0x0000	/* No increment/decrement */
#define PIC32_PMMODE_INCM_INC	0x0800	/* Increment address */
#define PIC32_PMMODE_INCM_DEC	0x1000	/* Decrement address */
#define PIC32_PMMODE_INCM_SLAVE	0x1800	/* Slave auto-increment */
#define PIC32_PMMODE_IRQM	0x6000	/* Interrupt request bitmask: */
#define PIC32_PMMODE_IRQM_DIS	0x0000	/* No interrupt generated */
#define PIC32_PMMODE_IRQM_END	0x2000	/* Interrupt at end of read/write cycle */
#define PIC32_PMMODE_IRQM_A3	0x4000	/* Interrupt on address 3 */
#define PIC32_PMMODE_BUSY	0x8000	/* Port is busy */

/*
 * PMP Address register.
 */
#define PIC32_PMADDR_PADDR	0x3FFF /* Destination address */
#define PIC32_PMADDR_CS1	0x4000 /* Chip select 1 is active */
#define PIC32_PMADDR_CS2	0x8000 /* Chip select 2 is active */

/*
 * PMP status register (slave only).
 */
#define PIC32_PMSTAT_OB0E	0x0001 /* Output buffer 0 empty */
#define PIC32_PMSTAT_OB1E	0x0002 /* Output buffer 1 empty */
#define PIC32_PMSTAT_OB2E	0x0004 /* Output buffer 2 empty */
#define PIC32_PMSTAT_OB3E	0x0008 /* Output buffer 3 empty */
#define PIC32_PMSTAT_OBUF	0x0040 /* Output buffer underflow */
#define PIC32_PMSTAT_OBE	0x0080 /* Output buffer empty */
#define PIC32_PMSTAT_IB0F	0x0100 /* Input buffer 0 full */
#define PIC32_PMSTAT_IB1F	0x0200 /* Input buffer 1 full */
#define PIC32_PMSTAT_IB2F	0x0400 /* Input buffer 2 full */
#define PIC32_PMSTAT_IB3F	0x0800 /* Input buffer 3 full */
#define PIC32_PMSTAT_IBOV	0x4000 /* Input buffer overflow */
#define PIC32_PMSTAT_IBF	0x8000 /* Input buffer full */

/*--------------------------------------
 * UART registers.
 */
#define U1MODE		PIC32_R (0x22000) /* Mode */
#define U1MODECLR	PIC32_R (0x22004)
#define U1MODESET	PIC32_R (0x22008)
#define U1MODEINV	PIC32_R (0x2200C)
#define U1STA		PIC32_R (0x22010) /* Status and control */
#define U1STACLR	PIC32_R (0x22014)
#define U1STASET	PIC32_R (0x22018)
#define U1STAINV	PIC32_R (0x2201C)
#define U1TXREG		PIC32_R (0x22020) /* Transmit */
#define U1RXREG		PIC32_R (0x22030) /* Receive */
#define U1BRG		PIC32_R (0x22040) /* Baud rate */
#define U1BRGCLR	PIC32_R (0x22044)
#define U1BRGSET	PIC32_R (0x22048)
#define U1BRGINV	PIC32_R (0x2204C)

#define U2MODE		PIC32_R (0x22200) /* Mode */
#define U2MODECLR	PIC32_R (0x22204)
#define U2MODESET	PIC32_R (0x22208)
#define U2MODEINV	PIC32_R (0x2220C)
#define U2STA		PIC32_R (0x22210) /* Status and control */
#define U2STACLR	PIC32_R (0x22214)
#define U2STASET	PIC32_R (0x22218)
#define U2STAINV	PIC32_R (0x2221C)
#define U2TXREG		PIC32_R (0x22220) /* Transmit */
#define U2RXREG		PIC32_R (0x22230) /* Receive */
#define U2BRG		PIC32_R (0x22240) /* Baud rate */
#define U2BRGCLR	PIC32_R (0x22244)
#define U2BRGSET	PIC32_R (0x22248)
#define U2BRGINV	PIC32_R (0x2224C)

#define U3MODE		PIC32_R (0x22400) /* Mode */
#define U3MODECLR	PIC32_R (0x22404)
#define U3MODESET	PIC32_R (0x22408)
#define U3MODEINV	PIC32_R (0x2240C)
#define U3STA		PIC32_R (0x22410) /* Status and control */
#define U3STACLR	PIC32_R (0x22414)
#define U3STASET	PIC32_R (0x22418)
#define U3STAINV	PIC32_R (0x2241C)
#define U3TXREG		PIC32_R (0x22420) /* Transmit */
#define U3RXREG		PIC32_R (0x22430) /* Receive */
#define U3BRG		PIC32_R (0x22440) /* Baud rate */
#define U3BRGCLR	PIC32_R (0x22444)
#define U3BRGSET	PIC32_R (0x22448)
#define U3BRGINV	PIC32_R (0x2244C)

#define U4MODE		PIC32_R (0x22600) /* Mode */
#define U4MODECLR	PIC32_R (0x22604)
#define U4MODESET	PIC32_R (0x22608)
#define U4MODEINV	PIC32_R (0x2260C)
#define U4STA		PIC32_R (0x22610) /* Status and control */
#define U4STACLR	PIC32_R (0x22614)
#define U4STASET	PIC32_R (0x22618)
#define U4STAINV	PIC32_R (0x2261C)
#define U4TXREG		PIC32_R (0x22620) /* Transmit */
#define U4RXREG		PIC32_R (0x22630) /* Receive */
#define U4BRG		PIC32_R (0x22640) /* Baud rate */
#define U4BRGCLR	PIC32_R (0x22644)
#define U4BRGSET	PIC32_R (0x22648)
#define U4BRGINV	PIC32_R (0x2264C)

#define U5MODE		PIC32_R (0x22800) /* Mode */
#define U5MODECLR	PIC32_R (0x22804)
#define U5MODESET	PIC32_R (0x22808)
#define U5MODEINV	PIC32_R (0x2280C)
#define U5STA		PIC32_R (0x22810) /* Status and control */
#define U5STACLR	PIC32_R (0x22814)
#define U5STASET	PIC32_R (0x22818)
#define U5STAINV	PIC32_R (0x2281C)
#define U5TXREG		PIC32_R (0x22820) /* Transmit */
#define U5RXREG		PIC32_R (0x22830) /* Receive */
#define U5BRG		PIC32_R (0x22840) /* Baud rate */
#define U5BRGCLR	PIC32_R (0x22844)
#define U5BRGSET	PIC32_R (0x22848)
#define U5BRGINV	PIC32_R (0x2284C)

#define U6MODE		PIC32_R (0x22A00) /* Mode */
#define U6MODECLR	PIC32_R (0x22A04)
#define U6MODESET	PIC32_R (0x22A08)
#define U6MODEINV	PIC32_R (0x22A0C)
#define U6STA		PIC32_R (0x22A10) /* Status and control */
#define U6STACLR	PIC32_R (0x22A14)
#define U6STASET	PIC32_R (0x22A18)
#define U6STAINV	PIC32_R (0x22A1C)
#define U6TXREG		PIC32_R (0x22A20) /* Transmit */
#define U6RXREG		PIC32_R (0x22A30) /* Receive */
#define U6BRG		PIC32_R (0x22A40) /* Baud rate */
#define U6BRGCLR	PIC32_R (0x22A44)
#define U6BRGSET	PIC32_R (0x22A48)
#define U6BRGINV	PIC32_R (0x22A4C)

/*
 * UART Mode register.
 */
#define PIC32_UMODE_STSEL	0x0001	/* 2 Stop bits */
#define PIC32_UMODE_PDSEL	0x0006	/* Bitmask: */
#define PIC32_UMODE_PDSEL_8NPAR	0x0000	/* 8-bit data, no parity */
#define PIC32_UMODE_PDSEL_8EVEN	0x0002	/* 8-bit data, even parity */
#define PIC32_UMODE_PDSEL_8ODD	0x0004	/* 8-bit data, odd parity */
#define PIC32_UMODE_PDSEL_9NPAR	0x0006	/* 9-bit data, no parity */
#define PIC32_UMODE_BRGH	0x0008	/* High Baud Rate Enable */
#define PIC32_UMODE_RXINV	0x0010	/* Receive Polarity Inversion */
#define PIC32_UMODE_ABAUD	0x0020	/* Auto-Baud Enable */
#define PIC32_UMODE_LPBACK	0x0040	/* UARTx Loopback Mode */
#define PIC32_UMODE_WAKE	0x0080	/* Wake-up on start bit during Sleep Mode */
#define PIC32_UMODE_UEN		0x0300	/* Bitmask: */
#define PIC32_UMODE_UEN_RTS	0x0100	/* Using UxRTS pin */
#define PIC32_UMODE_UEN_RTSCTS	0x0200	/* Using UxCTS and UxRTS pins */
#define PIC32_UMODE_UEN_BCLK	0x0300	/* Using UxBCLK pin */
#define PIC32_UMODE_RTSMD	0x0800	/* UxRTS Pin Simplex mode */
#define PIC32_UMODE_IREN	0x1000	/* IrDA Encoder and Decoder Enable bit */
#define PIC32_UMODE_SIDL	0x2000	/* Stop in Idle Mode */
#define PIC32_UMODE_FRZ		0x4000	/* Freeze in Debug Exception Mode */
#define PIC32_UMODE_ON		0x8000	/* UART Enable */

/*
 * UART Control and status register.
 */
#define PIC32_USTA_URXDA	0x00000001 /* Receive Data Available (read-only) */
#define PIC32_USTA_OERR		0x00000002 /* Receive Buffer Overrun */
#define PIC32_USTA_FERR		0x00000004 /* Framing error detected (read-only) */
#define PIC32_USTA_PERR		0x00000008 /* Parity error detected (read-only) */
#define PIC32_USTA_RIDLE	0x00000010 /* Receiver is idle (read-only) */
#define PIC32_USTA_ADDEN	0x00000020 /* Address Detect mode */
#define PIC32_USTA_URXISEL	0x000000C0 /* Bitmask: receive interrupt is set when... */
#define PIC32_USTA_URXISEL_NEMP	0x00000000 /* ...receive buffer is not empty */
#define PIC32_USTA_URXISEL_HALF	0x00000040 /* ...receive buffer becomes 1/2 full */
#define PIC32_USTA_URXISEL_3_4	0x00000080 /* ...receive buffer becomes 3/4 full */
#define PIC32_USTA_TRMT		0x00000100 /* Transmit shift register is empty (read-only) */
#define PIC32_USTA_UTXBF	0x00000200 /* Transmit buffer is full (read-only) */
#define PIC32_USTA_UTXEN	0x00000400 /* Transmit Enable */
#define PIC32_USTA_UTXBRK	0x00000800 /* Transmit Break */
#define PIC32_USTA_URXEN	0x00001000 /* Receiver Enable */
#define PIC32_USTA_UTXINV	0x00002000 /* Transmit Polarity Inversion */
#define PIC32_USTA_UTXISEL	0x0000C000 /* Bitmask: TX interrupt is generated when... */
#define PIC32_USTA_UTXISEL_1	0x00000000 /* ...the transmit buffer contains at least one empty space */
#define PIC32_USTA_UTXISEL_ALL	0x00004000 /* ...all characters have been transmitted */
#define PIC32_USTA_UTXISEL_EMP	0x00008000 /* ...the transmit buffer becomes empty */
#define PIC32_USTA_ADDR		0x00FF0000 /* Automatic Address Mask */
#define PIC32_USTA_ADM_EN	0x01000000 /* Automatic Address Detect */

/*
 * Compute the 16-bit baud rate divisor, given
 * the bus frequency and baud rate.
 * Round to the nearest integer.
 */
#define PIC32_BRG_BAUD(fr,bd)	((((fr)/8 + (bd)) / (bd) / 2) - 1)

/*--------------------------------------
 * Peripheral port select registers.
 */
#define INT1R           PIC32_R (0x1404)
#define INT2R           PIC32_R (0x1408)
#define INT3R           PIC32_R (0x140C)
#define INT4R           PIC32_R (0x1410)
#define T2CKR           PIC32_R (0x1418)
#define T3CKR           PIC32_R (0x141C)
#define T4CKR           PIC32_R (0x1420)
#define T5CKR           PIC32_R (0x1424)
#define T6CKR           PIC32_R (0x1428)
#define T7CKR           PIC32_R (0x142C)
#define T8CKR           PIC32_R (0x1430)
#define T9CKR           PIC32_R (0x1434)
#define IC1R            PIC32_R (0x1438)
#define IC2R            PIC32_R (0x143C)
#define IC3R            PIC32_R (0x1440)
#define IC4R            PIC32_R (0x1444)
#define IC5R            PIC32_R (0x1448)
#define IC6R            PIC32_R (0x144C)
#define IC7R            PIC32_R (0x1450)
#define IC8R            PIC32_R (0x1454)
#define IC9R            PIC32_R (0x1458)
#define OCFAR           PIC32_R (0x1460)
#define U1RXR           PIC32_R (0x1468)
#define U1CTSR          PIC32_R (0x146C)
#define U2RXR           PIC32_R (0x1470)
#define U2CTSR          PIC32_R (0x1474)
#define U3RXR           PIC32_R (0x1478)
#define U3CTSR          PIC32_R (0x147C)
#define U4RXR           PIC32_R (0x1480)
#define U4CTSR          PIC32_R (0x1484)
#define U5RXR           PIC32_R (0x1488)
#define U5CTSR          PIC32_R (0x148C)
#define U6RXR           PIC32_R (0x1490)
#define U6CTSR          PIC32_R (0x1494)
#define SDI1R           PIC32_R (0x149C)
#define SS1R            PIC32_R (0x14A0)
#define SDI2R           PIC32_R (0x14A8)
#define SS2R            PIC32_R (0x14AC)
#define SDI3R           PIC32_R (0x14B4)
#define SS3R            PIC32_R (0x14B8)
#define SDI4R           PIC32_R (0x14C0)
#define SS4R            PIC32_R (0x14C4)
#define SDI5R           PIC32_R (0x14CC)
#define SS5R            PIC32_R (0x14D0)
#define SDI6R           PIC32_R (0x14D8)
#define SS6R            PIC32_R (0x14DC)
#define C1RXR           PIC32_R (0x14E0)
#define C2RXR           PIC32_R (0x14E4)
#define REFCLK1R        PIC32_R (0x14E8)
#define REFCLK3R        PIC32_R (0x14F0)
#define REFCLK4R        PIC32_R (0x14F4)

#define RPA14R          PIC32_R (0x1538)
#define RPA15R          PIC32_R (0x153C)
#define RPB0R           PIC32_R (0x1540)
#define RPB1R           PIC32_R (0x1544)
#define RPB2R           PIC32_R (0x1548)
#define RPB3R           PIC32_R (0x154C)
#define RPB5R           PIC32_R (0x1554)
#define RPB6R           PIC32_R (0x1558)
#define RPB7R           PIC32_R (0x155C)
#define RPB8R           PIC32_R (0x1560)
#define RPB9R           PIC32_R (0x1564)
#define RPB10R          PIC32_R (0x1568)
#define RPB14R          PIC32_R (0x1578)
#define RPB15R          PIC32_R (0x157C)
#define RPC1R           PIC32_R (0x1584)
#define RPC2R           PIC32_R (0x1588)
#define RPC3R           PIC32_R (0x158C)
#define RPC4R           PIC32_R (0x1590)
#define RPC13R          PIC32_R (0x15B4)
#define RPC14R          PIC32_R (0x15B8)
#define RPD0R           PIC32_R (0x15C0)
#define RPD1R           PIC32_R (0x15C4)
#define RPD2R           PIC32_R (0x15C8)
#define RPD3R           PIC32_R (0x15CC)
#define RPD4R           PIC32_R (0x15D0)
#define RPD5R           PIC32_R (0x15D4)
#define RPD6R           PIC32_R (0x15D8)
#define RPD7R           PIC32_R (0x15DC)
#define RPD9R           PIC32_R (0x15E4)
#define RPD10R          PIC32_R (0x15E8)
#define RPD11R          PIC32_R (0x15EC)
#define RPD12R          PIC32_R (0x15F0)
#define RPD14R          PIC32_R (0x15F8)
#define RPD15R          PIC32_R (0x15FC)
#define RPE3R           PIC32_R (0x160C)
#define RPE5R           PIC32_R (0x1614)
#define RPE8R           PIC32_R (0x1620)
#define RPE9R           PIC32_R (0x1624)
#define RPF0R           PIC32_R (0x1640)
#define RPF1R           PIC32_R (0x1644)
#define RPF2R           PIC32_R (0x1648)
#define RPF3R           PIC32_R (0x164C)
#define RPF4R           PIC32_R (0x1650)
#define RPF5R           PIC32_R (0x1654)
#define RPF8R           PIC32_R (0x1660)
#define RPF12R          PIC32_R (0x1670)
#define RPF13R          PIC32_R (0x1674)
#define RPG0R           PIC32_R (0x1680)
#define RPG1R           PIC32_R (0x1684)
#define RPG6R           PIC32_R (0x1698)
#define RPG7R           PIC32_R (0x169C)
#define RPG8R           PIC32_R (0x16A0)
#define RPG9R           PIC32_R (0x16A4)

#if 0 // TODO
/*
 * Compute the 16-bit baud rate divisor, given
 * the bus frequency and baud rate.
 * Round to the nearest integer.
 */
#define PIC32_BRG_BAUD(fr,bd)	((((fr)/8 + (bd)) / (bd) / 2) - 1)

/*--------------------------------------
 * A/D Converter registers.
 */
#define AD1CON1		PIC32_R (0x9000) /* Control register 1 */
#define AD1CON1CLR	PIC32_R (0x9004)
#define AD1CON1SET	PIC32_R (0x9008)
#define AD1CON1INV	PIC32_R (0x900C)
#define AD1CON2		PIC32_R (0x9010) /* Control register 2 */
#define AD1CON2CLR	PIC32_R (0x9014)
#define AD1CON2SET	PIC32_R (0x9018)
#define AD1CON2INV	PIC32_R (0x901C)
#define AD1CON3		PIC32_R (0x9020) /* Control register 3 */
#define AD1CON3CLR	PIC32_R (0x9024)
#define AD1CON3SET	PIC32_R (0x9028)
#define AD1CON3INV	PIC32_R (0x902C)
#define AD1CHS		PIC32_R (0x9040) /* Channel select */
#define AD1CHSCLR	PIC32_R (0x9044)
#define AD1CHSSET	PIC32_R (0x9048)
#define AD1CHSINV	PIC32_R (0x904C)
#define AD1CSSL		PIC32_R (0x9050) /* Input scan selection */
#define AD1CSSLCLR	PIC32_R (0x9054)
#define AD1CSSLSET	PIC32_R (0x9058)
#define AD1CSSLINV	PIC32_R (0x905C)
#define AD1PCFG		PIC32_R (0x9060) /* Port configuration */
#define AD1PCFGCLR	PIC32_R (0x9064)
#define AD1PCFGSET	PIC32_R (0x9068)
#define AD1PCFGINV	PIC32_R (0x906C)
#define ADC1BUF0	PIC32_R (0x9070) /* Result words */
#define ADC1BUF1	PIC32_R (0x9080)
#define ADC1BUF2	PIC32_R (0x9090)
#define ADC1BUF3	PIC32_R (0x90A0)
#define ADC1BUF4	PIC32_R (0x90B0)
#define ADC1BUF5	PIC32_R (0x90C0)
#define ADC1BUF6	PIC32_R (0x90D0)
#define ADC1BUF7	PIC32_R (0x90E0)
#define ADC1BUF8	PIC32_R (0x90F0)
#define ADC1BUF9	PIC32_R (0x9100)
#define ADC1BUFA	PIC32_R (0x9110)
#define ADC1BUFB	PIC32_R (0x9120)
#define ADC1BUFC	PIC32_R (0x9130)
#define ADC1BUFD	PIC32_R (0x9140)
#define ADC1BUFE	PIC32_R (0x9150)
#define ADC1BUFF	PIC32_R (0x9160)

/*--------------------------------------
 * USB registers.
 */
#define U1OTGIR		PIC32_R (0x85040) /* OTG interrupt flags */
#define U1OTGIE		PIC32_R (0x85050) /* OTG interrupt enable */
#define U1OTGSTAT	PIC32_R (0x85060) /* Comparator and pin status */
#define U1OTGCON	PIC32_R (0x85070) /* Resistor and pin control */
#define U1PWRC		PIC32_R (0x85080) /* Power control */
#define U1IR		PIC32_R (0x85200) /* Pending interrupt */
#define U1IE		PIC32_R (0x85210) /* Interrupt enable */
#define U1EIR		PIC32_R (0x85220) /* Pending error interrupt */
#define U1EIE		PIC32_R (0x85230) /* Error interrupt enable */
#define U1STAT		PIC32_R (0x85240) /* Status FIFO */
#define U1CON		PIC32_R (0x85250) /* Control */
#define U1ADDR		PIC32_R (0x85260) /* Address */
#define U1BDTP1		PIC32_R (0x85270) /* Buffer descriptor table pointer 1 */
#define U1FRML		PIC32_R (0x85280) /* Frame counter low */
#define U1FRMH		PIC32_R (0x85290) /* Frame counter high */
#define U1TOK		PIC32_R (0x852A0) /* Host control */
#define U1SOF		PIC32_R (0x852B0) /* SOF counter */
#define U1BDTP2		PIC32_R (0x852C0) /* Buffer descriptor table pointer 2 */
#define U1BDTP3		PIC32_R (0x852D0) /* Buffer descriptor table pointer 3 */
#define U1CNFG1		PIC32_R (0x852E0) /* Debug and idle */
#define U1EP0		PIC32_R (0x85300) /* Endpoint control */
#define U1EP1		PIC32_R (0x85310)
#define U1EP2		PIC32_R (0x85320)
#define U1EP3		PIC32_R (0x85330)
#define U1EP4		PIC32_R (0x85340)
#define U1EP5		PIC32_R (0x85350)
#define U1EP6		PIC32_R (0x85360)
#define U1EP7		PIC32_R (0x85370)
#define U1EP8		PIC32_R (0x85380)
#define U1EP9		PIC32_R (0x85390)
#define U1EP10		PIC32_R (0x853A0)
#define U1EP11		PIC32_R (0x853B0)
#define U1EP12		PIC32_R (0x853C0)
#define U1EP13		PIC32_R (0x853D0)
#define U1EP14		PIC32_R (0x853E0)
#define U1EP15		PIC32_R (0x853F0)

/*
 * USB Control register.
 */
#define PIC32_U1CON_USBEN	0x0001 /* USB module enable (device mode) */
#define PIC32_U1CON_SOFEN	0x0001 /* SOF sent every 1 ms (host mode) */
#define PIC32_U1CON_PPBRST	0x0002 /* Ping-pong buffers reset */
#define PIC32_U1CON_RESUME	0x0004 /* Resume signaling enable */
#define PIC32_U1CON_HOSTEN	0x0008 /* Host mode enable */
#define PIC32_U1CON_USBRST	0x0010 /* USB reset */
#define PIC32_U1CON_PKTDIS	0x0020 /* Packet transfer disable */
#define PIC32_U1CON_TOKBUSY	0x0020 /* Token busy indicator */
#define PIC32_U1CON_SE0		0x0040 /* Single ended zero detected */
#define PIC32_U1CON_JSTATE	0x0080 /* Live differential receiver JSTATE flag */

/*
 * USB Power control register.
 */
#define PIC32_U1PWRC_USBPWR	0x0001 /* USB operation enable */
#define PIC32_U1PWRC_USUSPEND	0x0002 /* USB suspend mode */
#define PIC32_U1PWRC_USLPGRD	0x0010 /* USB sleep entry guard */
#define PIC32_U1PWRC_UACTPND	0x0080 /* UAB activity pending */

/*
 * USB Pending interrupt register.
 * USB Interrupt enable register.
 */
#define PIC32_U1I_DETACH	0x0001 /* Detach (host mode) */
#define PIC32_U1I_URST		0x0001 /* USB reset (device mode) */
#define PIC32_U1I_UERR		0x0002 /* USB error condition */
#define PIC32_U1I_SOF		0x0004 /* SOF token  */
#define PIC32_U1I_TRN		0x0008 /* Token processing complete */
#define PIC32_U1I_IDLE		0x0010 /* Idle detect */
#define PIC32_U1I_RESUME	0x0020 /* Resume */
#define PIC32_U1I_ATTACH	0x0040 /* Peripheral attach */
#define PIC32_U1I_STALL		0x0080 /* STALL handshake */

/*
 * USB OTG interrupt flags register.
 * USB OTG interrupt enable register.
 */
#define PIC32_U1OTGI_VBUSVD	0x0001 /* A-device Vbus change */
#define PIC32_U1OTGI_SESEND	0x0004 /* B-device Vbus change */
#define PIC32_U1OTGI_SESVD	0x0008 /* Session valid change */
#define PIC32_U1OTGI_ACTV	0x0010 /* Bus activity indicator */
#define PIC32_U1OTGI_LSTATE	0x0020 /* Line state stable */
#define PIC32_U1OTGI_T1MSEC	0x0040 /* 1 millisecond timer expired */
#define PIC32_U1OTGI_ID		0x0080 /* ID state change */

#define PIC32_U1OTGSTAT_VBUSVD	0x0001 /*  */
#define PIC32_U1OTGSTAT_SESEND	0x0004 /*  */
#define PIC32_U1OTGSTAT_SESVD	0x0008 /*  */
#define PIC32_U1OTGSTAT_LSTATE	0x0020 /*  */
#define PIC32_U1OTGSTAT_ID	0x0080 /*  */

#define PIC32_U1OTGCON_VBUSDIS	0x0001 /*  */
#define PIC32_U1OTGCON_VBUSCHG	0x0002 /*  */
#define PIC32_U1OTGCON_OTGEN	0x0004 /*  */
#define PIC32_U1OTGCON_VBUSON	0x0008 /*  */
#define PIC32_U1OTGCON_DMPULDWN	0x0010 /*  */
#define PIC32_U1OTGCON_DPPULDWN	0x0020 /*  */
#define PIC32_U1OTGCON_DMPULUP	0x0040 /*  */
#define PIC32_U1OTGCON_DPPULUP	0x0080 /*  */

#define PIC32_U1EI_PID		0x0001 /*  */
#define PIC32_U1EI_CRC5		0x0002 /*  */
#define PIC32_U1EI_EOF		0x0002 /*  */
#define PIC32_U1EI_CRC16	0x0004 /*  */
#define PIC32_U1EI_DFN8		0x0008 /*  */
#define PIC32_U1EI_BTO		0x0010 /*  */
#define PIC32_U1EI_DMA		0x0020 /*  */
#define PIC32_U1EI_BMX		0x0040 /*  */
#define PIC32_U1EI_BTS		0x0080 /*  */

#define PIC32_U1STAT_PPBI	0x0004 /*  */
#define PIC32_U1STAT_DIR	0x0008 /*  */
#define PIC32_U1STAT_ENDPT0	0x0010 /*  */
#define PIC32_U1STAT_ENDPT	0x00F0 /*  */
#define PIC32_U1STAT_ENDPT1	0x0020 /*  */
#define PIC32_U1STAT_ENDPT2	0x0040 /*  */
#define PIC32_U1STAT_ENDPT3	0x0080 /*  */

#define PIC32_U1ADDR_DEVADDR	0x007F /*  */
#define PIC32_U1ADDR_USBADDR0	0x0001 /*  */
#define PIC32_U1ADDR_DEVADDR1	0x0002 /*  */
#define PIC32_U1ADDR_DEVADDR2	0x0004 /*  */
#define PIC32_U1ADDR_DEVADDR3	0x0008 /*  */
#define PIC32_U1ADDR_DEVADDR4	0x0010 /*  */
#define PIC32_U1ADDR_DEVADDR5	0x0020 /*  */
#define PIC32_U1ADDR_DEVADDR6	0x0040 /*  */
#define PIC32_U1ADDR_LSPDEN	0x0080 /*  */

#define PIC32_U1FRML_FRM0	0x0001 /*  */
#define PIC32_U1FRML_FRM1	0x0002 /*  */
#define PIC32_U1FRML_FRM2	0x0004 /*  */
#define PIC32_U1FRML_FRM3	0x0008 /*  */
#define PIC32_U1FRML_FRM4	0x0010 /*  */
#define PIC32_U1FRML_FRM5	0x0020 /*  */
#define PIC32_U1FRML_FRM6	0x0040 /*  */
#define PIC32_U1FRML_FRM7	0x0080 /*  */

#define PIC32_U1FRMH_FRM8	0x0001 /*  */
#define PIC32_U1FRMH_FRM9	0x0002 /*  */
#define PIC32_U1FRMH_FRM10	0x0004 /*  */

#define PIC32_U1TOK_EP0		0x0001 /*  */
#define PIC32_U1TOK_EP		0x000F /*  */
#define PIC32_U1TOK_EP1		0x0002 /*  */
#define PIC32_U1TOK_EP2		0x0004 /*  */
#define PIC32_U1TOK_EP3		0x0008 /*  */
#define PIC32_U1TOK_PID0	0x0010 /*  */
#define PIC32_U1TOK_PID		0x00F0 /*  */
#define PIC32_U1TOK_PID1	0x0020 /*  */
#define PIC32_U1TOK_PID2	0x0040 /*  */
#define PIC32_U1TOK_PID3	0x0080 /*  */

#define PIC32_U1CNFG1_USBSIDL	0x0010 /*  */
#define PIC32_U1CNFG1_USBFRZ	0x0020 /*  */
#define PIC32_U1CNFG1_UOEMON	0x0040 /*  */
#define PIC32_U1CNFG1_UTEYE	0x0080 /*  */

#define PIC32_U1EP_EPHSHK	0x0001 /*  */
#define PIC32_U1EP_EPSTALL	0x0002 /*  */
#define PIC32_U1EP_EPTXEN	0x0004 /*  */
#define PIC32_U1EP_EPRXEN	0x0008 /*  */
#define PIC32_U1EP_EPCONDIS	0x0010 /*  */
#define PIC32_U1EP_RETRYDIS	0x0040 /*  */
#define PIC32_U1EP_LSPD		0x0080 /*  */

/*--------------------------------------
 * SPI registers.
 */
#define SPI3CON		PIC32_R (0x5800) /* Control */
#define SPI3CONCLR	PIC32_R (0x5804)
#define SPI3CONSET	PIC32_R (0x5808)
#define SPI3CONINV	PIC32_R (0x580C)
#define SPI3STAT	PIC32_R (0x5810) /* Status */
#define SPI3STATCLR	PIC32_R (0x5814)
#define SPI3STATSET	PIC32_R (0x5818)
#define SPI3STATINV	PIC32_R (0x581C)
#define SPI3BUF		PIC32_R (0x5820) /* Transmit and receive buffer */
#define SPI3BRG		PIC32_R (0x5830) /* Baud rate generator */
#define SPI3BRGCLR	PIC32_R (0x5834)
#define SPI3BRGSET	PIC32_R (0x5838)
#define SPI3BRGINV	PIC32_R (0x583C)

#define SPI4CON		PIC32_R (0x5C00) /* Control */
#define SPI4CONCLR	PIC32_R (0x5C04)
#define SPI4CONSET	PIC32_R (0x5C08)
#define SPI4CONINV	PIC32_R (0x5C0C)
#define SPI4STAT	PIC32_R (0x5C10) /* Status */
#define SPI4STATCLR	PIC32_R (0x5C14)
#define SPI4STATSET	PIC32_R (0x5C18)
#define SPI4STATINV	PIC32_R (0x5C1C)
#define SPI4BUF		PIC32_R (0x5C20) /* Transmit and receive buffer */
#define SPI4BRG		PIC32_R (0x5C30) /* Baud rate generator */
#define SPI4BRGCLR	PIC32_R (0x5C34)
#define SPI4BRGSET	PIC32_R (0x5C38)
#define SPI4BRGINV	PIC32_R (0x5C3C)

#define SPI1CON		PIC32_R (0x5E00) /* Control */
#define SPI1CONCLR	PIC32_R (0x5E04)
#define SPI1CONSET	PIC32_R (0x5E08)
#define SPI1CONINV	PIC32_R (0x5E0C)
#define SPI1STAT	PIC32_R (0x5E10) /* Status */
#define SPI1STATCLR	PIC32_R (0x5E14)
#define SPI1STATSET	PIC32_R (0x5E18)
#define SPI1STATINV	PIC32_R (0x5E1C)
#define SPI1BUF		PIC32_R (0x5E20) /* Transmit and receive buffer */
#define SPI1BRG		PIC32_R (0x5E30) /* Baud rate generator */
#define SPI1BRGCLR	PIC32_R (0x5E34)
#define SPI1BRGSET	PIC32_R (0x5E38)
#define SPI1BRGINV	PIC32_R (0x5E3C)

#define SPI2CON		PIC32_R (0x5A00) /* Control */
#define SPI2CONCLR	PIC32_R (0x5A04)
#define SPI2CONSET	PIC32_R (0x5A08)
#define SPI2CONINV	PIC32_R (0x5A0C)
#define SPI2STAT	PIC32_R (0x5A10) /* Status */
#define SPI2STATCLR	PIC32_R (0x5A14)
#define SPI2STATSET	PIC32_R (0x5A18)
#define SPI2STATINV	PIC32_R (0x5A1C)
#define SPI2BUF		PIC32_R (0x5A20) /* Transmit and receive buffer */
#define SPI2BRG		PIC32_R (0x5A30) /* Baud rate generator */
#define SPI2BRGCLR	PIC32_R (0x5A34)
#define SPI2BRGSET	PIC32_R (0x5A38)
#define SPI2BRGINV	PIC32_R (0x5A3C)

/*
 * SPI Control register.
 */
#define PIC32_SPICON_MSTEN	0x00000020	/* Master mode */
#define PIC32_SPICON_CKP	0x00000040      /* Idle clock is high level */
#define PIC32_SPICON_SSEN	0x00000080      /* Slave mode: SSx pin enable */
#define PIC32_SPICON_CKE	0x00000100      /* Output data changes on
						 * transition from active clock
						 * state to Idle clock state */
#define PIC32_SPICON_SMP	0x00000200      /* Master mode: input data sampled
						 * at end of data output time. */
#define PIC32_SPICON_MODE16	0x00000400      /* 16-bit data width */
#define PIC32_SPICON_MODE32	0x00000800      /* 32-bit data width */
#define PIC32_SPICON_DISSDO	0x00001000      /* SDOx pin is not used */
#define PIC32_SPICON_SIDL	0x00002000      /* Stop in Idle mode */
#define PIC32_SPICON_FRZ	0x00004000      /* Freeze in Debug mode */
#define PIC32_SPICON_ON		0x00008000      /* SPI Peripheral is enabled */
#define PIC32_SPICON_ENHBUF	0x00010000      /* Enhanced buffer enable */
#define PIC32_SPICON_SPIFE	0x00020000      /* Frame synchronization pulse
						 * coincides with the first bit clock */
#define PIC32_SPICON_FRMPOL	0x20000000      /* Frame pulse is active-high */
#define PIC32_SPICON_FRMSYNC	0x40000000      /* Frame sync pulse input (Slave mode) */
#define PIC32_SPICON_FRMEN	0x80000000      /* Framed SPI support */

/*
 * SPI Status register.
 */
#define PIC32_SPISTAT_SPIRBF	0x00000001      /* Receive buffer is full */
#define PIC32_SPISTAT_SPITBF	0x00000002      /* Transmit buffer is full */
#define PIC32_SPISTAT_SPITBE	0x00000008      /* Transmit buffer is empty */
#define PIC32_SPISTAT_SPIRBE    0x00000020      /* Receive buffer is empty */
#define PIC32_SPISTAT_SPIROV	0x00000040      /* Receive overflow flag */
#define PIC32_SPISTAT_SPIBUSY	0x00000800      /* SPI is busy */

/*--------------------------------------
 * DMA controller registers.
 */
#define DMACON          PIC32_R (0x83000)       /* DMA Control */
#define DMACONCLR	PIC32_R (0x83004)
#define DMACONSET	PIC32_R (0x83008)
#define DMACONINV	PIC32_R (0x8300C)
#define DMASTAT         PIC32_R (0x83010)       /* DMA Status */
#define DMAADDR         PIC32_R (0x83020)       /* DMA Address */
// TODO: other DMA registers.

/*--------------------------------------
 * System controller registers.
 */
#define OSCCON          PIC32_R (0xF000)
#define OSCTUN          PIC32_R (0xF010)
#define DDPCON          PIC32_R (0xF200)        /* Debug Data Port Control */
#define DEVID           PIC32_R (0xF220)
#define SYSKEY          PIC32_R (0xF230)
#define RCON            PIC32_R (0xF600)
#define RCONCLR         PIC32_R (0xF604)
#define RCONSET         PIC32_R (0xF608)
#define RCONINV         PIC32_R (0xF60C)
#define RSWRST          PIC32_R (0xF610)
#define RSWRSTCLR       PIC32_R (0xF614)
#define RSWRSTSET       PIC32_R (0xF618)
#define RSWRSTINV       PIC32_R (0xF61C)

/*
 * Reset control register.
 */
#define PIC32_RCON_POR          0x00000001
#define PIC32_RCON_BOR          0x00000002
#define PIC32_RCON_IDLE         0x00000004
#define PIC32_RCON_SLEEP        0x00000008
#define PIC32_RCON_WDTO         0x00000010
#define PIC32_RCON_SWR          0x00000040
#define PIC32_RCON_EXTR         0x00000080
#define PIC32_RCON_VREGS        0x00000100
#define PIC32_RCON_CMR          0x00000200

/*--------------------------------------
 * Prefetch cache controller registers.
 */
#define CHECON          PIC32_R (0x84000)       /* Prefetch cache control */
#define CHECONCLR	PIC32_R (0x84004)
#define CHECONSET	PIC32_R (0x84008)
#define CHECONINV	PIC32_R (0x8400C)
// TODO: other prefetech registers

/*--------------------------------------
 * Bus matrix control registers.
 */
#define BMXCON          PIC32_R (0x82000)       /* Memory configuration */
#define BMXCONCLR	PIC32_R (0x82004)
#define BMXCONSET	PIC32_R (0x82008)
#define BMXCONINV	PIC32_R (0x8200C)
#define BMXDKPBA        PIC32_R (0x82010)       /* Data RAM kernel program base address */
#define BMXDKPBACLR	PIC32_R (0x82014)
#define BMXDKPBASET	PIC32_R (0x82018)
#define BMXDKPBAINV	PIC32_R (0x8201C)
#define BMXDUDBA        PIC32_R (0x82020)       /* Data RAM user data base address */
#define BMXDUDBACLR	PIC32_R (0x82024)
#define BMXDUDBASET	PIC32_R (0x82028)
#define BMXDUDBAINV	PIC32_R (0x8202C)
#define BMXDUPBA        PIC32_R (0x82030)       /* Data RAM user program base address */
#define BMXDUPBACLR	PIC32_R (0x82034)
#define BMXDUPBASET	PIC32_R (0x82038)
#define BMXDUPBAINV	PIC32_R (0x8203C)
#define BMXDRMSZ        PIC32_R (0x82040)       /* Data RAM size */
#define BMXPUPBA        PIC32_R (0x82050)       /* Program Flash user program base address */
#define BMXPUPBACLR	PIC32_R (0x82054)
#define BMXPUPBASET	PIC32_R (0x82058)
#define BMXPUPBAINV	PIC32_R (0x8205C)
#define BMXPFMSZ        PIC32_R (0x82060)       /* Program Flash size */
#define BMXBOOTSZ       PIC32_R (0x82070)       /* Boot Flash size */

/*--------------------------------------
 * Real time clock and calendar.
 */
#define RTCCON          PIC32_R (0x0200)  /* RTC control */
#define RTCALRM         PIC32_R (0x0210)  /* RTC alarm control */
#define RTCTIME         PIC32_R (0x0220)  /* RTC time value */
#define RTCDATE         PIC32_R (0x0230)  /* RTC date value */
#define ALRMTIME        PIC32_R (0x0240)  /* Alarm time value */
#define ALRMDATE        PIC32_R (0x0250)  /* Alarm date value */

/*--------------------------------------
 * Timer registers.
 */
#define T1CON           PIC32_R (0x0600)  /* Timer 1 control */
#define TMR1            PIC32_R (0x0610)  /* Timer 1 count */
#define PR1             PIC32_R (0x0620)  /* Timer 1 period */
#define T2CON           PIC32_R (0x0800)  /* Timer 2 control */
#define TMR2            PIC32_R (0x0810)  /* Timer 2 count */
#define PR2             PIC32_R (0x0820)  /* Timer 2 period */
#define T3CON           PIC32_R (0x0A00)  /* Timer 3 control */
#define TMR3            PIC32_R (0x0A10)  /* Timer 3 count */
#define PR3             PIC32_R (0x0A20)  /* Timer 3 period */
#define T4CON           PIC32_R (0x0C00)  /* Timer 4 control */
#define TMR4            PIC32_R (0x0C10)  /* Timer 4 count */
#define PR4             PIC32_R (0x0C20)  /* Timer 4 period */
#define T5CON           PIC32_R (0x0E00)  /* Timer 5 control */
#define TMR5            PIC32_R (0x0E10)  /* Timer 5 count */
#define PR5             PIC32_R (0x0E20)  /* Timer 5 period */

/*
 * Timer Control register.
 */
#define PIC32_TCON_ON           0x8000  /* Timer is enabled */
#define PIC32_TCON_FRZ          0x4000  /* Freeze when CPU is in Debug mode */
#define PIC32_TCON_SIDL         0x2000  /* Stop in Idle mode */
#define PIC32_TCON_TWDIS        0x1000  /* (Timer A) Asynchronous Timer Write Disable */
#define PIC32_TCON_TWIP         0x0800  /* (Timer A) Asynchronous Timer Write in Progress */
#define PIC32_TCON_TGATE        0x0080  /* Enable gated time accumulation (only when TCS=0) */
#define PIC32_TCON_TCKPS_MASK   0x0070  /* Timer Input Clock Prescale Select */
#define PIC32_TCON_TCKPS_256    0x0070  /* 1:256 */
#define PIC32_TCON_TCKPS_64     0x0060  /* 1:64 */
#define PIC32_TCON_TCKPS_32     0x0050  /* 1:32 */
#define PIC32_TCON_TCKPS_16     0x0040  /* 1:16 */
#define PIC32_TCON_TCKPS_8      0x0030  /* 1:8 */
#define PIC32_TCON_TCKPS_4      0x0020  /* 1:4 */
#define PIC32_TCON_TCKPS_2      0x0010  /* 1:2 */
#define PIC32_TCON_TCKPS_1      0x0000  /* 1:1 */
#define PIC32_TCON_T32          0x0008  /* (Timer B) TMRx and TMRy form a 32-bit timer */
#define PIC32_TCON_TSYNC        0x0004  /* (Timer A) External clock input is synchronized */
#define PIC32_TCON_TCS          0x0002  /* External clock from TxCKI pin */

/*--------------------------------------
 * Non-volatile memory control registers.
 */
#define NVMCON          PIC32_R (0xF400)
#define NVMCONCLR       PIC32_R (0xF404)
#define NVMCONSET       PIC32_R (0xF408)
#define NVMCONINV       PIC32_R (0xF40C)
#define NVMKEY          PIC32_R (0xF410)
#define NVMADDR         PIC32_R (0xF420)
#define NVMADDRCLR      PIC32_R (0xF424)
#define NVMADDRSET      PIC32_R (0xF428)
#define NVMADDRINV      PIC32_R (0xF42C)
#define NVMDATA         PIC32_R (0xF430)
#define NVMSRCADDR      PIC32_R (0xF440)

#define PIC32_NVMCON_NVMOP      0x0000000F
#define PIC32_NVMCON_NOP                 0 /* No operation */
#define PIC32_NVMCON_WORD_PGM            1 /* Word program */
#define PIC32_NVMCON_ROW_PGM             3 /* Row program */
#define PIC32_NVMCON_PAGE_ERASE          4 /* Page erase */

#define PIC32_NVMCON_LVDSTAT    0x00000800
#define PIC32_NVMCON_LVDERR     0x00001000
#define PIC32_NVMCON_WRERR      0x00002000
#define PIC32_NVMCON_WREN       0x00004000
#define PIC32_NVMCON_WR         0x00008000

/*--------------------------------------
 * Interrupt controller registers.
 */
#define INTCON		PIC32_R (0x81000)	/* Interrupt Control */
#define INTCONCLR	PIC32_R (0x81004)
#define INTCONSET	PIC32_R (0x81008)
#define INTCONINV	PIC32_R (0x8100C)
#define INTSTAT		PIC32_R (0x81010)	/* Interrupt Status */
#define IPTMR		PIC32_R (0x81020)	/* Temporal Proximity Timer */
#define IPTMRCLR	PIC32_R (0x81024)
#define IPTMRSET	PIC32_R (0x81028)
#define IPTMRINV	PIC32_R (0x8102C)
#define IFS(n)		PIC32_R (0x81030+((n)<<4)) /* IFS(0..2) - Interrupt Flag Status */
#define IFSCLR(n)	PIC32_R (0x81034+((n)<<4))
#define IFSSET(n)	PIC32_R (0x81038+((n)<<4))
#define IFSINV(n)	PIC32_R (0x8103C+((n)<<4))
#define IEC(n)		PIC32_R (0x81060+((n)<<4)) /* IEC(0..2) - Interrupt Enable Control */
#define IECCLR(n)	PIC32_R (0x81064+((n)<<4))
#define IECSET(n)	PIC32_R (0x81068+((n)<<4))
#define IECINV(n)	PIC32_R (0x8106C+((n)<<4))
#define IPC(n)		PIC32_R (0x81090+((n)<<4)) /* IPC(0..12) - Interrupt Priority Control */
#define IPCCLR(n)	PIC32_R (0x81094+((n)<<4))
#define IPCSET(n)	PIC32_R (0x81098+((n)<<4))
#define IPCINV(n)	PIC32_R (0x8109C+((n)<<4))

/*
 * Interrupt Control register.
 */
#define PIC32_INTCON_INT0EP	0x0001	/* External interrupt 0 polarity rising edge */
#define PIC32_INTCON_INT1EP	0x0002	/* External interrupt 1 polarity rising edge */
#define PIC32_INTCON_INT2EP	0x0004	/* External interrupt 2 polarity rising edge */
#define PIC32_INTCON_INT3EP	0x0008	/* External interrupt 3 polarity rising edge */
#define PIC32_INTCON_INT4EP	0x0010	/* External interrupt 4 polarity rising edge */
#define PIC32_INTCON_TPC(x)	((x)<<8) /* Temporal proximity group priority */
#define PIC32_INTCON_MVEC	0x1000	/* Multi-vectored mode */
#define PIC32_INTCON_FRZ	0x4000	/* Freeze in debug mode */
#define PIC32_INTCON_SS0	0x8000	/* Single vector has a shadow register set */

/*
 * Interrupt Status register.
 */
#define PIC32_INTSTAT_VEC(s)	((s) & 0xFF)	/* Interrupt vector */
#define PIC32_INTSTAT_SRIPL(s)	((s) >> 8 & 7)	/* Requested priority level */
#define PIC32_INTSTAT_SRIPL_MASK 0x0700

/*
 * Interrupt Prority Control register.
 */
#define PIC32_IPC_IS0(x)	(x)		/* Interrupt 0 subpriority */
#define PIC32_IPC_IP0(x)	((x)<<2)	/* Interrupt 0 priority */
#define PIC32_IPC_IS1(x)	((x)<<8)	/* Interrupt 1 subpriority */
#define PIC32_IPC_IP1(x)	((x)<<10)	/* Interrupt 1 priority */
#define PIC32_IPC_IS2(x)	((x)<<16)	/* Interrupt 2 subpriority */
#define PIC32_IPC_IP2(x)	((x)<<18)	/* Interrupt 2 priority */
#define PIC32_IPC_IS3(x)	((x)<<24)	/* Interrupt 3 subpriority */
#define PIC32_IPC_IP3(x)	((x)<<26)	/* Interrupt 3 priority */

/*
 * IRQ numbers for PIC32MZ
 */
#define PIC32_IRQ_CT        0   /* Core Timer Interrupt */
#define PIC32_IRQ_CS0       1   /* Core Software Interrupt 0 */
#define PIC32_IRQ_CS1       2   /* Core Software Interrupt 1 */
#define PIC32_IRQ_INT0      3   /* External Interrupt 0 */
#define PIC32_IRQ_T1        4   /* Timer1 */
#define PIC32_IRQ_IC1       5   /* Input Capture 1 */
#define PIC32_IRQ_OC1       6   /* Output Compare 1 */
#define PIC32_IRQ_INT1      7   /* External Interrupt 1 */
#define PIC32_IRQ_T2        8   /* Timer2 */
#define PIC32_IRQ_IC2       9   /* Input Capture 2 */
#define PIC32_IRQ_OC2       10  /* Output Compare 2 */
#define PIC32_IRQ_INT2      11  /* External Interrupt 2 */
#define PIC32_IRQ_T3        12  /* Timer3 */
#define PIC32_IRQ_IC3       13  /* Input Capture 3 */
#define PIC32_IRQ_OC3       14  /* Output Compare 3 */
#define PIC32_IRQ_INT3      15  /* External Interrupt 3 */
#define PIC32_IRQ_T4        16  /* Timer4 */
#define PIC32_IRQ_IC4       17  /* Input Capture 4 */
#define PIC32_IRQ_OC4       18  /* Output Compare 4 */
#define PIC32_IRQ_INT4      19  /* External Interrupt 4 */
#define PIC32_IRQ_T5        20  /* Timer5 */
#define PIC32_IRQ_IC5       21  /* Input Capture 5 */
#define PIC32_IRQ_OC5       22  /* Output Compare 5 */
#define PIC32_IRQ_SPI1E     23  /* SPI1 Fault */
#define PIC32_IRQ_SPI1TX    24  /* SPI1 Transfer Done */
#define PIC32_IRQ_SPI1RX    25  /* SPI1 Receive Done */

#define PIC32_IRQ_SPI3E     26  /* SPI3 Fault */
#define PIC32_IRQ_SPI3TX    27  /* SPI3 Transfer Done */
#define PIC32_IRQ_SPI3RX    28  /* SPI3 Receive Done */
#define PIC32_IRQ_U1E       26  /* UART1 Error */
#define PIC32_IRQ_U1RX      27  /* UART1 Receiver */
#define PIC32_IRQ_U1TX      28  /* UART1 Transmitter */
#define PIC32_IRQ_I2C3B     26  /* I2C3 Bus Collision Event */
#define PIC32_IRQ_I2C3S     27  /* I2C3 Slave Event */
#define PIC32_IRQ_I2C3M     28  /* I2C3 Master Event */

#define PIC32_IRQ_I2C1B     29  /* I2C1 Bus Collision Event */
#define PIC32_IRQ_I2C1S     30  /* I2C1 Slave Event */
#define PIC32_IRQ_I2C1M     31  /* I2C1 Master Event */
#define PIC32_IRQ_CN        32  /* Input Change Interrupt */
#define PIC32_IRQ_AD1       33  /* ADC1 Convert Done */
#define PIC32_IRQ_PMP       34  /* Parallel Master Port */
#define PIC32_IRQ_CMP1      35  /* Comparator Interrupt */
#define PIC32_IRQ_CMP2      36  /* Comparator Interrupt */

#define PIC32_IRQ_SPI2E     37  /* SPI2 Fault */
#define PIC32_IRQ_SPI2TX    38  /* SPI2 Transfer Done */
#define PIC32_IRQ_SPI2RX    39  /* SPI2 Receive Done */
#define PIC32_IRQ_U3E       37  /* UART3 Error */
#define PIC32_IRQ_U3RX      38  /* UART3 Receiver */
#define PIC32_IRQ_U3TX      39  /* UART3 Transmitter */
#define PIC32_IRQ_I2C4B     37  /* I2C4 Bus Collision Event */
#define PIC32_IRQ_I2C4S     38  /* I2C4 Slave Event */
#define PIC32_IRQ_I2C4M     39  /* I2C4 Master Event */

#define PIC32_IRQ_SPI4E     40  /* SPI4 Fault */
#define PIC32_IRQ_SPI4TX    41  /* SPI4 Transfer Done */
#define PIC32_IRQ_SPI4RX    42  /* SPI4 Receive Done */
#define PIC32_IRQ_U2E       40  /* UART2 Error */
#define PIC32_IRQ_U2RX      41  /* UART2 Receiver */
#define PIC32_IRQ_U2TX      42  /* UART2 Transmitter */
#define PIC32_IRQ_I2C5B     40  /* I2C5 Bus Collision Event */
#define PIC32_IRQ_I2C5S     41  /* I2C5 Slave Event */
#define PIC32_IRQ_I2C5M     42  /* I2C5 Master Event */

#define PIC32_IRQ_I2C2B     43  /* I2C2 Bus Collision Event */
#define PIC32_IRQ_I2C2S     44  /* I2C2 Slave Event */
#define PIC32_IRQ_I2C2M     45  /* I2C2 Master Event */
#define PIC32_IRQ_FSCM      46  /* Fail-Safe Clock Monitor */
#define PIC32_IRQ_RTCC      47  /* Real-Time Clock and Calendar */
#define PIC32_IRQ_DMA0      48  /* DMA Channel 0 */
#define PIC32_IRQ_DMA1      49  /* DMA Channel 1 */
#define PIC32_IRQ_DMA2      50  /* DMA Channel 2 */
#define PIC32_IRQ_DMA3      51  /* DMA Channel 3 */
#define PIC32_IRQ_DMA4      52  /* DMA Channel 4 */
#define PIC32_IRQ_DMA5      53  /* DMA Channel 5 */
#define PIC32_IRQ_DMA6      54  /* DMA Channel 6 */
#define PIC32_IRQ_DMA7      55  /* DMA Channel 7 */
#define PIC32_IRQ_FCE       56  /* Flash Control Event */
#define PIC32_IRQ_USB       57  /* USB */
#define PIC32_IRQ_CAN1      58  /* Control Area Network 1 */
#define PIC32_IRQ_CAN2      59  /* Control Area Network 2 */
#define PIC32_IRQ_ETH       60  /* Ethernet Interrupt */
#define PIC32_IRQ_IC1E      61  /* Input Capture 1 Error */
#define PIC32_IRQ_IC2E      62  /* Input Capture 2 Error */
#define PIC32_IRQ_IC3E      63  /* Input Capture 3 Error */
#define PIC32_IRQ_IC4E      64  /* Input Capture 4 Error */
#define PIC32_IRQ_IC5E      65  /* Input Capture 5 Error */
#define PIC32_IRQ_PMPE      66  /* Parallel Master Port Error */
#define PIC32_IRQ_U4E       67  /* UART4 Error */
#define PIC32_IRQ_U4RX      68  /* UART4 Receiver */
#define PIC32_IRQ_U4TX      69  /* UART4 Transmitter */
#define PIC32_IRQ_U6E       70  /* UART6 Error */
#define PIC32_IRQ_U6RX      71  /* UART6 Receiver */
#define PIC32_IRQ_U6TX      72  /* UART6 Transmitter */
#define PIC32_IRQ_U5E       73  /* UART5 Error */
#define PIC32_IRQ_U5RX      74  /* UART5 Receiver */
#define PIC32_IRQ_U5TX      75  /* UART5 Transmitter */

/*
 * Interrupt vector numbers for PIC32MZ
 */
#define PIC32_VECT_CT       0   /* Core Timer Interrupt */
#define PIC32_VECT_CS0      1   /* Core Software Interrupt 0 */
#define PIC32_VECT_CS1      2   /* Core Software Interrupt 1 */
#define PIC32_VECT_INT0     3   /* External Interrupt 0 */
#define PIC32_VECT_T1       4   /* Timer1 */
#define PIC32_VECT_IC1      5   /* Input Capture 1 */
#define PIC32_VECT_OC1      6   /* Output Compare 1 */
#define PIC32_VECT_INT1     7   /* External Interrupt 1 */
#define PIC32_VECT_T2       8   /* Timer2 */
#define PIC32_VECT_IC2      9   /* Input Capture 2 */
#define PIC32_VECT_OC2      10  /* Output Compare 2 */
#define PIC32_VECT_INT2     11  /* External Interrupt 2 */
#define PIC32_VECT_T3       12  /* Timer3 */
#define PIC32_VECT_IC3      13  /* Input Capture 3 */
#define PIC32_VECT_OC3      14  /* Output Compare 3 */
#define PIC32_VECT_INT3     15  /* External Interrupt 3 */
#define PIC32_VECT_T4       16  /* Timer4 */
#define PIC32_VECT_IC4      17  /* Input Capture 4 */
#define PIC32_VECT_OC4      18  /* Output Compare 4 */
#define PIC32_VECT_INT4     19  /* External Interrupt 4 */
#define PIC32_VECT_T5       20  /* Timer5 */
#define PIC32_VECT_IC5      21  /* Input Capture 5 */
#define PIC32_VECT_OC5      22  /* Output Compare 5 */
#define PIC32_VECT_SPI1     23  /* SPI1 */

#define PIC32_VECT_SPI3     24  /* SPI3 */
#define PIC32_VECT_U1       24  /* UART1 */
#define PIC32_VECT_I2C3     24  /* I2C3 */

#define PIC32_VECT_I2C1     25  /* I2C1 */
#define PIC32_VECT_CN       26  /* Input Change Interrupt */
#define PIC32_VECT_AD1      27  /* ADC1 Convert Done */
#define PIC32_VECT_PMP      28  /* Parallel Master Port */
#define PIC32_VECT_CMP1     29  /* Comparator Interrupt */
#define PIC32_VECT_CMP2     30  /* Comparator Interrupt */

#define PIC32_VECT_SPI2     31  /* SPI2 */
#define PIC32_VECT_U3       31  /* UART3 */
#define PIC32_VECT_I2C4     31  /* I2C4 */

#define PIC32_VECT_SPI4     32  /* SPI4 */
#define PIC32_VECT_U2       32  /* UART2 */
#define PIC32_VECT_I2C5     32  /* I2C5 */

#define PIC32_VECT_I2C2     33  /* I2C2 */
#define PIC32_VECT_FSCM     34  /* Fail-Safe Clock Monitor */
#define PIC32_VECT_RTCC     35  /* Real-Time Clock and Calendar */
#define PIC32_VECT_DMA0     36  /* DMA Channel 0 */
#define PIC32_VECT_DMA1     37  /* DMA Channel 1 */
#define PIC32_VECT_DMA2     38  /* DMA Channel 2 */
#define PIC32_VECT_DMA3     39  /* DMA Channel 3 */
#define PIC32_VECT_DMA4     40  /* DMA Channel 4 */
#define PIC32_VECT_DMA5     41  /* DMA Channel 5 */
#define PIC32_VECT_DMA6     42  /* DMA Channel 6 */
#define PIC32_VECT_DMA7     43  /* DMA Channel 7 */
#define PIC32_VECT_FCE      44  /* Flash Control Event */
#define PIC32_VECT_USB      45  /* USB */
#define PIC32_VECT_CAN1     46  /* Control Area Network 1 */
#define PIC32_VECT_CAN2     47  /* Control Area Network 2 */
#define PIC32_VECT_ETH      48  /* Ethernet Interrupt */
#define PIC32_VECT_U4       49  /* UART4 */
#define PIC32_VECT_U6       50  /* UART6 */
#define PIC32_VECT_U5       51  /* UART5 */

#endif // TODO

#endif /* _IO_PIC32MZ_H */
