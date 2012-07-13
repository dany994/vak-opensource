
EJTAGproxy is a utility for debugging PIC32 microcontrollers with GDB
via JTAG or ICSP adapter.  Supported adapters:

 * Microchip PICkit2
 * Microchip PICkit3 with scripting firmware
 * Olimex ARM-USB-Tiny

Usage:
    ejtagproxy [options]

Options:
    -d, --daemon        run as daemon
    -p, --port=PORT     use the specified TCP port (default 2000)
    -D, --debug         output debug messages

To start a debug session:
1) Run ejtagproxy.
2) Attach a USB adapter to target board.
3) Run gdb and connect to the PIC32 target:
    set remote hardware-breakpoint-limit 6
    set remote hardware-watchpoint-limit 2
    target remote localhost:2000

When gdb session is closed, ejtagproxy disconnects from the target board.
You can use other tools, like pic32prog, to update the target software
and then start a new gdb session.  No need to restart ejtagproxy between
session.  You can safely run it as a daemon.

To build on Linux or Mac OS X, run:
    make
    make install

To build on Windows using MINGW compiler, run:
    gmake -f make-mingw

Based on sources of :
 * GDBproxy project by Steve Underwood: http://gdbproxy.sourceforge.net/
 * PIC32prog project by Serge Vakulenko: http://code.google.com/p/pic32prog/
