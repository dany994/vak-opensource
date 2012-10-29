/*
 * Display key events from Morse paddle.
 *
 * Usage:
 *      dump-paddle /dev/tty.usbserial-A1016UNH
 *
 * Copyright (C) 2012 Serge Vakulenko, <serge@vak.ru>
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this file and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING.txt" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "paddle.h"
#include "audio.h"

char *progname;
int wpm = 12;
int tone = 800;
volatile int daah_active, dit_active;
volatile int action;
volatile int idle_count = 6;

short dit_data [22000];
short daah_data [44000];
int dit_len;
int daah_len;

typedef struct {
    const int character;	/* The character represented */
    const char *representation; /* Dot-dash shape of the character */
} cw_entry_t;

static const cw_entry_t cw_table[] = {
    /* ASCII 7bit letters */
    { 'A', ".-"     }, { 'B', "-..."   }, { 'C', "-.-."   },
    { 'D', "-.."    }, { 'E', "."      }, { 'F', "..-."   },
    { 'G', "--."    }, { 'H', "...."   }, { 'I', ".."     },
    { 'J', ".---"   }, { 'K', "-.-"    }, { 'L', ".-.."   },
    { 'M', "--"     }, { 'N', "-."     }, { 'O', "---"    },
    { 'P', ".--."   }, { 'Q', "--.-"   }, { 'R', ".-."    },
    { 'S', "..."    }, { 'T', "-"      }, { 'U', "..-"    },
    { 'V', "...-"   }, { 'W', ".--"    }, { 'X', "-..-"   },
    { 'Y', "-.--"   }, { 'Z', "--.."   },

    /* Numerals */
    { '0', "-----"  }, { '1', ".----"  }, { '2', "..---"  },
    { '3', "...--"  }, { '4', "....-"  }, { '5', "....."  },
    { '6', "-...."  }, { '7', "--..."  }, { '8', "---.."  },
    { '9', "----."  },

    /* Punctuation */
    { '"', ".-..-." }, { '\'',".----." }, { '$', "...-..-"},
    { '(', "-.--."  }, { ')', "-.--.-" }, { '+', ".-.-."  },
    { ',', "--..--" }, { '-', "-....-" }, { '.', ".-.-.-" },
    { '/', "-..-."  }, { ':', "---..." }, { ';', "-.-.-." },
    { '=', "-...-"  }, { '?', "..--.." }, { '_', "..--.-" },
    { '@', ".--.-." },
};

static const int cw_table_size = sizeof(cw_table) / sizeof(cw_entry_t);

void decode (int element)
{
    static char word [16];
    static int nbits = 0;
    int i;

    if (element != ' ' && nbits < sizeof(word)-1) {
        /* Append element to the word. */
        word [nbits++] = element;
        printf ("%c", action);
        return;
    }
    if (nbits == 0)
        return;

    /* Compare with morse table. */
    word[nbits] = '\0';
    nbits = 0;
    for (i=0; i<cw_table_size; i++) {
        //printf ("comparing sign %d...\n", i);
        if (strcmp (cw_table[i].representation, word) == 0) {
            printf ("[%c]", cw_table[i].character);
            return;
        }
    }
    printf ("[??]");
}

/*
 * Fill the array with sinusoidal data
 */
static void fill_data (short *data, int len, int samples_per_cycle)
{
    int n;

    /* First cycle. */
    for (n=0; n<samples_per_cycle; ++n) {
        data[n] = 20000 * sin (2*M_PI*n / samples_per_cycle);
    }

    /* Next cycles. */
    for (; n<len; n+=samples_per_cycle) {
        memcpy (data + n, data, samples_per_cycle * sizeof(data[0]));
    }

    /* Make slow rising and falling edges. */
    int ncycles = 3 * samples_per_cycle;
    double phi = M_PI / ncycles;

    for (n=0; n<ncycles; ++n) {
        double factor = (1 - cos (n * phi)) / 2;

        data [n] = lround (data [n] * factor);
        data [len - 1 - n + ncycles] = lround (data [len - 1 - n] * factor);
    }
}

static void sigint (int sig)
{
    audio_stop();
    paddle_close();
    exit (-1);
}

void usage ()
{
    fprintf (stderr, "Simple Morse keyer.\n");
    fprintf (stderr, "Usage:\n\t%s [options...]\n", progname);
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "\t-w wpm\t\trate, default %d words per minute\n", wpm);
    fprintf (stderr, "\t-t tone\t\ttone frequency, default %d Hz\n", tone);
    fprintf (stderr, "\t-d\tdebug\n");
    exit (-1);
}

int main (int argc, char **argv)
{
    progname = *argv;
    for (;;) {
        switch (getopt (argc, argv, "dw:t:")) {
        case EOF:
            break;
        case 'w':
            wpm = strtol (optarg, 0, 0);
            continue;
        case 't':
            tone = strtol (optarg, 0, 0);
            continue;
        case 'd':
            ++paddle_debug;
            continue;
        default:
            usage ();
        }
        break;
    }
    argc -= optind;
    argv += optind;

    if (argc != 0 || wpm == 0)
        usage ();

    int sample_rate = audio_init();
    int samples_per_cycle = sample_rate / tone / 2 * 2;
    dit_len = sample_rate * 12 / wpm / 10 /
              samples_per_cycle * samples_per_cycle;
    daah_len = 3 * dit_len;
    fill_data (dit_data, dit_len, samples_per_cycle);
    fill_data (daah_data, daah_len, samples_per_cycle);

    paddle_open();
    atexit (paddle_close);
    signal (SIGINT, sigint);

    printf ("Keyer ready at %d wpm, tone %d Hz.\n", wpm, tone);
    audio_start();

    for (;;) {
        int daah = 0, dit = 0;
        paddle_poll (&daah, &dit);
        dit_active = dit;
        daah_active = daah;

        if (action) {
            decode (action);
            fflush (stdout);
            action = 0;
        }
        if (idle_count == 5) {
            idle_count++;
            printf (" ");
            fflush (stdout);
        }
    }
    audio_stop();
    paddle_close();
    return (0);
}

/*
 * Fill the data array with audio samples.
 * Return the number of samples.
 */
int audio_output (short *data, int maxdata)
{
    if (dit_active) {
        /* Play a dot and one inter-sign interval. */
        action = '.';
        idle_count = 0;
        memcpy (data, dit_data, (dit_len + dit_len) * sizeof(data[0]));
        return dit_len + dit_len;
    }
    if (daah_active) {
        action = '-';
        idle_count = 0;
        memcpy (data, daah_data, (daah_len + dit_len) * sizeof(data[0]));
        return daah_len + dit_len;
    }
    idle_count++;
    if (idle_count == 1) {
        action = ' ';
        idle_count++;
        memset (data, 0, (dit_len + dit_len) * sizeof(data[0]));
        return dit_len + dit_len;
    }
    memset (data, 0, dit_len * sizeof(data[0]));
    return dit_len;
}
