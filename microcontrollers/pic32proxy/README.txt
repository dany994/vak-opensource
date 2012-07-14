
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
sessions.  You can safely run it as a daemon.

To build on Linux or Mac OS X, run:
    make
    make install

To build on Windows using MINGW compiler, run:
    gmake -f make-mingw

Based on sources of:
 * GDBproxy project by Steve Underwood: http://gdbproxy.sourceforge.net/
 * PIC32prog project by Serge Vakulenko: http://code.google.com/p/pic32prog/

List of supported processors:
* PIC32MX1xx/2xx family:
    pic32mx110f016b, pic32mx110f016c, pic32mx110f016d, pic32mx120f032b,
    pic32mx120f032c, pic32mx120f032d, pic32mx130f064b, pic32mx130f064c,
    pic32mx130f064d, pic32mx150f128b, pic32mx150f128c, pic32mx150f128d,
    pic32mx210f016b, pic32mx210f016c, pic32mx210f016d, pic32mx220f032b,
    pic32mx220f032c, pic32mx220f032d, pic32mx230f064b, pic32mx230f064c,
    pic32mx230f064d, pic32mx250f128b, pic32mx250f128c, pic32mx250f128d.
 * PIC32MX3xx/4xx family:
    pic32mx320f032h, pic32mx320f064h, pic32mx320f128h, pic32mx320f128l,
    pic32mx340f128h, pic32mx340f128l, pic32mx340f256h, pic32mx340f512h,
    pic32mx360f256l, pic32mx360f512l, pic32mx420f032h, pic32mx440f128h,
    pic32mx440f128l, pic32mx440f256h, pic32mx440f512h, pic32mx460f256l,
    pic32mx460f512l.
* PIC32MX5xx/6xx/7xx family:
    pic32mx534f064h, pic32mx534f064l, pic32mx564f064h, pic32mx564f064l,
    pic32mx564f128h, pic32mx564f128l, pic32mx575f256h, pic32mx575f256l,
    pic32mx575f512h, pic32mx575f512l, pic32mx664f064h, pic32mx664f064l,
    pic32mx664f128h, pic32mx664f128l, pic32mx675f256h, pic32mx675f256l,
    pic32mx675f512h, pic32mx675f512l, pic32mx695f512h, pic32mx695f512l,
    pic32mx764f128h, pic32mx764f128l, pic32mx775f256h, pic32mx775f256l,
    pic32mx775f512h, pic32mx775f512l, pic32mx795f512h, pic32mx795f512l.
