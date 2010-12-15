/****************************************************************************
 File        : usb-linux.c
 Description : Encapsulates all nonportable, Linux-specific USB I/O code
               within the ubw32 program.  Each supported operating system has
               its own source file, providing a common calling syntax to the
               portable sections of the code.
 History     : 3/16/2009  Initial Linux support for ubw32 program.
 License     : Copyright 2009 Phillip Burgess - pburgess@dslextreme.com

               This file is part of 'ubw32' program.

               'ubw32' is free software: you can redistribute it and/or
               modify it under the terms of the GNU General Public License
               as published by the Free Software Foundation, either version
               3 of the License, or (at your option) any later version.

               'ubw32' is distributed in the hope that it will be useful,
               but WITHOUT ANY WARRANTY; without even the implied warranty
               of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
               See the GNU General Public License for more details.

               You should have received a copy of the GNU General Public
               License along with 'ubw32' source code.  If not,
               see <http://www.gnu.org/licenses/>.

               This license applies specifically to the 'ubw32' software,
               which does not originate from nor is directly affiliated with
               UBW32 hardware developer Brian Schmalz, manufacturer/
               distributor SparkFun Electronics, nor component supplier
               Microchip Technology.  The UBW32 hardware, design files and
               documents, bootloader and firmware code are the property of
               their respective rights holders and may or may not be
               distributed under different terms than this software.
 ****************************************************************************/

#include <stdio.h>
#include <usb.h>
#include <hid.h>
#include "ubw32.h"
#include <errno.h>

static HIDInterface *hid = NULL;
unsigned char        usbBuf[64];

/****************************************************************************
 Function    : usbOpen
 Description : Searches for and opens the first available UBW32 device.
 Parameters  : unsigned short         Vendor ID to search for.
               unsigned short         Product ID to search for.
 Returns     : Status code:
                 ERR_NONE             Success; device open and ready for I/O.
                 ERR_USB_INIT1        Initialization error in HID init code.
                 ERR_USB_INIT2        New HID alloc failed.
                 ERR_UBW32_NOT_FOUND  UBW32 not detected on any USB bus
                                      (might be connected but not in
                                       Bootloader mode).
 Notes       : If multiple UBW32 devices are connected, only the first device
               found (and not in use by another application) is returned.
               This code sets no particular preference or sequence in the
               search ordering; whatever the default libhid 'matching
               function' decides.
 ****************************************************************************/
ErrorCode usbOpen(
  const unsigned short vendorID,
  const unsigned short productID)
{
	ErrorCode           status = ERR_USB_INIT1;
	HIDInterfaceMatcher matcher;

	matcher.vendor_id  = vendorID;
	matcher.product_id = productID;
	matcher.matcher_fn = NULL;

	if(HID_RET_SUCCESS == hid_init()) {
		status = ERR_USB_INIT2;
		if((hid = hid_new_HIDInterface())) {
			if(HID_RET_SUCCESS ==
			  hid_force_open(hid,0,&matcher,3)) {
				return ERR_NONE;
			}
			status = ERR_UBW32_NOT_FOUND;
			hid_delete_HIDInterface(&hid);
		}
		hid_cleanup();
	}

	return status;
}

/****************************************************************************
 Function    : usbWrite
 Description : Write data packet to currently-open UBW32 device, optionally
               followed by a packet read operation.  Data source is always
               global array usbBuf[].  For read operation, destination is
               always usbBuf[] also, overwriting contents there.
 Parameters  : char       Size of source data in bytes (max 64).
               char       If set, read response packet.
 Returns     : ErrorCode  ERR_NONE on success, ERR_USB_WRITE on error.
 Notes       : Device is assumed to have already been successfully opened
               by the time this function is called; no checks performed here.
 ****************************************************************************/
ErrorCode usbWrite(
  const char len,
  const char read)
{
#ifdef DEBUG
	int i;
	(void)puts("Sending:");
	for(i=0;i<8;i++) (void)printf("%02x ",((unsigned char *)usbBuf)[i]);
	(void)printf(": ");
	for(;i<64;i++) (void)printf("%02x ",((unsigned char *)usbBuf)[i]);
	(void)putchar('\n'); fflush(stdout);
	DEBUGMSG("\nAbout to write");
#endif

	if(HID_RET_SUCCESS != hid_interrupt_write(hid,0x01,usbBuf,len,0))
		return ERR_USB_WRITE;

	DEBUGMSG("Done w/write");

	if(read) {
		DEBUGMSG("About to read");
		if(HID_RET_SUCCESS != hid_interrupt_read(hid,0x81,usbBuf,64,0))
			return ERR_USB_READ;
#ifdef DEBUG
		(void)puts("Done reading\nReceived:");
		for(i=0;i<8;i++) (void)printf("%02x ",usbBuf[i]);
		(void)printf(": ");
		for(;i<64;i++) (void)printf("%02x ",usbBuf[i]);
		(void)putchar('\n'); fflush(stdout);
#endif
	}

	return ERR_NONE;
}

/****************************************************************************
 Function    : usbClose
 Description : Closes previously-opened USB device.
 Parameters  : None (void)
 Returns     : Nothing (void)
 Notes       : Device is assumed to have already been successfully opened
               by the time this function is called; no checks performed here.
 ****************************************************************************/
void usbClose(void)
{
	(void)hid_close(hid);
	hid_delete_HIDInterface(&hid);
	(void)hid_cleanup();
	hid = NULL;
}
