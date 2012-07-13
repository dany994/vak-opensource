
MIPSproxy is a utility for debugging PIC32 microcontrollers with GDB
via JTAG or ICSP adapter.  Supported adapters:

 * Microchip PICkit2
 * Microchip PICkit3 with scripting firmware
 * Olimex ARM-USB-Tiny

To build on Linux or Mac OS X, run:
    make
    make install

To build on Windows using MINGW compiler, run:
    gmake -f make-mingw

Based on sources of :
 * GDBproxy project by Steve Underwood: http://gdbproxy.sourceforge.net/
 * PIC32prog project by Serge Vakulenko: http://code.google.com/p/pic32prog/
