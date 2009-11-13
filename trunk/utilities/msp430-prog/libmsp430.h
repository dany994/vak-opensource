/*
 * Library: MSP430.dll
 */

/*
 * Initialize the interface.
 * Parameters:
 * port    - interface port reference (application specific).
 * version - the version number of the MSP430 DLL is returned.
 */
int (*MSP430_Initialize) (const char *port, long *version);

/*
 * Close the interface.
 * Parameters: vccOff - turn off the device Vcc (0 volts) if TRUE.
 * Returns 0 on success.
 */
int (*MSP430_Close) (int vccOff);

int (*MSP430_FET_GetFwVersion) (long *version);

int (*MSP430_Identify) (char *buf, int len, int arg);

/*
 * Configure the device interface.
 */
int (*MSP430_Configure) (int op, int arg);

#if 0
int MSP430_Erase (int, int, int);
int MSP430_Error_Number (void);
int MSP430_FET_FwUpdate (string, IntPtr, int);
int MSP430_FET_Reset (void);
int MSP430_FET_SelfTest (int, Byte[]);
int MSP430_GetExtVoltage (Int32&, Int32&);
int MSP430_GetNameOfUsbIf (int, String&, Int32&);
int MSP430_GetNumberOfUsbIfs (Int32&);
int MSP430_Memory (int, Byte[], int, int);
int MSP430_ProgramFile (string, int, int);
int MSP430_Reset (int, int, int);
int MSP430_VCC (int);
string MSP430_Error_String (int);
#endif

/*
 * Configurations of the MSP430 driver
 */
#define CONFIGURE_VERIFICATION_MODE	0 /* Verify data downloaded to FLASH memories */
#define CONFIGURE_EMULATION_MODE	1
#define CONFIGURE_CLK_CNTRL_MODE	2
#define CONFIGURE_MCLK_CNTRL_MODE	3
#define CONFIGURE_FLASH_TEST_MODE	4
#define CONFIGURE_LOCKED_FLASH_ACCESS	5 /* Allows Locked Info Mem Segment A access (if set to '1') */
#define CONFIGURE_FLASH_SWOP		6
#define CONFIGURE_EDT_TRACE_MODE	7
#define CONFIGURE_INTERFACE_MODE	8 /* see INTERFACE_TYPE below */
#define CONFIGURE_SET_MDB_BEFORE_RUN	9
#define CONFIGURE_RAM_PRESERVE_MODE	10 /* Configure whether RAM content should be preserved/restored */

/* Interface type */
#define INTERFACE_JTAG_IF		0
#define INTERFACE_SPYBIWIRE_IF		1
#define INTERFACE_SPYBIWIREJTAG_IF	2
