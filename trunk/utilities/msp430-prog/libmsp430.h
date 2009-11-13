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

#if 0
int MSP430_Configure (int, int);
int MSP430_Erase (int, int, int);
int MSP430_Error_Number (void);
int MSP430_FET_FwUpdate (string, IntPtr, int);
int MSP430_FET_GetFwVersion (Int32&);
int MSP430_FET_Reset (void);
int MSP430_FET_SelfTest (int, Byte[]);
int MSP430_GetExtVoltage (Int32&, Int32&);
int MSP430_GetNameOfUsbIf (int, String&, Int32&);
int MSP430_GetNumberOfUsbIfs (Int32&);
int MSP430_Identify (Byte[], int, int);
int MSP430_Memory (int, Byte[], int, int);
int MSP430_ProgramFile (string, int, int);
int MSP430_Reset (int, int, int);
int MSP430_VCC (int);
string MSP430_Error_String (int);
#endif
