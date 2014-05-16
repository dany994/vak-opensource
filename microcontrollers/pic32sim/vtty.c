/*
 * Virtual console TTY.
 * Copyright (c) 2005,2006 Christophe Fillot (cf@utc.fr)
 * Copyright (C) yajin 2008 <yajinzhou@gmail.com>
 * Copyright (C) 2014 Serge Vakulenko <serge@vak.ru>
 *
 * "Interactive" part idea by Mtve.
 * TCP console added by Mtve.
 * Serial console by Peter Ross (suxen_drol@hotmail.com)
 *
 * This file is part of the pic32sim distribution.
 * See LICENSE file for terms of the license.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include "globals.h"
#include "vtty.h"

/*
 * 4 Kb should be enough for a keyboard buffer
 */
#define VTTY_BUFFER_SIZE    4096

/*
 * VTTY connection states (for TCP)
 */
enum {
    VTTY_STATE_TCP_INVALID,     /* connection is not working */
    VTTY_STATE_TCP_WAITING,     /* waiting for incoming connection */
    VTTY_STATE_TCP_RUNNING,     /* character reading/writing ok */
};

/*
 * VTTY input states
 */
enum {
    VTTY_INPUT_TEXT,
    VTTY_INPUT_VT1,
    VTTY_INPUT_VT2,
    VTTY_INPUT_REMOTE,
    VTTY_INPUT_TELNET,
    VTTY_INPUT_TELNET_IYOU,
    VTTY_INPUT_TELNET_SB1,
    VTTY_INPUT_TELNET_SB2,
    VTTY_INPUT_TELNET_SB_TTYPE,
    VTTY_INPUT_TELNET_NEXT
};

/*
 * Virtual TTY structure
 */
typedef struct virtual_tty vtty_t;
struct virtual_tty {
    char *name;
    int type, state;
    int tcp_port;
    int terminal_support;
    int input_state;
    int telnet_cmd, telnet_opt, telnet_qual;
    int fd, accept_fd, *select_fd;
    FILE *fstream;
    u_char buffer[VTTY_BUFFER_SIZE];
    u_int read_ptr, write_ptr;
    pthread_mutex_t lock;
};

static vtty_t porttab[VTTY_NPORTS];
static pthread_t vtty_thread;

#define VTTY_LOCK(tty)      pthread_mutex_lock(&(tty)->lock);
#define VTTY_UNLOCK(tty)    pthread_mutex_unlock(&(tty)->lock);

static struct termios tios, tios_orig;

/*
 * Send Telnet command: WILL TELOPT_ECHO
 */
static void vtty_telnet_will_echo (vtty_t * vtty)
{
    u_char cmd[] = { IAC, WILL, TELOPT_ECHO };
    if (write (vtty->fd, cmd, sizeof (cmd)) < 0)
        perror ("vtty_telnet_will_echo");
}

/*
 * Send Telnet command: Suppress Go-Ahead
 */
static void vtty_telnet_will_suppress_go_ahead (vtty_t * vtty)
{
    u_char cmd[] = { IAC, WILL, TELOPT_SGA };
    if (write (vtty->fd, cmd, sizeof (cmd)) < 0)
        perror ("vtty_telnet_will_suppress_go_ahead");
}

/*
 * Send Telnet command: Don't use linemode
 */
static void vtty_telnet_dont_linemode (vtty_t * vtty)
{
    u_char cmd[] = { IAC, DONT, TELOPT_LINEMODE };
    if (write (vtty->fd, cmd, sizeof (cmd)) < 0)
        perror ("vtty_telnet_dont_linemode");
}

/*
 * Send Telnet command: does the client support terminal type message?
 */
static void vtty_telnet_do_ttype (vtty_t * vtty)
{
    u_char cmd[] = { IAC, DO, TELOPT_TTYPE };
    if (write (vtty->fd, cmd, sizeof (cmd)) < 0)
        perror ("vtty_telnet_do_ttype");
}

/*
 * Restore TTY original settings
 */
static void vtty_term_reset (void)
{
    tcsetattr (STDIN_FILENO, TCSANOW, &tios_orig);
}

/*
 * Initialize real TTY
 */
static void vtty_term_init (void)
{
    tcgetattr (STDIN_FILENO, &tios);

    memcpy (&tios_orig, &tios, sizeof (struct termios));
    atexit (vtty_term_reset);

    tios.c_cc[VTIME] = 0;
    tios.c_cc[VMIN] = 1;

    /* Disable Ctrl-C, Ctrl-S, Ctrl-Q and Ctrl-Z */
    tios.c_cc[VINTR] = 0;
    tios.c_cc[VSTART] = 0;
    tios.c_cc[VSTOP] = 0;
    tios.c_cc[VSUSP] = 0;

    tios.c_lflag &= ~(ICANON | ECHO);
    tios.c_iflag &= ~ICRNL;
    tcsetattr (STDIN_FILENO, TCSANOW, &tios);
    tcflush (STDIN_FILENO, TCIFLUSH);
}

/*
 * Wait for a TCP connection
 */
static int vtty_tcp_conn_wait (vtty_t * vtty)
{
    struct sockaddr_in serv;
    int one = 1;

    vtty->state = VTTY_STATE_TCP_INVALID;

    if ((vtty->accept_fd = socket (PF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("vtty_tcp_waitcon: socket");
        return (-1);
    }

    if (setsockopt (vtty->accept_fd, SOL_SOCKET, SO_REUSEADDR, &one,
            sizeof (one)) < 0) {
        perror ("vtty_tcp_waitcon: setsockopt(SO_REUSEADDR)");
        goto error;
    }

    memset (&serv, 0, sizeof (serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl (INADDR_ANY);
    serv.sin_port = htons (vtty->tcp_port);

    if (bind (vtty->accept_fd, (struct sockaddr *) &serv, sizeof (serv)) < 0) {
        perror ("vtty_tcp_waitcon: bind");
        goto error;
    }

    if (listen (vtty->accept_fd, 1) < 0) {
        perror ("vtty_tcp_waitcon: listen");
        goto error;
    }

    fprintf (stderr, "%s: waiting connection on tcp port %d (FD %d)\n", vtty->name,
        vtty->tcp_port, vtty->accept_fd);

    vtty->select_fd = &vtty->accept_fd;
    vtty->state = VTTY_STATE_TCP_WAITING;
    return (0);

error:
    close (vtty->accept_fd);
    vtty->accept_fd = -1;
    vtty->select_fd = NULL;
    return (-1);
}

/*
 * Accept a TCP connection
 */
static int vtty_tcp_conn_accept (vtty_t * vtty)
{
    if ((vtty->fd = accept (vtty->accept_fd, NULL, NULL)) < 0) {
        fprintf (stderr,
            "vtty_tcp_conn_accept: accept on port %d failed %s\n",
            vtty->tcp_port, strerror (errno));
        return (-1);
    }

    fprintf (stderr, "%s is now connected (accept_fd=%d, conn_fd=%d)\n",
        vtty->name, vtty->accept_fd, vtty->fd);

    /* Adapt Telnet settings */
    if (vtty->terminal_support) {
        vtty_telnet_do_ttype (vtty);
        vtty_telnet_will_echo (vtty);
        vtty_telnet_will_suppress_go_ahead (vtty);
        vtty_telnet_dont_linemode (vtty);
        vtty->input_state = VTTY_INPUT_TELNET;
    }

    vtty->fstream = fdopen (vtty->fd, "wb");
    if (! vtty->fstream) {
        close (vtty->fd);
        vtty->fd = -1;
        return (-1);
    }

    fprintf (vtty->fstream, "Connected to pic32sim - %s\r\n\r\n", vtty->name);

    vtty->select_fd = &vtty->fd;
    vtty->state = VTTY_STATE_TCP_RUNNING;
    return (0);
}

/*
 * Create a virtual tty
 */
void vtty_create (unsigned port, char *name, int type, int tcp_port)
{
    vtty_t *vtty = porttab + port;

    if (port >= VTTY_NPORTS) {
        fprintf (stderr, "%s: unable to create a virtual tty port #%u.\n",
            name, port);
        exit(1);
    }

    memset (vtty, 0, sizeof (*vtty));
    vtty->name = name;
    vtty->type = type;
    vtty->fd = -1;
    vtty->fstream = NULL;
    vtty->accept_fd = -1;
    pthread_mutex_init (&vtty->lock, NULL);
    vtty->input_state = VTTY_INPUT_TEXT;

    switch (vtty->type) {
    case VTTY_TYPE_TERM:
        vtty_term_init ();
        vtty->fd = STDIN_FILENO;
        vtty->select_fd = &vtty->fd;
        vtty->fstream = stdout;
        break;

    case VTTY_TYPE_TCP:
        vtty->terminal_support = 1;
        vtty->tcp_port = tcp_port;
        vtty_tcp_conn_wait (vtty);
        break;

    default:
        fprintf (stderr, "tty_create: bad vtty type %d\n", vtty->type);
        exit(1);
    }
}

/*
 * Delete a virtual tty
 */
void vtty_delete (unsigned port)
{
    vtty_t *vtty = porttab + port;

    if (port < VTTY_NPORTS && vtty->type != VTTY_TYPE_NONE) {

        if (vtty->fstream && vtty->fstream != stdout) {
            fclose (vtty->fstream);
            vtty->fstream = 0;
        }

        /* We don't close FD 0 since it is stdin */
        if (vtty->fd > 0) {
            fprintf (stderr, "%s: closing FD %d\n", vtty->name, vtty->fd);
            close (vtty->fd);
            vtty->fd = 0;
        }

        if (vtty->accept_fd >= 0) {
            fprintf (stderr, "%s: closing accept FD %d\n",
                vtty->name, vtty->accept_fd);
            close (vtty->accept_fd);
            vtty->accept_fd = -1;
        }
        vtty->type = VTTY_TYPE_NONE;
    }
}

/*
 * Store a character in the FIFO buffer
 */
static int vtty_store (vtty_t * vtty, u_char c)
{
    u_int nwptr;

    VTTY_LOCK (vtty);
    nwptr = vtty->write_ptr + 1;
    if (nwptr == VTTY_BUFFER_SIZE)
        nwptr = 0;

    if (nwptr == vtty->read_ptr) {
        VTTY_UNLOCK (vtty);
        return (-1);
    }

    vtty->buffer[vtty->write_ptr] = c;
    vtty->write_ptr = nwptr;
    VTTY_UNLOCK (vtty);
    return (0);
}

/*
 * Read a character from the terminal.
 */
static int vtty_term_read (vtty_t * vtty)
{
    u_char c;

    if (read (vtty->fd, &c, 1) == 1)
        return (c);

    perror ("read from vtty failed");
    return (-1);
}

/*
 * Read a character from the TCP connection.
 */
static int vtty_tcp_read (vtty_t * vtty)
{
    u_char c;

    switch (vtty->state) {
    case VTTY_STATE_TCP_RUNNING:
        if (read (vtty->fd, &c, 1) == 1)
            return (c);

        /* Problem with the connection: Re-enter wait mode */
        shutdown (vtty->fd, 2);
        fclose (vtty->fstream);
        close (vtty->fd);
        vtty->fstream = NULL;
        vtty->fd = -1;
        vtty->select_fd = &vtty->accept_fd;
        vtty->state = VTTY_STATE_TCP_WAITING;
        return (-1);

    case VTTY_STATE_TCP_WAITING:
        /* A new connection has arrived */
        vtty_tcp_conn_accept (vtty);
        return (-1);
    }

    /* Shouldn't happen... */
    return (-1);
}

/*
 * Read a character from the virtual TTY.
 *
 * If the VTTY is a TCP connection, restart it in case of error.
 */
static int vtty_read (vtty_t * vtty)
{
    switch (vtty->type) {
    case VTTY_TYPE_TERM:
        return (vtty_term_read (vtty));
    case VTTY_TYPE_TCP:
        return (vtty_tcp_read (vtty));
    default:
        fprintf (stderr, "vtty_read: bad vtty type %d\n", vtty->type);
        return (-1);
    }
}

/*
 * Read a character (until one is available) and store it in buffer
 */
static void vtty_read_and_store (int port)
{
    vtty_t *vtty = porttab + port;
    int c;

    /* wait until we get a character input */
    c = vtty_read (vtty);

    /* if read error, do nothing */
    if (c < 0)
        return;

    if (! vtty->terminal_support) {
        vtty_store (vtty, c);
        return;
    }

    switch (vtty->input_state) {
    case VTTY_INPUT_TEXT:
        switch (c) {
        case 0x1b:
            vtty->input_state = VTTY_INPUT_VT1;
            return;

            /* Ctrl + ']' (0x1d, 29), or Alt-Gr + '*' (0xb3, 179) */
        case 0x1d:
        case 0xb3:
            vtty->input_state = VTTY_INPUT_REMOTE;
            return;
        case IAC:
            vtty->input_state = VTTY_INPUT_TELNET;
            return;
        case 0:                /* NULL - Must be ignored - generated by Linux telnet */
        case 10:               /* LF (Line Feed) - Must be ignored on Windows platform */
            return;
        default:
            /* Store a standard character */
            vtty_store (vtty, c);
            return;
        }

    case VTTY_INPUT_VT1:
        switch (c) {
        case 0x5b:
            vtty->input_state = VTTY_INPUT_VT2;
            return;
        default:
            vtty_store (vtty, 0x1b);
            vtty_store (vtty, c);
        }
        vtty->input_state = VTTY_INPUT_TEXT;
        return;

    case VTTY_INPUT_VT2:
        switch (c) {
        case 0x41:             /* Up Arrow */
            vtty_store (vtty, 16);
            break;
        case 0x42:             /* Down Arrow */
            vtty_store (vtty, 14);
            break;
        case 0x43:             /* Right Arrow */
            vtty_store (vtty, 6);
            break;
        case 0x44:             /* Left Arrow */
            vtty_store (vtty, 2);
            break;
        default:
            vtty_store (vtty, 0x5b);
            vtty_store (vtty, 0x1b);
            vtty_store (vtty, c);
            break;
        }
        vtty->input_state = VTTY_INPUT_TEXT;
        return;

    case VTTY_INPUT_REMOTE:
        //remote_control(vtty, c);
        vtty->input_state = VTTY_INPUT_TEXT;
        return;

    case VTTY_INPUT_TELNET:
        vtty->telnet_cmd = c;
        switch (c) {
        case WILL:
        case WONT:
        case DO:
        case DONT:
            vtty->input_state = VTTY_INPUT_TELNET_IYOU;
            return;
        case SB:
            vtty->telnet_cmd = c;
            vtty->input_state = VTTY_INPUT_TELNET_SB1;
            return;
        case SE:
            break;
        case IAC:
            vtty_store (vtty, IAC);
            break;
        }
        vtty->input_state = VTTY_INPUT_TEXT;
        return;

    case VTTY_INPUT_TELNET_IYOU:
        vtty->telnet_opt = c;
        /* if telnet client can support ttype, ask it to send ttype string */
        if ((vtty->telnet_cmd == WILL) && (vtty->telnet_opt == TELOPT_TTYPE)) {
            vtty_put_char (port, IAC);
            vtty_put_char (port, SB);
            vtty_put_char (port, TELOPT_TTYPE);
            vtty_put_char (port, TELQUAL_SEND);
            vtty_put_char (port, IAC);
            vtty_put_char (port, SE);
        }
        vtty->input_state = VTTY_INPUT_TEXT;
        return;

    case VTTY_INPUT_TELNET_SB1:
        vtty->telnet_opt = c;
        vtty->input_state = VTTY_INPUT_TELNET_SB2;
        return;

    case VTTY_INPUT_TELNET_SB2:
        vtty->telnet_qual = c;
        if ((vtty->telnet_opt == TELOPT_TTYPE)
            && (vtty->telnet_qual == TELQUAL_IS))
            vtty->input_state = VTTY_INPUT_TELNET_SB_TTYPE;
        else
            vtty->input_state = VTTY_INPUT_TELNET_NEXT;
        return;

    case VTTY_INPUT_TELNET_SB_TTYPE:
#if 0
        /* parse ttype string: first char is sufficient */
        /* if client is xterm or vt, set the title bar */
        if (c=='x' || c=='X' || c=='v' || c=='V') {
            fprintf(vtty->fstream, "\033]0;pic32sim %s\07", vtty->name);
        }
#endif
        vtty->input_state = VTTY_INPUT_TELNET_NEXT;
        return;

    case VTTY_INPUT_TELNET_NEXT:
        /* ignore all chars until next IAC */
        if (c == IAC)
            vtty->input_state = VTTY_INPUT_TELNET;
        return;
    }
}

int vtty_is_full (unsigned port)
{
    vtty_t *vtty = porttab + port;

    return (port < VTTY_NPORTS) && (vtty->read_ptr == vtty->write_ptr);
}

/*
 * Read a character from the buffer (-1 if the buffer is empty)
 */
int vtty_get_char (unsigned port)
{
    vtty_t *vtty = porttab + port;
    u_char c;

    if (port >= VTTY_NPORTS)
        return -1;
    VTTY_LOCK (vtty);

    if (vtty->read_ptr == vtty->write_ptr) {
        VTTY_UNLOCK (vtty);
        return (-1);
    }

    c = vtty->buffer[vtty->read_ptr++];

    if (vtty->read_ptr == VTTY_BUFFER_SIZE)
        vtty->read_ptr = 0;

    VTTY_UNLOCK (vtty);
    return (c);
}

/*
 * Returns TRUE if a character is available in buffer
 */
int vtty_is_char_avail (unsigned port)
{
    vtty_t *vtty = porttab + port;
    int res;

    if (port >= VTTY_NPORTS)
        return 0;
    VTTY_LOCK (vtty);
    res = (vtty->read_ptr != vtty->write_ptr);
    VTTY_UNLOCK (vtty);
    return (res);
}

/*
 * Put char to vtty
 */
void vtty_put_char (unsigned port, char ch)
{
    vtty_t *vtty = porttab + port;

    if (port >= VTTY_NPORTS)
        return;
    switch (vtty->type) {
    case VTTY_TYPE_TERM:
        fwrite (&ch, 1, 1, vtty->fstream);
        break;

    case VTTY_TYPE_TCP:
        if (vtty->state == VTTY_STATE_TCP_RUNNING &&
            fwrite (&ch, 1, 1, vtty->fstream) != 1)
        {
            fprintf (stderr, "%s: put char %#x failed (%s)\n",
                vtty->name, (unsigned char) ch, strerror (errno));
        }
        break;

    default:
        fprintf (stderr, "vtty_put_char(port = %u, ch = %#x): port not initialized\n",
            port, (unsigned char) ch);
        break;
    }
}

/*
 * VTTY thread
 */
static void *vtty_thread_main (void *arg)
{
    vtty_t *vtty;
    struct timeval tv;
    int fd, fd_max, res, port;
    fd_set rfds;

    for (;;) {
        /* Build the FD set */
        FD_ZERO (&rfds);
        fd_max = -1;
        for (port=0; port<VTTY_NPORTS; port++) {
            vtty = porttab + port;
            if (! vtty->select_fd)
                continue;

            fd = *vtty->select_fd;
            if (fd < 0)
                continue;

            if (fd > fd_max)
                fd_max = fd;
            FD_SET (fd, &rfds);
        }
        if (fd_max < 0) {
            /* No vttys created yet. */
            usleep (200000);
            continue;
        }

        /* Wait for incoming data */
        tv.tv_sec = 0;
        tv.tv_usec = 50 * 1000; /* 50 ms */
        res = select (fd_max + 1, &rfds, NULL, NULL, &tv);

        if (res == -1) {
            if (errno != EINTR) {
                perror ("vtty_thread: select");
                for (port=0; port<VTTY_NPORTS; port++) {
                    vtty = porttab + port;
                    if (vtty->name)
                        fprintf (stderr, "   %s: FD %d\n", vtty->name, vtty->fd);
                }
            }
            continue;
        }

        /* Examine active FDs and call user handlers */
        for (port=0; port<VTTY_NPORTS; port++) {
            vtty = porttab + port;
            if (! vtty->select_fd)
                continue;

            fd = *vtty->select_fd;
            if (fd < 0)
                continue;

            if (FD_ISSET (fd, &rfds)) {
                vtty_read_and_store (port);
            }

            /* Flush any pending output */
            if (vtty->fstream)
                fflush (vtty->fstream);
        }
    }
    return NULL;
}

/*
 * Initialize the VTTY thread
 */
void vtty_init (void)
{
    if (pthread_create (&vtty_thread, NULL, vtty_thread_main, NULL)) {
        perror ("vtty: pthread_create");
        exit (1);
    }
}
