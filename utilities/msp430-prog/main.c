/*
 * Программатор flash-памяти для микроконтроллеров TI MSP430.
 *
 * Copyright (C) 2009 Сергей Вакуленко
 *
 * Этот файл распространяется в надежде, что он окажется полезным, но
 * БЕЗ КАКИХ БЫ ТО НИ БЫЛО ГАРАНТИЙНЫХ ОБЯЗАТЕЛЬСТВ; в том числе без косвенных
 * гарантийных обязательств, связанных с ПОТРЕБИТЕЛЬСКИМИ СВОЙСТВАМИ и
 * ПРИГОДНОСТЬЮ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
 *
 * Вы вправе распространять и/или изменять этот файл в соответствии
 * с условиями Генеральной Общественной Лицензии GNU (GPL) в том виде,
 * как она была опубликована Фондом Свободного ПО; либо версии 2 Лицензии
 * либо (по вашему желанию) любой более поздней версии. Подробности
 * смотрите в прилагаемом файле 'COPYING.txt'.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/time.h>
#ifdef MINGW32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "libmsp430.h"

#define PROGNAME	"Programmer for TI MSP430"
#define VERSION		"1.0"
#define BLOCKSZ		1024

/* Macros for converting between hex and binary. */
#define NIBBLE(x)	(isdigit(x) ? (x)-'0' : tolower(x)+10-'a')
#define HEX(buffer)	((NIBBLE((buffer)[0])<<4) + NIBBLE((buffer)[1]))

unsigned char memory_data [0x40000];	/* Code - up to 256 kbytes */
int memory_len;
unsigned memory_base;
unsigned progress_count;
int debug;
char *progname;

void *fix_time ()
{
	static struct timeval t0;

	gettimeofday (&t0, 0);
	return &t0;
}

unsigned mseconds_elapsed (void *arg)
{
	struct timeval t1, *t0 = arg;
	unsigned mseconds;

	gettimeofday (&t1, 0);
	mseconds = (t1.tv_sec - t0->tv_sec) * 1000 +
		(t1.tv_usec - t0->tv_usec) / 1000;
	if (mseconds < 1)
		mseconds = 1;
	return mseconds;
}

/*
 * Read the S record file.
 */
int read_srec (char *filename, unsigned char *output)
{
	FILE *fd;
	unsigned char buf [256];
	unsigned char *data;
	unsigned address;
	int bytes, output_len;

	fd = fopen (filename, "r");
	if (! fd) {
		perror (filename);
		exit (1);
	}
	output_len = 0;
	while (fgets ((char*) buf, sizeof(buf), fd)) {
		if (buf[0] == '\n')
			continue;
		if (buf[0] != 'S') {
			if (output_len == 0)
				break;
			fprintf (stderr, "%s: bad file format\n", filename);
			exit (1);
		}
		if (buf[1] == '7' || buf[1] == '8' || buf[1] == '9')
			break;

		/* Starting an S-record.  */
		if (! isxdigit (buf[2]) || ! isxdigit (buf[3])) {
			fprintf (stderr, "%s: bad record: %s\n", filename, buf);
			exit (1);
		}
		bytes = HEX (buf + 2);

		/* Ignore the checksum byte.  */
		--bytes;

		address = 0;
		data = buf + 4;
		switch (buf[1]) {
		case '3':
			address = HEX (data);
			data += 2;
			--bytes;
			/* Fall through.  */
		case '2':
			address = (address << 8) | HEX (data);
			data += 2;
			--bytes;
			/* Fall through.  */
		case '1':
			address = (address << 8) | HEX (data);
			data += 2;
			address = (address << 8) | HEX (data);
			data += 2;
			bytes -= 2;

			if (! memory_base) {
				/* Автоматическое определение базового адреса. */
				memory_base = address;
			}
			if (address < memory_base) {
				fprintf (stderr, "%s: incorrect address %05X, must be %05X or greater\n",
					filename, address, memory_base);
				exit (1);
			}
			address -= memory_base;
			if (address+bytes > sizeof (memory_data)) {
				fprintf (stderr, "%s: address too large: %05X + %05X\n",
					filename, address + memory_base, bytes);
				exit (1);
			}
			while (bytes-- > 0) {
				output[address++] = HEX (data);
				data += 2;
			}
			if (output_len < (int) address)
				output_len = address;
			break;
		}
	}
	fclose (fd);
	return output_len;
}

void print_symbols (char symbol, int cnt)
{
	while (cnt-- > 0)
		putchar (symbol);
}

void load_library ()
{
#ifdef MINGW32
	HINSTANCE h;

	h = LoadLibrary (LIBRARY);
	if (! h) {
		fprintf (stderr, "%s: not found\n", LIBRARY);
		exit (1);
	}
	MSP430_Initialize       = (void*) GetProcAddress (h, "MSP430_Initialize");
	MSP430_Close            = (void*) GetProcAddress (h, "MSP430_Close");
	MSP430_Configure        = (void*) GetProcAddress (h, "MSP430_Configure");
	MSP430_VCC              = (void*) GetProcAddress (h, "MSP430_VCC");
	MSP430_Identify         = (void*) GetProcAddress (h, "MSP430_Identify");
	MSP430_Reset            = (void*) GetProcAddress (h, "MSP430_Reset");
	MSP430_Erase            = (void*) GetProcAddress (h, "MSP430_Erase");
	MSP430_Memory           = (void*) GetProcAddress (h, "MSP430_Memory");
#else
	void *h;

	h = dlopen (LIBRARY, 2);
	if (! h) {
		fprintf (stderr, "%s: not found\n", LIBRARY);
		exit (1);
	}
	MSP430_Initialize       = (void*) dlsym (h, "MSP430_Initialize");
	MSP430_Close            = (void*) dlsym (h, "MSP430_Close");
	MSP430_Configure        = (void*) dlsym (h, "MSP430_Configure");
	MSP430_VCC              = (void*) dlsym (h, "MSP430_VCC");
	MSP430_Identify         = (void*) dlsym (h, "MSP430_Identify");
	MSP430_Reset            = (void*) dlsym (h, "MSP430_Reset");
	MSP430_Erase            = (void*) dlsym (h, "MSP430_Erase");
	MSP430_Memory           = (void*) dlsym (h, "MSP430_Memory");
#endif
	if (! MSP430_Initialize || ! MSP430_Close || ! MSP430_Configure ||
	    ! MSP430_VCC || ! MSP430_Identify || ! MSP430_Reset ||
	    ! MSP430_Erase || ! MSP430_Memory) {
		fprintf (stderr, "%s: incompatible library\n", LIBRARY);
		exit (1);
	}
}

void quit (void)
{
	if (MSP430_Reset)
		MSP430_Reset (RESET_VCC, 1, 1);
	if (MSP430_Close)
		MSP430_Close (0);
}

int block_is_clean (unsigned addr, int len)
{
	int i;

	/* Skip clean blocks. */
	for (i=0; i<len; i++) {
		if (memory_data [addr+i] != 0xff) {
			if (debug)
				fprintf (stderr, "write %02x at address %04x\n",
					memory_data [addr+i], addr + i + memory_base);
			return 0;
		}
	}
	return 1;
}

void program_block (unsigned addr, int len)
{
	/* Write flash memory. */
	if (MSP430_Memory (memory_base + addr, memory_data + addr,
	    len, MEMORY_WRITE) != 0) {
		fprintf (stderr, "Error writing flash.\n");
		exit (1);
	}
}

void progress ()
{
	++progress_count;
	putchar ("/-\\|" [progress_count & 3]);
	putchar ('\b');
	putchar ('#');
	fflush (stdout);
}

void verify_block (unsigned addr, int len)
{
	int i;
	unsigned char block [BLOCKSZ];
	unsigned word, expected;

	if (MSP430_Memory (memory_base + addr, block, len,
	    MEMORY_READ) != 0) {
		fprintf (stderr, "Error reading memory.\n");
		exit (1);
	}
	for (i=0; i<len; i+=2) {
		expected = *(unsigned short*) (memory_data + addr + i);
		if (expected == 0xffff)
			continue;
		word = *(unsigned short*) (block + i);
		if (debug)
			fprintf (stderr, "read word %04X at address %05X\n",
				word, addr + i + memory_base);
		if (word != expected) {
			printf ("\nerror at address %05X: file=%04X, mem=%04X\n",
				addr + i + memory_base, expected, word);
			exit (1);
		}
	}
}

void do_program (int verify_only)
{
	unsigned addr;
	int len, first, last;
	void *t0;

	printf ("Memory:");
	progress_count = 0;
	first = -1;
	last = 0;
	for (addr=0; (int)addr<memory_len; addr+=BLOCKSZ) {
		len = BLOCKSZ;
		if (memory_len - addr < len)
			len = memory_len - addr;
		if (block_is_clean (addr, len)) {
			if (first >= 0) {
				printf (" %05X-%05X,", memory_base + first,
					memory_base + last + BLOCKSZ-1);
				first = -1;
			}
		} else {
			++progress_count;
			if (first < 0)
				first = addr;
			last = addr;
		}
	}
	if (first >= 0)
		printf (" %05X-%05X,", memory_base + first,
			memory_base + last + BLOCKSZ-1);
	printf (" total %d bytes\n", progress_count * BLOCKSZ);

	if (MSP430_Configure (CONFIGURE_LOCKED_FLASH_ACCESS, 1) != 0) {
		fprintf (stderr, "Error enabling locked flash access.\n");
		exit (1);
	}
	if (! verify_only) {
		/* Erase flash. */
		if (MSP430_Erase (ERASE_MAIN, memory_base, memory_len) != 0) {
			fprintf (stderr, "Error erasing flash.\n");
			exit (1);
		}
	}
	printf (verify_only ? "Verify: " : "Program: " );
	print_symbols ('.', progress_count);
	print_symbols ('\b', progress_count);
	fflush (stdout);

	progress_count = 0;
	t0 = fix_time ();
	for (addr=0; (int)addr<memory_len; addr+=BLOCKSZ) {
		len = BLOCKSZ;
		if (memory_len - addr < len)
			len = memory_len - addr;
		if (block_is_clean (addr, len))
			continue;
		if (! verify_only)
			program_block (addr, len);
		progress ();
		verify_block (addr, len);
	}
	printf (" done\n");
	printf ("Rate: %ld bytes per second\n",
		memory_len * 1000L / mseconds_elapsed (t0));
}

void do_probe (const char *port, int iface)
{
	char devtype [80];
	long version;

	/* Open and detect the device. */
	if (MSP430_Initialize (port, &version) != 0) {
		fprintf (stderr, "%s: cannot detect device -- check cable!\n",
			port);
		exit (1);
	}
	atexit (quit);

	printf ("Library: %s", LIBRARY);
	if (version >= 0)
		printf (", version %ld", version);
	printf ("\n");

	if (MSP430_Configure (CONFIGURE_INTERFACE_MODE, iface) != 0) {
		fprintf (stderr, "Error setting interface -- check cable!\n");
		exit (1);
	}
	if (MSP430_VCC (3300) != 0) {
		fprintf (stderr, "Error setting VCC.\n");
		exit (1);
	}
	MSP430_Reset (RESET_RST, 0, 0);
	if (MSP430_Identify (devtype, sizeof (devtype), 0) != 0) {
		fprintf (stderr, "Cannot identify microcontroller -- check power!\n");
		exit (1);
	}
	printf ("Device type: %s\n", &devtype[4]);
}

int main (int argc, char **argv)
{
	int ch, verify_only = 0;

	setvbuf (stdout, (char *)NULL, _IOLBF, 0);
	setvbuf (stderr, (char *)NULL, _IOLBF, 0);
	printf (PROGNAME ", Version " VERSION ", Copyright (C) 2009 Serge Vakulenko\n");
	progname = argv[0];

	while ((ch = getopt(argc, argv, "vDh")) != -1) {
		switch (ch) {
		case 'v':
			++verify_only;
			continue;
		case 'D':
			++debug;
			continue;
		case 'h':
			break;
		}
usage:		printf ("Probe:\n");
		printf ("        msp430-prog\n");
		printf ("\nWrite flash memory:\n");
		printf ("        msp430-prog [-v] file.srec\n");
		printf ("\nArgs:\n");
		printf ("        file.srec  Code file in SREC format\n");
		printf ("        -v         Verify only\n");
		exit (0);
	}
	argc -= optind;
	argv += optind;
	if (argc > 1)
		goto usage;

	load_library ();
#ifdef MINGW32
	do_probe ("COM1", INTERFACE_SPYBIWIRE_IF);
#else
	do_probe ("/dev/ttyUSB0", INTERFACE_SPYBIWIRE_IF);
#endif
	if (argc) {
		memset (memory_data, 0xff, sizeof (memory_data));
		memory_len = read_srec (argv[0], memory_data);
		if (memory_len == 0) {
			fprintf (stderr, "%s: read error\n", argv[0]);
			exit (1);
		}
		do_program (verify_only);
	}
	quit ();
	return 0;
}
