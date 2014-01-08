/*
 * USB HID Function Driver.
 *
 * This file contains all of functions, macros, definitions, variables,
 * datatypes, etc. that are required for usage with the HID function
 * driver. This file should be included in projects that use the HID
 * function driver.
 *
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
#include "usb-config.h"
#include "usb-device.h"
#include "usb-function-hid.h"

unsigned char idle_rate;
unsigned char active_protocol;   // [0] Boot Protocol [1] Report Protocol
unsigned char hid_rpt_rx_len;

void HIDGetReportHandler (void);
void HIDSetReportHandler (void);

/*
 * Section C: non-EP0 Buffer Space
 */
volatile unsigned char hid_report_out[HID_INT_OUT_EP_SIZE];
volatile unsigned char hid_report_in[HID_INT_IN_EP_SIZE];

volatile unsigned char hid_report_feature[HID_FEATURE_REPORT_BYTES];

/*
 * This routine handles HID specific request that happen on EP0.  These
 * include, but are not limited to, requests for the HID report
 * descriptors.  This function should be called from the
 * USBCBCheckOtherReq() call back function whenever using an HID device.
 *
 * Typical Usage:
 * <code>
 * void USBCBCheckOtherReq(void)
 * {
 *     //Since the stack didn't handle the request I need to check
 *     //  my class drivers to see if it is for them
 *     USBCheckHIDRequest();
 * }
 * </code>
 */
void USBCheckHIDRequest (void)
{
	if (SetupPkt.Recipient != RCPT_INTF)
		return;
	if (SetupPkt.bIntfID != HID_INTF_ID)
		return;

	/*
	 * There are two standard requests that hid.c may support.
	 * 1. GET_DSC(DSC_HID,DSC_RPT,DSC_PHY);
	 * 2. SET_DSC(DSC_HID,DSC_RPT,DSC_PHY);
	 */
	if (SetupPkt.bRequest == GET_DSC) {
		switch (SetupPkt.bDescriptorType) {
		case DSC_HID:
			if (USBActiveConfiguration == 1)	{
				USBEP0SendROMPtr ((const unsigned char*)
					&configDescriptor1 + 18,
					sizeof(USB_HID_DSC)+3,
					USB_EP0_INCLUDE_ZERO);
			}
			break;
		case DSC_RPT:
			if (USBActiveConfiguration == 1)	{
				USBEP0SendROMPtr ((const unsigned char*)
					&hid_rpt01[0],
					HID_RPT01_SIZE,     // See target.cfg
					USB_EP0_INCLUDE_ZERO);
			}
			break;
		case DSC_PHY:
			USBEP0Transmit (USB_EP0_NO_DATA);
			break;
		}
	}

	if (SetupPkt.RequestType != CLASS)
		return;

	switch (SetupPkt.bRequest) {
        case GET_REPORT:
		HIDGetReportHandler();
		break;
        case SET_REPORT:
		HIDSetReportHandler();
		break;
        case GET_IDLE:
		USBEP0SendRAMPtr ((unsigned char*)&idle_rate,
			1, USB_EP0_INCLUDE_ZERO);
		break;
        case SET_IDLE:
		USBEP0Transmit (USB_EP0_NO_DATA);
		idle_rate = SetupPkt.W_Value >> 8;
		break;
        case GET_PROTOCOL:
		USBEP0SendRAMPtr ((unsigned char*)&active_protocol,
			1, USB_EP0_NO_OPTIONS);
            break;
        case SET_PROTOCOL:
		USBEP0Transmit (USB_EP0_NO_DATA);
		active_protocol = SetupPkt.W_Value & 0xff;
		break;
    }
}

/*
 * Check to see if the HID supports a specific Output or Feature report.
 *
 * Return: 1 if it's a supported Input report
 *	   2 if it's a supported Output report
 *         3 if it's a supported Feature report
 *         0 for all other cases
 */
static unsigned char ReportSupported (void)
{
	// Find out if an Output or Feature report has arrived on the control pipe.

	USBDeviceTasks();
	switch (SetupPkt.W_Value >> 8) {
	case 0x01: 			// Input report
    		switch (SetupPkt.W_Value & 0xff) {
		case 0x00:		// Report ID 0
			return 1;
		default:
			return 0;	// Other report IDs not supported.
		}
	case 0x02: 			// Output report
    		switch (SetupPkt.W_Value & 0xff) {
		case 0x00:		// Report ID 0
			return 2;
		default:
			return 0;	// Other report IDs not supported.
		}
	case 0x03:			// Feature report
    		switch (SetupPkt.W_Value & 0xff) {
		case 0x00:		// Report ID 0
			return 3;
		default:
			return 0;	// Other report IDs not supported.
		}
	default:
		return 0;
	}
}

void HIDGetReportHandler (void)
{
	if (ReportSupported() == 1) {
		// Input Report
		inPipes[0].pSrc.bRam = (unsigned char*) &hid_report_in[0]; // Set Source
		inPipes[0].info.bits.ctrl_trf_mem = _RAM;		// Set memory type
		inPipes[0].wCount = HID_INPUT_REPORT_BYTES;		// Set data count
		inPipes[0].info.bits.busy = 1;

	} else if (ReportSupported() == 3) {
		// Feature Report
		inPipes[0].pSrc.bRam = (unsigned char*) &hid_report_feature[0];	// Set Source
		inPipes[0].info.bits.ctrl_trf_mem = _RAM;		// Set memory type
		inPipes[0].wCount = HID_FEATURE_REPORT_BYTES;		// Set data count
		inPipes[0].info.bits.busy = 1;
	}
}

/*
 * Check to see if an Output or Feature report has arrived
 * on the control pipe. If yes, extract and use the data.
 */
static void mySetReportHandler (void)
{
	unsigned char count = 0;

	// Find out if an Output or Feature report has arrived on the control pipe.
	// Get the report type from the Setup packet.

	switch (SetupPkt.W_Value >> 8) {
	case 0x02:				// Output report
    		switch (SetupPkt.W_Value & 0xff) {
		case 0:				// Report ID 0
			// This example application copies the Output report data
			// to hid_report_in.
			// (Assumes Input and Output reports are the same length.)
			// A "real" application would do something more useful with the data.

			// wCount holds the number of bytes read in the Data stage.
			// This example assumes the report fits in one transaction.

			for (count=0; count <= HID_OUTPUT_REPORT_BYTES - 1; count++) {
				hid_report_in[count] = hid_report_out[count] ;
    			}
			break;
		}
		break;

	case 0x03:			// Feature report
		// Get the report ID from the Setup packet.

    		switch (SetupPkt.W_Value & 0xff) {
		case 0:				// Report ID 0
			// The Feature report data is in hid_report_feature.
			// This example application just sends the data back in the next
			// Get_Report request for a Feature report.

			// wCount holds the number of bytes read in the Data stage.
			// This example assumes the report fits in one transaction.

			// The Feature report uses a single buffer so to send the same data back
			// in the next IN Feature report, there is nothing to copy.
			// The data is in hid_report_feature[HID_FEATURE_REPORT_BYTES]

			break;

		}
		break;
	}
}

void HIDSetReportHandler (void)
{
	if (ReportSupported() == 2) {
		// Output Report
		outPipes[0].wCount = SetupPkt.wLength;
		outPipes[0].pFunc = mySetReportHandler;
		outPipes[0].pDst.bRam = (unsigned char*) &hid_report_out[0];
		outPipes[0].info.bits.busy = 1;

	} else if (ReportSupported() == 3) {
		// Feature Report
		outPipes[0].wCount = SetupPkt.wLength;
		outPipes[0].pFunc = mySetReportHandler;
		outPipes[0].pDst.bRam = (unsigned char*) &hid_report_feature[0];
		outPipes[0].info.bits.busy = 1;
	}
}
