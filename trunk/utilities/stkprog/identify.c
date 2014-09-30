/*
 * Identify AVR device.
 */
#include <stdio.h>
#include <sys/types.h>
#include "avr.h"
#include "prog.h"
#include "avr-int.h"

extern int debug;

void avr_identify (avr_t *avr)
{
	/* Assume ATmega128. */
	avr->flash_size = 0xfe00 * 2;
	avr->have_fuse = 1;
	avr->page_delay = 5;
	avr->page_size = 256;

	/* Identify device according to family. */
	switch (avr->part_family) {
	case 0x01:
		avr->have_fuse = 0;
		avr->flash_size = 0x20000;
		avr->page_delay = 56;
		avr->name = "Old ATmega103";
		if (avr->vendor_code == 0 && avr->part_number == 2) {
			if (debug)
				printf ("Device is protected, assuming ATmega103.\n");
			avr->name = "Protected";
		}
		break;

	case 0x90:
		avr->flash_size = 1024;
		avr->page_size = 32;
		switch (avr->part_number) {
		case 0x01:
			avr->name = "AT90s1200";
			avr->page_size = 0;
			break;
		case 0x04:
			avr->name = "ATtiny11";
			avr->page_size = 0;
			break;
		case 0x05:
			avr->name = "ATtiny12";
			avr->page_size = 0;
			break;
		case 0x06:
			avr->name = "ATtiny15";
			avr->page_size = 0;
			break;
		case 0x07:
			avr->name = "ATtiny13";
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATtiny13.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	case 0x91:
		avr->flash_size = 2048;
		avr->page_size = 32;
		switch (avr->part_number) {
		case 0x01:
			avr->name = "AT90s2313";
			avr->page_size = 0;
			break;
		case 0x02:
			avr->name = "AT90s2323";
			avr->page_size = 0;
			break;
		case 0x03:
			avr->name = "AT90s2343";
			avr->page_size = 0;
			break;
		case 0x06:
			avr->name = "ATtiny22";
			avr->page_size = 0;
			break;
		case 0x07:
			avr->name = "ATtiny28";
			avr->page_size = 0;
			break;
		case 0x08:
			avr->name = "ATtiny25";
			break;
		case 0x09:
			avr->name = "ATtiny26";
			break;
		case 0x0A:
			avr->name = "ATtiny2313";
			break;
		case 0x0B:
			avr->name = "ATtiny24";
			break;
		case 0x0C:
			avr->name = "ATtiny261";
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATtiny24.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	case 0x92:
		avr->flash_size = 4096;
		avr->page_size = 64;
		switch (avr->part_number) {
		case 0x01:
			avr->name = "AT90s4414";
			avr->page_size = 0;
			break;
		case 0x03:
			avr->name = "AT90s4433";
			avr->page_size = 0;
			break;
		case 0x05:
			avr->name = "ATmega48";
			break;
		case 0x06:
			avr->name = "ATtiny45";
			break;
		case 0x07:
			avr->name = "ATtiny44";
			avr->page_size = 32;
			break;
		case 0x08:
			avr->name = "ATtiny461";
			avr->page_size = 32;
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATtiny44.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	case 0x93:
		avr->flash_size = 0xf80*2;
		avr->page_size = 64;
		switch (avr->part_number) {
		case 0x01:
			avr->name = "AT90s8515";
			avr->page_size = 0;
			break;
		case 0x03:
			avr->name = "AT90s8535";
			avr->page_size = 0;
			break;
		case 0x06:
			avr->name = "ATmega8515";
			break;
		case 0x07:
			avr->name = "ATmega8";
			break;
		case 0x08:
			avr->name = "ATmega8535";
			break;
		case 0x0A:
			avr->name = "ATmega88";
			break;
		case 0x0B:
			avr->name = "ATtiny85";
			break;
		case 0x0C:
			avr->name = "ATtiny84";
			break;
		case 0x0D:
			avr->name = "ATtiny861";
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATmega8.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	case 0x94:
		avr->flash_size = 0x1f80*2;
		avr->page_size = 128;
		switch (avr->part_number) {
		case 0x01:
			avr->name = "ATmega161";
			break;
		case 0x02:
			avr->name = "ATmega163";
			break;
		case 0x03:
			avr->name = "ATmega16";
			break;
		case 0x04:
			avr->name = "ATmega162";
			break;
		case 0x05:
			avr->name = "ATmega169";
			break;
		case 0x06:
			avr->name = "ATmega168";
			break;
		case 0x07:
			avr->name = "ATmega165";
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATmega16.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	case 0x95:
		avr->flash_size = 0x3f00*2;
		avr->page_size = 128;
		switch (avr->part_number) {
		case 0x01:
			avr->name = "ATmega323";
			break;
		case 0x02:
			avr->name = "ATmega32";
			break;
		case 0x03:
			avr->name = "ATmega329";
			break;
		case 0x04:
			avr->name = "ATmega3290";
			break;
		case 0x05:
			avr->name = "ATmega325";
			break;
		case 0x06:
			avr->name = "ATmega3250";
			break;
		case 0x07:
			avr->name = "ATmega406";
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATmega32.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	case 0x96:
		avr->flash_size = 0x7e00*2;
		switch (avr->part_number) {
		case 0x01:
			avr->name = "ATmega603";
			avr->flash_size = 0x10000;
			avr->have_fuse = 0;
			avr->page_delay = 56;
			break;
		case 0x02:
			avr->name = "ATmega64";
			break;
		case 0x03:
			avr->name = "ATmega649";
			break;
		case 0x04:
			avr->name = "ATmega6490";
			break;
		case 0x05:
			avr->name = "ATmega645";
			break;
		case 0x06:
			avr->name = "ATmega6450";
			break;
		case 0x08:
			avr->name = "ATmega640";
			break;
		case 0x09:
			avr->name = "ATmega644";
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATmega64.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	case 0x97:
		switch (avr->part_number) {
		case 0x01:
			avr->name = "ATmega103";
			avr->flash_size = 0x20000;
			avr->have_fuse = 0;
			avr->page_delay = 56;
			break;
		case 0x02:
			avr->name = "ATmega128";
			break;
		case 0x03:
			avr->name = "ATmega1280";
			break;
		case 0x04:
			avr->name = "ATmega1281";
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATmega128.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	case 0x98:
		avr->flash_size = 0x3E000; // LY: use only first 248K, see ERRATA;
		switch (avr->part_number) {
		case 0x01:
			avr->name = "ATmega2560";
			break;
		case 0x02:
			avr->name = "ATmega2561";
			break;
		default:
			if (debug)
				printf ("Unknown part number %#x, assuming ATmega256.\n",
					avr->part_number);
			avr->name = "Unknown";
			break;
		}
		break;

	default:
		if (debug)
			printf ("Unknown part family %#x, assuming ATmega128.\n",
				avr->part_family);
		avr->name = "Unknown";
		break;
	}
}
