/*
 * Define memory map for PIC32 microcontroller.
 *
 * Copyright (C) 2014 Serge Vakulenko, <serge@vak.ru>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#define PROGRAM_MEM_START   0x1d000000
#define PROGRAM_MEM_SIZE    (512*1024)          // 512 kbytes
#define BOOT_MEM_START      0x1fc00000
#define BOOT_MEM_SIZE       (12*1024)           // 12 kbytes
#define DATA_MEM_START      0x00000000
#define DATA_MEM_SIZE       (128*1024)          // 128 kbytes
#define IO_MEM_START        0x1f800000
#define IO_MEM_SIZE         (64*1024)           // 64 kbytes
#define IO2_MEM_START       0x1f880000
#define IO2_MEM_SIZE        (64*1024)           // 64 kbytes

#define IN_PROGRAM_MEM(addr) (addr >= PROGRAM_MEM_START && \
                             addr < PROGRAM_MEM_START+PROGRAM_MEM_SIZE)
#define IN_BOOT_MEM(addr)   (addr >= BOOT_MEM_START && \
                             addr < BOOT_MEM_START+BOOT_MEM_SIZE)

#define PROGMEM(addr) progmem [(addr & 0xfffff) >> 2]
#define BOOTMEM(addr) bootmem [(addr & 0xffff) >> 2]


extern char *progname;          // base name of current program
extern int trace_peripherals;   // trace special function registers

int load_file(void *progmem, void *bootmem, const char *filename);

void io_init (void *datap, void *data2p, void *bootp);
void io_reset (void);
unsigned io_read32 (unsigned address, unsigned *bufp, const char **namep);
void io_write32 (unsigned address, unsigned *bufp, unsigned data, const char **namep);

void eic_level_vector (int ripl, int vector);

extern unsigned sdcard_spi_port;    // SPI port number of SD card
extern unsigned sdcard_gpio_port0;  // GPIO port number of CS0 signal
extern unsigned sdcard_gpio_port1;  // GPIO port number of CS1 signal
extern unsigned sdcard_gpio_cs0;    // GPIO pin mask of CS0 signal
extern unsigned sdcard_gpio_cs1;    // GPIO pin mask of CS1 signal

void sdcard_init (int unit, const char *name, const char *filename, int cs_port, int cs_pin);
void sdcard_reset (void);
void sdcard_select (int unit, int on);
unsigned sdcard_io (unsigned data);

void vtty_create (unsigned port, char *name, int tcp_port);
void vtty_delete (unsigned port);
int vtty_get_char (unsigned port);
void vtty_put_char (unsigned port, char ch);
int vtty_is_char_avail (unsigned port);
int vtty_is_full (unsigned port);
void vtty_init (void);
