/*
 * Run an Imperas MIPS simulator.
 *
 * Copyright (C) 2014 Serge Vakulenko, <serge@vak.ru>
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include "icm/icmCpuManager.h"
#include "globals.h"

char *progname;                         // base name of current program

static uint32_t progmem [PROGRAM_FLASH_SIZE/4];
static uint32_t bootmem [BOOT_FLASH_SIZE/4];
static char datamem [DATA_MEM_SIZE];    // storage for RAM area
uint32_t iomem [0x100000/4];            // backing storage for I/O area

int trace_instructions;                 // print cpu instructions and registers
int trace_peripherals;                  // trace special function registers

icmProcessorP processor;                // top level processor object
icmNetP eic_ripl;                       // EIC request priority level
icmNetP eic_vector;                     // EIC vector number

static void usage()
{
#ifdef PIC32MX7
    icmPrintf("Simulator of PIC32MX7 microcontroller\n");
#endif
#ifdef PIC32MZ
    icmPrintf("Simulator of PIC32MZ microcontroller\n");
#endif
    icmPrintf("Usage:\n");
    icmPrintf("        %s [-vtm] application.hex [sd0.img [sd1.img]]\n", progname);
    icmPrintf("Options:\n");
    icmPrintf("        -v      verbose mode\n");
    icmPrintf("        -i      trace CPU instructions and registers\n");
    icmPrintf("        -r      trace special function registers\n");
    icmPrintf("        -m      enable magic opcodes\n");
    exit(-1);
}

//
// Callback for printing user defined attributes.
//
static void print_user_attribute (const char *owner, const char *name,
    const char *value, Bool set, Bool used, Bool numeric, void *userData)
{
    if (! set || ! used)
        return;
    icmPrintf("    %s.%s = ", owner, name);
    if (value == 0)
        value = "UNDEF";

    if (numeric) {
        icmPrintf("%s\n", value);
    } else {
        icmPrintf("'%s'\n", value);
    }
}

//
// Callback for reading peripheral registers.
//
static void mem_read (icmProcessorP proc, Addr address, Uns32 bytes,
    void *value, void *user_data, Addr VA, Bool isFetch)
{
    Uns32 offset = address & 0xfffff;
    const char *name = "???";
    Uns32 data;

    switch (bytes) {
    case 1:
        data = io_read32 (address, (Uns32*) (user_data + (offset & ~3)), &name);
        if ((offset &= 3) != 0) {
            // Unaligned read.
            data >>= offset * 8;
        }
        if (trace_peripherals) {
            icmPrintf("--- I/O Read  %02x from %s\n", data, name);
        }
        *(Uns8*) value = data;
        break;
    case 2:
        data = io_read32 (address, (Uns32*) (user_data + (offset & ~1)), &name);
        if (offset & 1) {
            // Unaligned read.
            data >>= 16;
        }
        if (trace_peripherals) {
            icmPrintf("--- I/O Read  %04x from %s\n", data, name);
        }
        *(Uns16*) value = data;
        break;
    case 4:
        data = io_read32 (address, (Uns32*) (user_data + offset), &name);
        if (trace_peripherals) {
            icmPrintf("--- I/O Read  %08x from %s\n", data, name);
        }
        *(Uns32*) value = data;
        break;
    default:
        icmPrintf("--- I/O Read  %08x: incorrect size %u bytes\n",
            (Uns32) address, bytes);
        icmExit(proc);
    }
}

//
// Callback for writing peripheral registers.
//
static void mem_write (icmProcessorP proc, Addr address, Uns32 bytes,
    const void *value, void *user_data, Addr VA)
{
    Uns32 data;
    const char *name = "???";

    if (bytes != 4) {
        icmPrintf("--- I/O Write %08x: incorrect size %u bytes\n",
            (Uns32) address, bytes);
        icmExit(proc);
    }
    data = *(Uns32*) value;
    io_write32 (address, (Uns32*) (user_data + (address & 0xffffc)),
        data, &name);
    if (trace_peripherals && name != 0) {
        icmPrintf("--- I/O Write %08x to %s \n", data, name);
    }
}

//
// Callback for timer interrupt.
//
void timer_irq (void *arg, Uns32 value)
{
    //icmPrintf("--- timer interrupt: %u\n", value);
    if (value)
        irq_raise (0);
    else
        irq_clear (0);
}

/*
 * When uarts are idle, insert uspeep()
 * to decrease the cpu load.
 */
static void pause_idle()
{
    static unsigned idle_timeout;
    fd_set rfds;

    if (idle_timeout > 0) {
	idle_timeout--;
	return;
    }
    idle_timeout = 2000;

    /* Wait for incoming data */
    vtty_wait (&rfds);
}

void quit()
{
    icmPrintf("***** Stop *****\n");
    icmTerminate();
}

void killed(int sig)
{
    icmPrintf("\n***** Killed *****\n");
    exit(1);
}

//
// Main simulation routine
//
int main(int argc, char **argv)
{
    // Extract a base name of a program.
    progname = strrchr (*argv, '/');
    if (progname)
        progname++;
    else
        progname = *argv;

    //
    // Parse command line arguments.
    // Setup the configuration attributes for the simulator
    //
    Uns32 icm_attrs   = 0;
    Uns32 sim_attrs   = ICM_STOP_ON_CTRLC;
    Uns32 model_flags = 0;
    Uns32 magic_opcodes = 0;

    for (;;) {
        switch (getopt (argc, argv, "virm")) {
        case EOF:
            break;
        case 'v':
            sim_attrs |= ICM_VERBOSE;
            continue;
        case 'i':
            trace_instructions++;
            continue;
        case 'r':
            trace_peripherals++;
            continue;
        case 'm':
            magic_opcodes++;
            continue;
        default:
            usage ();
        }
        break;
    }
    argc -= optind;
    argv += optind;

    if (argc < 1 || argc > 3) {
        if (argc > 0)
            icmPrintf("%s: Wrong number of args (%d)\n", progname, argc);
        usage ();
    }
    const char *app_file = argv[0];
    const char *sd0_file = argc >= 2 ? argv[1] : 0;
    const char *sd1_file = argc >= 3 ? argv[2] : 0;

    //
    // Initialize CpuManager
    //
    icmInit(sim_attrs, NULL, 0);
    atexit(quit);

    // Use ^\ to kill the simulation.
    signal(SIGQUIT, killed);

    //
    // Setup the configuration attributes for the MIPS model
    //
    icmAttrListP user_attrs = icmNewAttrList();
    char *cpu_type;

#ifdef PIC32MX7
    cpu_type = "M4K";
    icmAddUns64Attr(user_attrs, "pridRevision", 0x65);  // Product revision
    icmAddUns64Attr(user_attrs, "srsctlHSS",    1);     // Number of shadow register sets
    icmAddUns64Attr(user_attrs, "MIPS16eASE",   1);     // Support mips16e
    icmAddUns64Attr(user_attrs, "configSB",     1);     // Simple bus transfers only
#endif
#ifdef PIC32MZ
    cpu_type = "microAptivP";
    icmAddUns64Attr(user_attrs, "pridRevision", 0x28);  // Product revision
    icmAddUns64Attr(user_attrs, "srsctlHSS",    7);     // Number of shadow register sets
#if 0
    icmAddStringAttr(user_attrs,"cacheenable", "full"); // Enable cache
    icmAddUns64Attr(user_attrs, "config1IS",    2);     // Icache: 256 sets per way
    icmAddUns64Attr(user_attrs, "config1IL",    3);     // Icache: line size 16 bytes
    icmAddUns64Attr(user_attrs, "config1IA",    3);     // Icache: 4-way associativity
    icmAddUns64Attr(user_attrs, "config1DS",    0);     // Dcache: 64 sets per way
    icmAddUns64Attr(user_attrs, "config1DL",    3);     // Dcache: line size 16 bytes
    icmAddUns64Attr(user_attrs, "config1DA",    3);     // Dcache: 4-way associativity
#endif
    icmAddUns64Attr(user_attrs, "config1WR",    0);     // Disable watch registers
    icmAddUns64Attr(user_attrs, "config3ULRI",  1);     // UserLocal register implemented
    icmAddUns64Attr(user_attrs, "config7HCI",   1);     // Cache initialized by hardware
#endif

    // Processor configuration
    icmAddStringAttr(user_attrs,"variant", cpu_type);

    // PIC32 is always little endian
    icmAddStringAttr(user_attrs, "endian", "Little");

    // Enable external interrupt controller (EIC) and vectored interrupts mode
    icmAddStringAttr(user_attrs, "vectoredinterrupt", "enable");
    icmAddStringAttr(user_attrs, "externalinterrupt", "enable");
    icmAddUns64Attr(user_attrs, "EIC_OPTION", 2);

    // Interrupt pin for Timer interrupt
    icmAddUns64Attr(user_attrs, "intctlIPTI", 0);

    if (trace_instructions) {
        // Enable MIPS-format trace
        icmAddStringAttr(user_attrs, "MIPS_TRACE", "enable");
        icm_attrs |= ICM_ATTR_TRACE |
            ICM_ATTR_TRACE_REGS_BEFORE | ICM_ATTR_TRACE_REGS_AFTER;

        // Trace Count/Compare, TLB and FPU
        model_flags |= 0x0c000020;
    }
    if (magic_opcodes) {
        // Enable magic Pass/Fail opcodes
        icmAddStringAttr(user_attrs, "IMPERAS_MIPS_AVP_OPCODES", "enable");
    }

    // Select processor model from library
    const char *model_file = icmGetVlnvString(NULL,
        "mips.ovpworld.org", "processor", "mips32", "1.0", "model");

    //
    // create a processor
    //
    processor = icmNewProcessor(
        cpu_type,                     // processor name
        "mips",                       // processor type
        0,                            // processor cpuId
        model_flags,                  // processor model flags
        32,                           // physical address bits
        model_file,                   // model file
        "modelAttrs",                 // morpher attributes
        icm_attrs,                    // processor attributes
        user_attrs,                   // user attribute list
        0,                            // semihosting file name
        0);                           // semihosting attribute symbol
    icmBusP bus = icmNewBus("bus", 32);
    icmConnectProcessorBusses(processor, bus, bus);

    // Interrupt controller.
    eic_ripl = icmNewNet ("EIC_RIPL");
    icmConnectProcessorNet (processor, eic_ripl, "EIC_RIPL", ICM_INPUT);
    eic_vector = icmNewNet ("EIC_VectorNum");
    icmConnectProcessorNet (processor, eic_vector, "EIC_VectorNum", ICM_INPUT);

    // Callback for timer interrupt,
    icmNetP ti_output = icmNewNet ("causeTI");
    icmConnectProcessorNet (processor, ti_output, "causeTI", ICM_OUTPUT);
    icmAddNetCallback (ti_output, timer_irq, NULL);

    // Data memory.
    icmMapNativeMemory (bus, ICM_PRIV_RWX, DATA_MEM_START,
        DATA_MEM_START + DATA_MEM_SIZE - 1, datamem);
#ifdef PIC32MX7
    // User space 96 kbytes.
    icmMapNativeMemory (bus, ICM_PRIV_RWX, USER_MEM_START + 0x8000,
        USER_MEM_START + DATA_MEM_SIZE - 1, datamem + 0x8000);
#endif
    // Program memory.
    icmMapNativeMemory (bus, ICM_PRIV_RX, PROGRAM_FLASH_START,
        PROGRAM_FLASH_START + PROGRAM_FLASH_SIZE - 1, progmem);

    // Boot memory.
    icmMapNativeMemory (bus, ICM_PRIV_RX, BOOT_FLASH_START,
        BOOT_FLASH_START + BOOT_FLASH_SIZE - 1, bootmem);

    // I/O memory.
    icmMapExternalMemory(bus, "IO", ICM_PRIV_RW, IO_MEM_START,
        IO_MEM_START + IO_MEM_SIZE - 1, mem_read, mem_write, iomem);

    if (sim_attrs & ICM_VERBOSE) {
        // Print all user attributes.
        icmPrintf("\n***** User attributes *****\n");
        icmIterAllUserAttributes(print_user_attribute, 0);

        // Show Mapping on bus
        icmPrintf("\n***** Configuration of memory bus *****\n");
        icmPrintBusConnections(bus);
    }

    //
    // Initialize SD card.
    //
    int cs0_port, cs0_pin, cs1_port, cs1_pin;
#if defined EXPLORER16
    sdcard_spi_port = 0;                        // SD card at SPI1,
    cs0_port = 1; cs0_pin = 1;                  // select0 at B1,
    cs1_port = 1; cs1_pin = 2;                  // select1 at B2
#elif defined MAX32
    sdcard_spi_port = 3;                        // SD card at SPI4,
    cs0_port = 3; cs0_pin = 3;                  // select0 at D3,
    cs1_port = 3; cs1_pin = 4;                  // select1 at D4
#elif defined MAXIMITE
    sdcard_spi_port = 3;                        // SD card at SPI4,
    cs0_port = 4; cs0_pin = 0;                  // select0 at E0,
    cs1_port = -1; cs1_pin = -1;                // select1 not available
#elif defined WIFIRE
    //TODO
    sdcard_spi_port = 3;                        // SD card at SPI4,
    cs0_port = 4; cs0_pin = 0;                  // select0 at E0,
    cs1_port = -1; cs1_pin = -1;                // select1 not available
#endif
    sdcard_init (0, "sd0", sd0_file, cs0_port, cs0_pin);
    sdcard_init (1, "sd1", sd1_file, cs1_port, cs1_pin);

    //
    // Create virtual console on UART2
    //
#if defined(WIFIRE)
    vtty_create (4, "uart4", 0);
#elif defined(EXPLORER16) && defined(PIC32MX7)
    vtty_create (1, "uart2", 0);
#else
    vtty_create (0, "uart1", 0);
#endif
    vtty_init();

    //
    // Generic reset of all peripherals.
    //
    io_init (bootmem);

    //
    // Load Program
    //
    if (! load_file(progmem, bootmem, app_file)) {
        icmPrintf("Failed for '%s'", app_file);
        return -1;
    }

    //
    // Do a simulation run
    //
    icmSetPC(processor, 0xbfc00000);
    icmPrintf("\n***** Start '%s' *****\n", cpu_type);

    // Run the processor one instruction at a time until finished
    icmStopReason stop_reason;
    do {
        // simulate fixed number of instructions
        stop_reason = icmSimulate(processor, 100);
	if (stop_reason == ICM_SR_HALT) {
	    /* Suspended on WAIT instructon. */
	    stop_reason = ICM_SR_SCHED;

	    if (! uart_active())
		pause_idle();
	}

	// poll uarts
	uart_poll();
    } while (stop_reason == ICM_SR_SCHED);

    //
    // quit() implicitly called on return
    //
    return 0;
}

//
// EIC Interrupt
//
void eic_level_vector (int ripl, int vector)
{
    if (trace_peripherals || trace_instructions)
        printf ("--- RIPL = %u\n", ripl);

    icmWriteNet (eic_vector, 0);
    icmWriteNet (eic_ripl, ripl);
}

void soft_reset()
{
    Uns32 address = 0xfffffff0;
    Uns32 value;

    value = 4;
    if (! icmWriteProcessorMemory (processor, address, &value, 4)) {
        printf ("--- Cannot write %#x to %#x\n", value, address);
    }
}
