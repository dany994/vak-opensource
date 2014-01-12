/*
 * Interface to MK-54 calculator via USB HIP port.
 *
 * Copyright (C) 2014 Serge Vakulenko
 *
 * This file is part of PIC32PROG project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
typedef struct _device_t device_t;

device_t *device_open (int debug_level);

void device_close (device_t *t);

void device_read (device_t *t, unsigned char data[98]);

void device_program (device_t *t, unsigned char data[98]);
