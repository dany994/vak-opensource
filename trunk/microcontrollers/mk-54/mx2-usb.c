/*
 * The software supplied herewith by Microchip Technology Incorporated
 * (the `Company') for its PIC Microcontroller is intended and
 * supplied to you, the Company's customer, for use solely and
 * exclusively on Microchip PIC Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN `AS IS' CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 */
#include <stdint.h>
#include "calc.h"
#include "pic32mx.h"
#include "usb-config.h"
#include "usb.h"
#include "usb-function-hid.h"

/*
 * Pin assignment for pic32mx250 processor in DIP28 package.
 *
 *                  Bottom view
 *                  ------------
 *                  | 28     1 |
 *                  | 27     2 | RA0 - clk
 *       dot - RB15 | 26     3 | RA1 - keypad B
 * segment G - RB14 | 25     4 | RB0 - segment A, data
 * segment F - RB13 | 24     5 | RB1 - segment B
 *                  | 23     6 | RB2 - keypad A
 *                  | 22     7 | RB3 - segment C
 *                  | 21     8 |
 *                  | 20     9 |
 *                  | 19    10 |
 *   keypad E - RB9 | 18    11 | RB4 - segment E
 *   keypad D - RB8 | 17    12 | RA4 - segment D
 *   keypad C - RB7 | 16    13 |
 *                  | 15    14 |
 *                  ------------
 */
#define PIN(n)  (1 << (n))

/*
 * Chip configuration.
 */
PIC32_DEVCFG (
    DEVCFG0_DEBUG_DISABLED |    /* ICE debugger disabled */
    DEVCFG0_JTAGDIS,            /* Disable JTAG port */

    DEVCFG1_FNOSC_PRIPLL |      /* Primary oscillator with PLL */
    DEVCFG1_POSCMOD_HS |        /* HS oscillator */
    DEVCFG1_OSCIOFNC_OFF |      /* CLKO output disabled */
    DEVCFG1_FPBDIV_4 |          /* Peripheral bus clock = SYSCLK/4 */
    DEVCFG1_FCKM_DISABLE,       /* Fail-safe clock monitor disable */

    DEVCFG2_FPLLIDIV_3 |        /* PLL divider = 1/3 */
    DEVCFG2_FPLLMUL_24 |        /* PLL multiplier = 24x */
    DEVCFG2_UPLLIDIV_3 |        /* USB PLL divider = 1/3 */
    DEVCFG2_FPLLODIV_2,         /* PLL postscaler = 1/2 */

    DEVCFG3_USERID(0xffff));    /* User-defined ID */

/*
 * Boot code at bfc00000.
 * Setup stack pointer and $gp registers, and jump to main().
 */
asm ("          .section .exception,\"ax\",@progbits");
asm ("          .globl _start");
asm ("          .type _start, function");
asm ("_start:   la      $sp, _estack");
asm ("          la      $ra, main");
asm ("          la      $gp, _gp");
asm ("          jr      $ra");
asm ("          .text");

/*
 * Process the received data (hid_report_out) and
 * prepare the data for transmit (hid_report_in).
 */
static void process_data()
{
    unsigned int count;

    // TODO
    for (count=0; count<HID_OUTPUT_REPORT_BYTES; count++) {
        hid_report_in[count] = hid_report_out[count];
    }
}

/*
 * This routine will poll for a received Input report, process it
 * and send an Output report to the host.
 * Both directions use interrupt transfers.
 * The ownership of the USB buffers will change according
 * to the required operation.
 */
static void send_receive()
{
    static int usb_state = 'r';
    static USB_HANDLE last_transmit = 0;
    static USB_HANDLE last_receive = 0;

    switch (usb_state) {
    case 'r':
        if (! HIDRxHandleBusy (last_receive)) {
            // The CPU owns the endpoint. Start receiving data.
            last_receive = HIDRxPacket (HID_EP,
                (unsigned char*) &hid_report_out,
                HID_INT_OUT_EP_SIZE);
            usb_state = 'p';
        }
        break;
    case 'p':
        if (! HIDRxHandleBusy (last_receive)) {
            // The CPU owns the endpoint.
            if (last_receive->CNT > 0) {
                // Data was received. Copy it to the output buffer for sending.
                process_data();

                // Ready to transmit the received data back to the host.
                usb_state = 't';
            } else {
                // No data was received. Return to checking for new received data.
                usb_state = 'r';
            }
        }
        break;
    case 't':
        if (! HIDTxHandleBusy (last_transmit)) {
            // The CPU owns the endpoint. Start sending data.
            last_transmit = HIDTxPacket (HID_EP,
                (unsigned char*) &hid_report_in,
                HID_INPUT_REPORT_BYTES);

            // Return to checking for new received data.
            usb_state = 'r';
        }
        break;
    default:
        // Cannot happen.
        break;
    }
}

/*
 * Check USB status, receive and send data.
 */
static void poll_usb()
{
    // Check bus status and service USB interrupts.
    USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
                      // this function periodically.  This function will take care
                      // of processing and responding to SETUP transactions
                      // (such as during the enumeration process when you first
                      // plug in).  USB hosts require that USB devices should accept
                      // and process SETUP packets in a timely fashion.  Therefore,
                      // when using polling, this function should be called
                      // frequently (such as once about every 100 microseconds) at any
                      // time that a SETUP packet might reasonably be expected to
                      // be sent by the host to your device.  In most cases, the
                      // USBDeviceTasks() function does not take very long to
                      // execute (~50 instruction cycles) before it returns.

    // When USB link established - receive and send data
    if (USBDeviceState >= CONFIGURED_STATE &&
        ! (U1PWRC & PIC32_U1PWRC_USUSPEND))
    {
        send_receive();
    }
}

/*
 * Display a symbol on 7-segment LED
 */
void set_segments (unsigned digit, unsigned dot, int upper_flag)
{
    static const unsigned segments[16] = {
    //- A - B - C - D - E -- F -- G --
        1 + 2 + 4 + 8 + 16 + 32,        // digit 0
            2 + 4,                      // digit 1
        1 + 2     + 8 + 16      + 64,   // digit 2
        1 + 2 + 4 + 8           + 64,   // digit 3
            2 + 4          + 32 + 64,   // digit 4
        1     + 4 + 8      + 32 + 64,   // digit 5
        1     + 4 + 8 + 16 + 32 + 64,   // digit 6
        1 + 2 + 4,                      // digit 7
        1 + 2 + 4 + 8 + 16 + 32 + 64,   // digit 8
        1 + 2 + 4 + 8      + 32 + 64,   // digit 9
                                  64,   // symbol -
                    8 + 16 + 32,        // symbol L
        1 +         8 + 16 + 32,        // symbol C
        1 +             16 + 32,        // symbol Г
        1 +         8 + 16 + 32 + 64,   // symbol E
        0,                              // empty
    };

    switch (digit) {
    case '-': digit = 10; break;
    case 'L': digit = 11; break;
    case 'C': digit = 12; break;
    case 'R': digit = 13; break;
    case 'E': digit = 14; break;
    case ' ': digit = 15; break;
    }
    unsigned mask = segments [digit & 15];
    if (upper_flag)
        mask &= ~(4 + 8 + 16);

    if (mask & 1)   TRISBCLR = PIN(0);  // segment A - signal RB0
    if (mask & 2)   TRISBCLR = PIN(1);  // segment B - signal RB1
    if (mask & 4)   TRISBCLR = PIN(3);  // segment C - signal RB3
    if (mask & 8)   TRISACLR = PIN(4);  // segment D - signal RA4
    if (mask & 16)  TRISBCLR = PIN(4);  // segment E - signal RB4
    if (mask & 32)  TRISBCLR = PIN(13); // segment F - signal RB13
    if (mask & 64)  TRISBCLR = PIN(14); // segment G - signal RB14
    if (dot)        TRISBCLR = PIN(15); // dot       - signal RB15
}

/*
 * Clear 7-segment LED.
 */
static inline void clear_segments()
{
    // Set segment and dot pins to tristate.
    TRISBSET = PIN(0) | PIN(1) | PIN(3) | PIN(4) |
               PIN(13) | PIN(14) | PIN(15);
    TRISASET = PIN(4);
}

/*
 * Toggle clock signal.
 */
static inline void clk()
{
    LATASET = PIN(0);                   // set clock
    LATASET = PIN(0);                   // pulse 200 usec
    LATASET = PIN(0);
    LATASET = PIN(0);
    LATASET = PIN(0);
    LATASET = PIN(0);
    LATASET = PIN(0);
    LATASET = PIN(0);

    LATACLR = PIN(0);                   // clear clock
}

/*
 * Set or clear data signal: segment A - RB0.
 */
static inline void data (int on)
{
    if (on >= 0) {
        if (on)
            LATBSET = PIN(0);           // set data
        else
            LATBCLR = PIN(0);           // clear data
        TRISBCLR = PIN(0);              // activate
    } else
        TRISBSET = PIN(0);              // tristate
}

static unsigned rgd;                    // Radians/grads/degrees
static unsigned keycode;                // Code of pressed button
static unsigned key_pressed;            // Bitmask of active key

/*
 * Poll keypad: input pins RD4-RD7.
 */
int scan_keys (int row)
{
    static const int col_a [8] = {
        KEY_7,      //  7   sin
        KEY_4,      //  4   sin-1
        KEY_1,      //  1   e^x
        KEY_0,      //  0   10^x
        KEY_DOT,    //  ,   O
        KEY_NEG,    //  /-/ АВТ
        KEY_EXP,    //  ВП  ПРГ
        KEY_CLEAR,  //  Cx  CF
    };
    static const int col_b [8] = {
        KEY_K,      //  K
        KEY_LOAD,   //  ИП  L0
        KEY_8,      //  8   cos
        KEY_5,      //  5   cos-1
        KEY_2,      //  2   lg
        KEY_3,      //  3   ln
        KEY_XY,     //  xy  x^y
        KEY_ENTER,  //  B^  Bx
    };
    static const int col_c [8] = {
        KEY_F,      //  F
        KEY_NEXT,   //  ШГ> x<0
        KEY_PREV,   //  <ШГ x=0
        KEY_STORE,  //  П   L1
        KEY_9,      //  9   tg
        KEY_6,      //  6   tg-1
        KEY_ADD,    //  +   pi
        KEY_MUL,    //  *   x^2
    };
    static const int col_d [8] = {
        0,
        KEY_RET,    //  B/O x>=0
        KEY_GOTO,   //  БП  L2
        KEY_SUB,    //  -   sqrt
        KEY_DIV,    //  /   1/x
        KEY_CALL,   //  ПП  L3
        KEY_STOPGO, //  C/П x!=0
        0,
    };
    int porta = PORTA;
    int portb = PORTB;

    // Poll radians/grads/degrees switch
    if (portb & PIN(9)) {   // RB9 - keypad E
        switch (row) {
        case 0: rgd = MODE_RADIANS; break;
        case 7: rgd = MODE_DEGREES; break;
        }
    }

    if (portb & PIN(2))     // RB2 - keypad A
        return col_a[row];
    if (porta & PIN(1))     // RA1 - keypad B
        return col_b[row];
    if (portb & PIN(7))     // RB7 - keypad C
        return col_c[row];
    if (portb & PIN(8))     // RB8 - keypad D
        return col_d[row];
    return 0;
}

/*
 * Show the next display symbol.
 * Index counter is in range 0..11.
 */
void calc_display (int i, int digit, int dot)
{
    clear_segments();
    if (i >= 0) {
        if (i == 0) {
            data (1);                   // set data
            clk();                      // toggle clock
        }
        data (0);                       // clear data
        clk();                          // toggle clock
        data (-1);                      // tristate data

        if (digit >= 0)                 // display a digit
            set_segments (digit, dot, i==2);

        if (i < 8) {                    // scan keypad
            int key = scan_keys (i);
            if (key) {
                keycode = key;
                key_pressed |= (1 << i);
            } else {
                key_pressed &= ~(1 << i);
            }
        }
    }
}

/*
 * Poll the radians/grads/degrees switch.
 */
int calc_rgd()
{
    return rgd;
}

/*
 * Poll the keypad.
 */
int calc_keypad()
{
    poll_usb();

    if (! key_pressed)
        return 0;
    return keycode;
}

/*
 * Main program entry point.
 */
int main()
{
    /* Initialize coprocessor 0. */
    mtc0 (C0_COUNT, 0, 0);
    mtc0 (C0_COMPARE, 0, -1);
    mtc0 (C0_EBASE, 1, 0x9fc00000);     /* Vector base */
    mtc0 (C0_INTCTL, 1, 1 << 5);        /* Vector spacing 32 bytes */
    mtc0 (C0_CAUSE, 0, 1 << 23);        /* Set IV */
    mtc0 (C0_STATUS, 0, 0);             /* Clear BEV */

    /* Initialize .bss segment by zeroes. */
    extern unsigned _edata, _end;
    unsigned *dest = &_edata;
    while (dest < &_end)
        *dest++ = 0;

    /* Unlock CFGCON register. */
    SYSKEY = 0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    CFGCON &= (1 << 13);                // clear IOLOCK

    /* Disable JTAG ports, to make more pins available. */
    CFGCON &= (1 << 3);                 // clear JTAGEN

    PMCON = 0;

    /* Use all ports as digital. */
    ANSELA = 0;
    ANSELB = 0;
    LATA = 0;
    LATB = 0;

    /* Input pins: keypad.
     * Enable pull-down resistors. */
    TRISASET = PIN(1);          // keypad B - signal RA1
    TRISBSET = PIN(2) |         // keypad A - signal RB2
               PIN(9) |         // keypad E - signal RB9
               PIN(8) |         // keypad D - signal RB8
               PIN(7);          // keypad C - signal RB7
    CNPDASET = PIN(1);
    CNPDBSET = PIN(2);

    /* RA0 - clock. */
    TRISACLR = PIN(0);

    /* Output/tristate pins: segments A-G and dot. */
    clear_segments();

    USBDeviceInit();                    // initialize USB port

    int i;
    data (0);                           // clear data
    for (i=0; i<16; i++)                // clear register
        clk();
    data (-1);                          // tristate data

    calc_init();
    rgd = MODE_DEGREES;
    keycode = 0;
    key_pressed = 0;

    for (;;) {
        // Simulate one cycle of the calculator.
        calc_step();
    }
}

/*
 * USB Callback Functions
 *
 * The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
 * events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
 * packets to your device.  In response to this, all USB devices are supposed to decrease their power
 * consumption from the USB Vbus to <2.5mA each.  The USB module detects this condition (which according
 * to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
 * function.  You should modify these callback functions to take appropriate actions for each of these
 * conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
 * consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
 * microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
 * add code that undoes the power saving things done in the USBCBSuspend() function.
 *
 * The USBCBSendResume() function is special, in that the USB stack will not automatically call this
 * function.  This function is meant to be called from the application firmware instead.  See the
 * additional comments near the function.
 */

/*
 * This function is called when the device becomes
 * initialized, which occurs after the host sends a
 * SET_CONFIGURATION (wValue not = 0) request.  This
 * callback function should initialize the endpoints
 * for the device's usage according to the current
 * configuration.
 */
void USBCBInitEP()
{
    USBEnableEndpoint (HID_EP, USB_IN_ENABLED | USB_OUT_ENABLED |
        USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
}

/*
 * When SETUP packets arrive from the host, some
 * firmware must process the request and respond
 * appropriately to fulfill the request.  Some of
 * the SETUP packets will be for standard
 * USB "chapter 9" (as in, fulfilling chapter 9 of
 * the official USB specifications) requests, while
 * others may be specific to the USB device class
 * that is being implemented.  For example, a HID
 * class device needs to be able to respond to
 * "GET REPORT" type of requests.  This
 * is not a standard USB chapter 9 request, and
 * therefore not handled by usb_device.c.  Instead
 * this request should be handled by class specific
 * firmware, such as that contained in usb_function_hid.c.
 */
void USBCBCheckOtherReq()
{
    USBCheckHIDRequest();
}

/*
 * The USBCBStdSetDscHandler() callback function is
 * called when a SETUP, bRequest: SET_DESCRIPTOR request
 * arrives.  Typically SET_DESCRIPTOR requests are
 * not used in most applications, and it is
 * optional to support this type of request.
 */
void USBCBStdSetDscHandler()
{
    /* Must claim session ownership if supporting this request */
}

/*
 * The host may put USB peripheral devices in low power
 * suspend mode (by "sending" 3+ms of idle).  Once in suspend
 * mode, the host may wake the device back up by sending non-
 * idle state signalling.
 *
 * This call back is invoked when a wakeup from USB suspend is detected.
 */
void USBCBWakeFromSuspend()
{
    // If clock switching or other power savings measures were taken when
    // executing the USBCBSuspend() function, now would be a good time to
    // switch back to normal full power run mode conditions.  The host allows
    // a few milliseconds of wakeup time, after which the device must be
    // fully back to normal, and capable of receiving and processing USB
    // packets.  In order to do this, the USB module must receive proper
    // clocking (IE: 48MHz clock must be available to SIE for full speed USB
    // operation).
}

/*
 * Call back that is invoked when a USB suspend is detected
 */
void USBCBSuspend()
{
}

/*
 * The USB host sends out a SOF packet to full-speed
 * devices every 1 ms. This interrupt may be useful
 * for isochronous pipes. End designers should
 * implement callback routine as necessary.
 */
void USBCB_SOF_Handler()
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

/*
 * The purpose of this callback is mainly for
 * debugging during development. Check UEIR to see
 * which error causes the interrupt.
 */
void USBCBErrorHandler()
{
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.

    // Typically, user firmware does not need to do anything special
    // if a USB error occurs.  For example, if the host sends an OUT
    // packet to your device, but the packet gets corrupted (ex:
    // because of a bad connection, or the user unplugs the
    // USB cable during the transmission) this will typically set
    // one or more USB error interrupt flags.  Nothing specific
    // needs to be done however, since the SIE will automatically
    // send a "NAK" packet to the host.  In response to this, the
    // host will normally retry to send the packet again, and no
    // data loss occurs.  The system will typically recover
    // automatically, without the need for application firmware
    // intervention.

    // Nevertheless, this callback function is provided, such as
    // for debugging purposes.
}

/*
 * The USB specifications allow some types of USB
 * peripheral devices to wake up a host PC (such
 * as if it is in a low power suspend to RAM state).
 * This can be a very useful feature in some
 * USB applications, such as an Infrared remote
 * control receiver.  If a user presses the "power"
 * button on a remote control, it is nice that the
 * IR receiver can detect this signalling, and then
 * send a USB "command" to the PC to wake up.
 *
 * The USBCBSendResume() "callback" function is used
 * to send this special USB signalling which wakes
 * up the PC.  This function may be called by
 * application firmware to wake up the PC.  This
 * function should only be called when:
 *
 * 1.  The USB driver used on the host PC supports
 *     the remote wakeup capability.
 * 2.  The USB configuration descriptor indicates
 *     the device is remote wakeup capable in the
 *     bmAttributes field.
 * 3.  The USB host PC is currently sleeping,
 *     and has previously sent your device a SET
 *     FEATURE setup packet which "armed" the
 *     remote wakeup capability.
 *
 * This callback should send a RESUME signal that
 * has the period of 1-15ms.
 *
 * Note: Interrupt vs. Polling
 * -Primary clock
 * -Secondary clock ***** MAKE NOTES ABOUT THIS *******
 * > Can switch to primary first by calling USBCBWakeFromSuspend()
 *
 * The modifiable section in this routine should be changed
 * to meet the application needs. Current implementation
 * temporary blocks other functions from executing for a
 * period of 1-13 ms depending on the core frequency.
 *
 * According to USB 2.0 specification section 7.1.7.7,
 * "The remote wakeup device must hold the resume signaling
 * for at lest 1 ms but for no more than 15 ms."
 * The idea here is to use a delay counter loop, using a
 * common value that would work over a wide range of core
 * frequencies.
 * That value selected is 1800. See table below:
 * ==========================================================
 * Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 * ==========================================================
 *     48              12          1.05
 *      4              1           12.6
 * ==========================================================
 *  * These timing could be incorrect when using code
 *    optimization or extended instruction mode,
 *    or when having other interrupts enabled.
 *    Make sure to verify using the MPLAB SIM's Stopwatch
 *    and verify the actual signal on an oscilloscope.
 */
void USBCBSendResume()
{
    static unsigned delay_count;

    // Start RESUME signaling
    U1CON |= PIC32_U1CON_RESUME;

    // Set RESUME line for 1-13 ms
    delay_count = 1800U;
    do {
        delay_count--;
    } while (delay_count);

    U1CON &= ~PIC32_U1CON_RESUME;
}

/*
 * This function is called whenever a EP0 data
 * packet is received.  This gives the user (and
 * thus the various class examples a way to get
 * data that is received via the control endpoint.
 * This function needs to be used in conjunction
 * with the USBCBCheckOtherReq() function since
 * the USBCBCheckOtherReq() function is the apps
 * method for getting the initial control transfer
 * before the data arrives.
 *
 * PreCondition: ENABLE_EP0_DATA_RECEIVED_CALLBACK must be
 * defined already (in usb_config.h)
 */
#if defined(ENABLE_EP0_DATA_RECEIVED_CALLBACK)
void USBCBEP0DataReceived()
{
}
#endif

/* Device Descriptor */
const USB_DEVICE_DESCRIPTOR device_dsc = {
    0x12,                   // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0x00,                   // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usbcfg.h
    0x04d8,                 // Vendor ID: Microchip
    0x003F,                 // Product ID: HID Demo
    0x0001,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
const unsigned char configDescriptor1[] = {
    /* Configuration Descriptor */
    0x09,                           // sizeof(USB_CFG_DSC)
    USB_DESCRIPTOR_CONFIGURATION,   // CONFIGURATION descriptor type
    0x29, 0x00,                     // Total length of data for this cfg
    1,                              // Number of interfaces in this cfg
    1,                              // Index value of this configuration
    0,                              // Configuration string index
    _DEFAULT | _SELF,               // Attributes, see usbd.h
    50,                             // Max power consumption (2X mA)

    /* Interface Descriptor */
    0x09,                           // sizeof(USB_INTF_DSC)
    USB_DESCRIPTOR_INTERFACE,       // INTERFACE descriptor type
    0,                              // Interface Number
    0,                              // Alternate Setting Number
    2,                              // Number of endpoints in this intf
    HID_INTF,                       // Class code
    0,                              // Subclass code
    0,                              // Protocol code
    0,                              // Interface string index

    /* HID Class-Specific Descriptor */
    0x09,                           // sizeof(USB_HID_DSC)+3
    DSC_HID,                        // HID descriptor type
    0x11, 0x01,                     // HID Spec Release Number in BCD format (1.11)
    0x00,                           // Country Code (0x00 for Not supported)
    HID_NUM_OF_DSC,                 // Number of class descriptors, see usbcfg.h
    DSC_RPT,                        // Report descriptor type
    47, 0x00,                       // sizeof(hid_rpt01)

    /* Endpoint Descriptor */
    0x07,                           // sizeof(USB_EP_DSC)
    USB_DESCRIPTOR_ENDPOINT,        // Endpoint Descriptor
    HID_EP | _EP_IN,                // EndpointAddress
    _INTERRUPT,                     // Attributes
    0x08, 0x00,                     // size
    0x01,                           // Interval

    /* Endpoint Descriptor */
    0x07,                           // sizeof(USB_EP_DSC)
    USB_DESCRIPTOR_ENDPOINT,        // Endpoint Descriptor
    HID_EP | _EP_OUT,               // EndpointAddress
    _INTERRUPT,                     // Attributes
    0x08, 0x00,                     // size
    0x01                            // Interval
};

// Language code string descriptor
const struct {
    unsigned char bLength;
    unsigned char bDscType;
    uint16_t string [1];
} sd000 = {
    sizeof(sd000),
    USB_DESCRIPTOR_STRING,
    { 0x0409 }
};

// Manufacturer string descriptor
const struct {
    unsigned char bLength;
    unsigned char bDscType;
    uint16_t string [25];
} sd001 = {
    sizeof(sd001),
    USB_DESCRIPTOR_STRING,
{       'M','i','c','r','o','c','h','i','p',' ',
    'T','e','c','h','n','o','l','o','g','y',' ','I','n','c','.'
}};

// Product string descriptor
const struct {
    unsigned char bLength;
    unsigned char bDscType;
    uint16_t string [11];
} sd002 = {
    sizeof(sd002),
    USB_DESCRIPTOR_STRING,
{       'G','e','n','e','r','i','c',' ','H','I','D'
}};

// Class specific descriptor - Generic HID

// To change the number of bytes in a report, under Input report, Output report,
// or Feature report below, change Report Count from 0x02 bytes to a value
// from 0x01 to 0x08.
// In usb_config.h, edit these values to match the new report size(s):
// #define HID_INPUT_REPORT_BYTES   2
// #define HID_OUTPUT_REPORT_BYTES  2
// #define HID_FEATURE_REPORT_BYTES 2

// This firmware version doesn't support reports > 8 bytes.

const unsigned char hid_rpt01 [HID_RPT01_SIZE] = {
    0x06, 0xA0, 0xFF,       // Usage page (vendor defined)
    0x09, 0x01,             // Usage ID (vendor defined)
    0xA1, 0x01,             // Collection (application)

    // The Input report
    0x09, 0x03,             // Usage ID - vendor defined
    0x15, 0x00,             // Logical Minimum (0)
    0x26, 0xFF, 0x00,       // Logical Maximum (255)
    0x75, 0x08,             // Report Size (8 bits)
    0x95, 0x02,             // Report Count (2 fields)
    0x81, 0x02,             // Input (Data, Variable, Absolute)

    // The Output report
    0x09, 0x04,             // Usage ID - vendor defined
    0x15, 0x00,             // Logical Minimum (0)
    0x26, 0xFF, 0x00,       // Logical Maximum (255)
    0x75, 0x08,             // Report Size (8 bits)
    0x95, 0x02,             // Report Count (2 fields)
    0x91, 0x02,             // Output (Data, Variable, Absolute)

    // The Feature report
    0x09, 0x05,             // Usage ID - vendor defined
    0x15, 0x00,             // Logical Minimum (0)
    0x26, 0xFF, 0x00,       // Logical Maximum (255)
    0x75, 0x08,             // Report Size (8 bits)
    0x95, 0x02,             // Report Count (2 fields)
    0xB1, 0x02,             // Feature (Data, Variable, Absolute)

    0xC0
};

//Array of congiruation descriptors
const unsigned char *const USB_CD_Ptr[] = {
    (const unsigned char *const) &configDescriptor1
};

//Array of string descriptors
const unsigned char *const USB_SD_Ptr[] = {
    (const unsigned char *const) &sd000,
    (const unsigned char *const) &sd001,
    (const unsigned char *const) &sd002
};
