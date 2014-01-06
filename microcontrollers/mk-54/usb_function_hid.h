/*
 * USB HID Function Driver File
 *
 * This file contains all of functions, macros, definitions, variables,
 * datatypes, etc. that are required for usage with the HID function
 * driver. This file should be included in projects that use the HID
 * \function driver.  This file should also be included into the
 * usb_descriptors.c file and any other user file that requires access to the
 * HID interface.
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 */
#ifndef HID_H
#define HID_H

/*
 * Default HID configuration.
 */
#ifndef HID_EP
#   define HID_EP		1
#endif
#ifndef HID_INTF_ID
#   define HID_INTF_ID		0x00
#endif
#ifndef HID_BD_OUT
#   define HID_BD_OUT		USB_EP_1_OUT
#endif
#ifndef HID_INT_OUT_EP_SIZE
#   define HID_INT_OUT_EP_SIZE	3
#endif
#ifndef HID_BD_IN
#   define HID_BD_IN		USB_EP_1_IN
#endif
#ifndef HID_INT_IN_EP_SIZE
#   define HID_INT_IN_EP_SIZE	3
#endif
#ifndef HID_NUM_OF_DSC
#   define HID_NUM_OF_DSC	1
#endif
#ifndef HID_RPT01_SIZE
#   define HID_RPT01_SIZE	47
#endif

/* Class-Specific Requests */
#define GET_REPORT		0x01
#define GET_IDLE		0x02
#define GET_PROTOCOL		0x03
#define SET_REPORT		0x09
#define SET_IDLE		0x0A
#define SET_PROTOCOL		0x0B

/* Class Descriptor Types */
#define DSC_HID			0x21
#define DSC_RPT			0x22
#define DSC_PHY			0x23

/* Protocol Selection */
#define BOOT_PROTOCOL		0x00
#define RPT_PROTOCOL		0x01

/* HID Interface Class Code */
#define HID_INTF		0x03

/* HID Interface Class SubClass Codes */
#define BOOT_INTF_SUBCLASS	0x01

/* HID Interface Class Protocol Codes */
#define HID_PROTOCOL_NONE	0x00
#define HID_PROTOCOL_KEYBOARD	0x01
#define HID_PROTOCOL_MOUSE	0x02

#if !defined(USBDEVICE_C)
    extern const unsigned char hid_rpt01 [HID_RPT01_SIZE];
    extern volatile unsigned char hid_report_out[HID_INT_OUT_EP_SIZE];
    extern volatile unsigned char hid_report_in[HID_INT_IN_EP_SIZE];
    extern volatile unsigned char hid_report_feature[HID_FEATURE_REPORT_BYTES];
#endif

/*
    Macro:
        unsigned char mHIDGetRptRxLength(void)

    Description:
        mHIDGetRptRxLength is used to retrieve the number
        of bytes copied to user's buffer by the most
        recent call to HIDRxReport function.

    Precondition:
        None

    Parameters:
        None

    Return Values:
        unsigned char : mHIDGetRptRxLength returns hid_rpt_rx_len

    Remarks:
        None
 */
#define mHIDGetRptRxLength()        hid_rpt_rx_len

/*
    Function:
        BOOL HIDTxHandleBusy(USB_HANDLE handle)

    Summary:
        Retreives the status of the buffer ownership

    Description:
        Retreives the status of the buffer ownership.  This function will
        indicate if the previous transfer is complete or not.

        This function will take the input handle (pointer to a BDT entry) and
        will check the UOWN bit.  If the UOWN bit is set then that indicates
        that the transfer is not complete and the USB module still owns the data
        memory.  If the UOWN bit is clear that means that the transfer is
        complete and that the CPU now owns the data memory.

        For more information about the BDT, please refer to the appropriate
        datasheet for the device in use.

        Typical Usage:
        <code>
        //make sure that the last transfer isn't busy by checking the handle
        if(!HIDTxHandleBusy(USBInHandle))
        {
            //Send the data contained in the ToSendDataBuffer[] array out on
            //  endpoint HID_EP
            USBInHandle = HIDTxPacket(HID_EP,(unsigned char*)&ToSendDataBuffer[0],sizeof(ToSendDataBuffer));
        }
        </code>

    PreCondition:
        None.

    Parameters:
        USB_HANDLE handle - the handle for the transfer in question.
        The handle is returned by the HIDTxPacket() and HIDRxPacket()
        functions.  Please insure that USB_HANDLE objects are initialized
        to NULL.

    Return Values:
        TRUE - the HID handle is still busy
        FALSE - the HID handle is not busy and is ready to send
                additional data.

   Remarks:
        None
 */
#define HIDTxHandleBusy(handle) USBHandleBusy(handle)

/*
    Function:
        BOOL HIDRxHandleBusy(USB_HANDLE handle)

    Summary:
        Retreives the status of the buffer ownership

    Description:
        Retreives the status of the buffer ownership.  This function will
        indicate if the previous transfer is complete or not.

        This function will take the input handle (pointer to a BDT entry) and
        will check the UOWN bit.  If the UOWN bit is set then that indicates
        that the transfer is not complete and the USB module still owns the data
        memory.  If the UOWN bit is clear that means that the transfer is
        complete and that the CPU now owns the data memory.

        For more information about the BDT, please refer to the appropriate
        datasheet for the device in use.

        Typical Usage:
        <code>
        if(!HIDRxHandleBusy(USBOutHandle))
        {
            //The data is available in the buffer that was specified when the
            //  HIDRxPacket() was called.
        }
        </code>

    PreCondition:
        None

    Parameters:
        None

    Return Values:
        TRUE - the HID handle is still busy
        FALSE - the HID handle is not busy and is ready to receive
                additional data.

   Remarks:
        None
 */
#define HIDRxHandleBusy(handle) USBHandleBusy(handle)

/*
    Function:
        USB_HANDLE HIDTxPacket(unsigned char ep, unsigned char* data, uint16_t len)

    Summary:
        Sends the specified data out the specified endpoint

    Description:
        This function sends the specified data out the specified
        endpoint and returns a handle to the transfer information.

        Typical Usage:
        <code>
        //make sure that the last transfer isn't busy by checking the handle
        if(!HIDTxHandleBusy(USBInHandle))
        {
            //Send the data contained in the ToSendDataBuffer[] array out on
            //  endpoint HID_EP
            USBInHandle = HIDTxPacket(HID_EP,(unsigned char*)&ToSendDataBuffer[0],sizeof(ToSendDataBuffer));
        }
        </code>

    PreCondition:
        None

    Parameters:
        ep - the endpoint you want to send the data out of
        data - pointer to the data that you wish to send
        len - the length of the data that you wish to send

    Return Values:
        USB_HANDLE - a handle for the transfer.  This information
        should be kept to track the status of the transfer

    Remarks:
        None
 */
#define HIDTxPacket USBTxOnePacket

/*
    Function:
        USB_HANDLE HIDRxPacket(unsigned char ep, unsigned char* data, uint16_t len)

    Summary:
        Receives the specified data out the specified endpoint

    Description:
        Receives the specified data out the specified endpoint.

        Typical Usage:
        <code>
        //Read 64-bytes from endpoint HID_EP, into the ReceivedDataBuffer array.
        //  Make sure to save the return handle so that we can check it later
        //  to determine when the transfer is complete.
        USBOutHandle = HIDRxPacket(HID_EP,(unsigned char*)&ReceivedDataBuffer,64);
        </code>

    PreCondition:
        None

    Parameters:
        ep - the endpoint you want to receive the data into
        data - pointer to where the data will go when it arrives
        len - the length of the data that you wish to receive

    Return Values:
        USB_HANDLE - a handle for the transfer.  This information
        should be kept to track the status of the transfer

    Remarks:
        None
 */
#define HIDRxPacket USBRxOnePacket

//
// Section: STRUCTURES
//

//USB HID Descriptor header as detailed in section
//"6.2.1 HID Descriptor" of the HID class definition specification
typedef struct _USB_HID_DSC_HEADER
{
	unsigned char bDescriptorType;	// offset 9
	uint16_t wDscLength;		// offset 10

} USB_HID_DSC_HEADER;

//USB HID Descriptor header as detailed in section
//"6.2.1 HID Descriptor" of the HID class definition specification
typedef struct _USB_HID_DSC
{
	unsigned char bLength;		// offset 0
	unsigned char bDescriptorType;	// offset 1
	uint16_t bcdHID;		// offset 2
	unsigned char bCountryCode;	// offset 4
	unsigned char bNumDsc;		// offset 5

	//USB_HID_DSC_HEADER hid_dsc_header[HID_NUM_OF_DSC];
	/* HID_NUM_OF_DSC is defined in usbcfg.h */

} USB_HID_DSC;

extern unsigned char hid_rpt_rx_len;

void USBCheckHIDRequest(void);

#endif //HID_H
