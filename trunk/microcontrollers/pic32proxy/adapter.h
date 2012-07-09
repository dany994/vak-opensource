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
    const char *name;

    void (*close) (adapter_t *a, int power_on);
    unsigned (*get_idcode) (adapter_t *a);
    int (*cpu_stopped) (adapter_t *a);
    void (*reset_cpu) (adapter_t *a);
    void (*stop_cpu) (adapter_t *a);
    void (*exec) (adapter_t *a, int cycle,
                  int num_code_words, const unsigned *code,
                  int num_param_in, unsigned *param_in,
                  int num_param_out, unsigned *param_out);
};

adapter_t *adapter_open_pickit (void);
adapter_t *adapter_open_mpsse (void);

void mdelay (unsigned msec);
extern int debug_level;
