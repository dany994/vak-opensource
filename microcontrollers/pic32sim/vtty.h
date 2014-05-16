/*
 * Virtual console TTY.
 * Copyright (c) 2005,2006 Christophe Fillot (cf@utc.fr)
 * Copyright (C) yajin 2008 <yajinzhou@gmail.com >
 * Copyright (C) 2014 Serge Vakulenko <serge@vak.ru>
 *
 * This file is part of the pic32sim distribution.
 * See LICENSE file for terms of the license.
 */
#include <pthread.h>

/* Number of ports instantiated */
#define VTTY_NPORTS 6

/* VTTY connection types */
enum {
    VTTY_TYPE_NONE = 0,
    VTTY_TYPE_TERM,
    VTTY_TYPE_TCP,
};

/* create a virtual tty */
void vtty_create (unsigned port, char *name, int type, int tcp_port);

/* delete a virtual tty */
void vtty_delete (unsigned port);

/* read a character from the buffer (-1 if the buffer is empty) */
int vtty_get_char (unsigned port);

/* print a character to vtty */
void vtty_put_char (unsigned port, char ch);

/* returns TRUE if a character is available in buffer */
int vtty_is_char_avail (unsigned port);
int vtty_is_full (unsigned port);

/* Initialize the VTTY thread */
void vtty_init (void);
