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
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

#include "icm/icmCpuManager.h"
#include "pic32mx.h"
#include "globals.h"

char *progname;                         // base name of current program

static uint32_t progmem [PROGRAM_MEM_SIZE/4];
static uint32_t bootmem [BOOT_MEM_SIZE/4];
static char iomem [0x10000];            // backing storage for I/O area
static char iomem2 [0x10000];           // backing storage for second I/O area

int trace;                              // global trace flag

static void usage()
{
    icmPrintf("PIC32 simulator\n");
    icmPrintf("Usage:\n");
    icmPrintf("        %s [-vt] application.elf [cpu-type]\n", progname);
    icmPrintf("Options:\n");
    icmPrintf("        -v      verbose mode\n");
    icmPrintf("        -t      trace instructions and registers\n");
    icmPrintf("CPU types:\n");
    icmPrintf("        M4K, M14K, M14KcFMM, M14KcTLB, microAptivC, microAptivP, microAptivCF\n");
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
static void mem_read (icmProcessorP processor, Addr address, Uns32 bytes,
    void *value, void *user_data, Addr VA, Bool isFetch)
{
    Uns32 offset = address & 0xffff;
    const char *name = "???";
    Uns32 data;

    switch (bytes) {
    case 1:
        data = io_read32 (address, (Uns32*) (user_data + (offset & ~3)), &name);
        if ((offset &= 3) != 0) {
            // Unaligned read.
            data >>= offset * 8;
        }
        if (trace) {
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
        if (trace) {
            icmPrintf("--- I/O Read  %04x from %s\n", data, name);
        }
        *(Uns16*) value = data;
        break;
    case 4:
        data = io_read32 (address, (Uns32*) (user_data + offset), &name);
        if (trace) {
            icmPrintf("--- I/O Read  %08x from %s\n", data, name);
        }
        *(Uns32*) value = data;
        break;
    default:
        icmPrintf("--- I/O Read  %08x: incorrect size %u bytes\n",
            (Uns32) address, bytes);
        icmExit(processor);
    }
}

//
// Callback for writing peripheral registers.
//
static void mem_write (icmProcessorP processor, Addr address, Uns32 bytes,
    const void *value, void *user_data, Addr VA)
{
    Uns32 data;
    const char *name = "???";

    if (bytes != 4) {
        icmPrintf("--- I/O Write %08x: incorrect size %u bytes\n",
            (Uns32) address, bytes);
        icmExit(processor);
    }
    data = *(Uns32*) value;
    io_write32 (address, (Uns32*) (user_data + (address & 0xfffc)),
        data, &name);
    if (trace && name != 0) {
        icmPrintf("--- I/O Write %08x to %s \n", data, name);
    }
}

void quit()
{
    icmPrintf("***** Stop *****\n");
    icmTerminate();
}

//
// Main simulation routine
//
int main(int argc, char ** argv)
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

    for (;;) {
        switch (getopt (argc, argv, "vtd:")) {
        case EOF:
            break;
        case 'v':
            sim_attrs |= ICM_VERBOSE;
            continue;
        case 't':
            trace++;
            continue;
        default:
            usage ();
        }
        break;
    }
    argc -= optind;
    argv += optind;

    const char *app_file, *cpu_type = "M4K";
    if (argc == 2) {
        cpu_type = argv[1];
    } else if (argc != 1) {
        if (argc > 0)
            icmPrintf("%s: Wrong number of args (%d)\n", progname, argc);
        usage ();
    }
    app_file = argv[0];

    if (cpu_type != 0)
        icmPrintf("Processor Variant: %s\n", cpu_type);
    icmPrintf("Trace mode: %s\n", trace ? "On" : "Off");

    //
    // Initialize CpuManager
    //
    icmInit(sim_attrs, NULL, 0);
    atexit(quit);

    //
    // Setup the configuration attributes for the MIPS model
    //
    icmAttrListP user_attrs = icmNewAttrList();

    // PIC32 is always little endian
    icmAddStringAttr(user_attrs, "endian", "Little");

    // Select the processor configuration
    if (cpu_type != 0)
        icmAddStringAttr(user_attrs, "variant", cpu_type);

    // Enable vectored interrupts
    icmAddStringAttr(user_attrs, "vectoredinterrupt", "enable");

    if (trace >= 2) {
        // Enable MIPS-format trace
        icmAddStringAttr(user_attrs, "MIPS_TRACE", "enable");
        icm_attrs |= ICM_ATTR_TRACE |
            ICM_ATTR_TRACE_REGS_BEFORE | ICM_ATTR_TRACE_REGS_AFTER;

        // Trace Count/Compare, TLB and FPU
        model_flags |= 0x0c000020;
    }

    // Select processor model from library
    const char *model_file = icmGetVlnvString(NULL, "mips.ovpworld.org", "processor", "mips32", "1.0", "model");

    //
    // create a processor
    //
    icmProcessorP processor = icmNewProcessor(
        cpu_type,                     // processor name
        "mips",                       // processor type
        1,                            // processor cpuId
        model_flags,                  // processor model flags
        32,                           // physical address bits
        model_file,                   // model file
        "modelAttrs",                 // morpher attributes
        icm_attrs,                    // processor attributes
        user_attrs,                   // user attribute list
        0,                            // semihosting file name
        0                             // semihosting attribute symbol
    );
    icmBusP bus = icmNewBus("bus", 32);
    icmConnectProcessorBusses(processor, bus, bus);

    // Data memory.
    icmMemoryP datamem = icmNewMemory("SRAM", ICM_PRIV_RWX, DATA_MEM_SIZE - 1);
    icmConnectMemoryToBus(bus, "slave1", datamem, DATA_MEM_START);

    // Program memory.
    icmMapNativeMemory (bus, ICM_PRIV_RX, PROGRAM_MEM_START,
        PROGRAM_MEM_START + PROGRAM_MEM_SIZE - 1, progmem);

    // Boot memory.
    icmMapNativeMemory (bus, ICM_PRIV_RX, BOOT_MEM_START,
        BOOT_MEM_START + BOOT_MEM_SIZE - 1, bootmem);

    // Preset DEVCFG data, from Max32 bootloader.
    BOOTMEM(DEVCFG3) = 0xffff0722;
    BOOTMEM(DEVCFG2) = 0xd979f8f9;
    BOOTMEM(DEVCFG1) = 0x5bfd6aff;
    BOOTMEM(DEVCFG0) = 0xffffff7f;

    // I/O memory.
    icmMapExternalMemory(bus, "IO", ICM_PRIV_RW, IO_MEM_START,
        IO_MEM_START + IO_MEM_SIZE - 1, mem_read, mem_write, iomem);
    icmMapExternalMemory(bus, "IO2", ICM_PRIV_RW, IO2_MEM_START,
        IO2_MEM_START + IO2_MEM_SIZE - 1, mem_read, mem_write, iomem2);

    if (sim_attrs & ICM_VERBOSE) {
        // Print all user attributes.
        icmPrintf("\n***** User attributes *****\n");
        icmIterAllUserAttributes(print_user_attribute, 0);

        // Show Mapping on bus
        icmPrintf("\n***** Configuration of memory bus *****\n");
        icmPrintBusConnections(bus);
    }

    //
    // Load Program
    //
    if (! load_file(progmem, bootmem, app_file)) {
        icmPrintf("Failed for '%s'", app_file);
        return -1;
    }
    io_init (iomem, iomem2);

    //
    // Do a simulation run
    //
    icmSetPC(processor, 0xbfc00000);
    icmPrintf("\n***** Start '%s' *****\n", cpu_type);
#if 1
    // Simulate the platform until done
    icmSimulatePlatform();
#else
    // Run the processor one instruction at a time until finished
    icmStopReason stop_reason;
    do {
        // simulate one instruction
        stop_reason = icmSimulate(processor, 1);
    } while (stop_reason == ICM_SR_SCHED);
#endif

    //
    // quit() implicitly called on return
    //
    return 0;
}
