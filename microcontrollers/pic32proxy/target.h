/*
 * Generic debug interface to a target microcontroller.
 *
 * Copyright (C) 2011 Serge Vakulenko
 *
 * This file is part of PIC32PROXY project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
typedef struct _target_t target_t;

target_t *target_open (void);
void target_close (target_t *t, int power_on);

unsigned target_idcode (target_t *t);
const char *target_cpu_name (target_t *t);

unsigned target_read_word (target_t *t, unsigned addr);
void target_read_block (target_t *t, unsigned addr,
	unsigned nwords, unsigned *data);

void target_write_word (target_t *t, unsigned addr, unsigned word);
void target_write_block (target_t *t, unsigned addr,
	unsigned nwords, unsigned *data);

void target_stop (target_t *t);
void target_step (target_t *t);
void target_resume (target_t *t);
void target_run (target_t *t, unsigned addr);
void target_restart (target_t *t);
int target_is_stopped (target_t *t, int *is_aborted);

unsigned target_read_register (target_t *t, unsigned regno);
void target_write_register (target_t *t, unsigned regno, unsigned val);

void target_add_break (target_t *t, unsigned addr, int type);
void target_remove_break (target_t *t, unsigned addr);
