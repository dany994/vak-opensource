/*
 * Generic interface to a debug port adapter.
 *
 * Copyright (C) 2011 Serge Vakulenko
 *
 * This file is part of PIC32PROXY project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdarg.h>

typedef struct _adapter_t adapter_t;

struct _adapter_t {
    void (*close) (adapter_t *a, int power_on);
    unsigned (*get_idcode) (adapter_t *a);
    void (*read_data) (adapter_t *a, unsigned addr, unsigned nwords, unsigned *data);
    unsigned (*read_word) (adapter_t *a, unsigned addr);
};

adapter_t *adapter_open_pickit (void);
adapter_t *adapter_open_mpsse (void);

void mdelay (unsigned msec);
extern int debug_level;
