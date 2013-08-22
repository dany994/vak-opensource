/*
 * Clone Utility for Baofeng radios.
 *
 * Copyright (C) 2013 Serge Vakulenko, KK6ABQ
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. The name of the author may not be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <sys/stat.h>
#include "radio.h"

const char version[] = "1.0";
const char copyright[] = "Copyright (C) 2013 Serge Vakulenko KK6ABQ";

extern char *optarg;
extern int optind;

void usage ()
{
    fprintf (stderr, "Baofeng UV-5R Clone Utility, Version %s, %s\n", version, copyright);
    fprintf (stderr, "Usage:\n");
    fprintf (stderr, "    baoclone [-v] port                Save device binary image to file 'device.img',\n");
    fprintf (stderr, "                                      and text configuration to 'device.cfg'.\n");
    fprintf (stderr, "    baoclone -w [-v] port file.img    Write image to device.\n");
    fprintf (stderr, "    baoclone -c [-v] port file.cfg    Configure device from text file.\n");
    fprintf (stderr, "    baoclone file.img                 Show configuration from image file.\n");
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "    -v                                Verbose mode.\n");
    exit (-1);
}

//
// Check for a regular file.
//
int is_file (char *filename)
{
    struct stat st;

    if (stat (filename, &st) < 0) {
        // File not exist: treat it as a regular file.
        return 1;
    }
    return (st.st_mode & S_IFMT) == S_IFREG;
}

int main (int argc, char **argv)
{
    int verbose = 0, write_flag = 0, config_flag = 0;

    for (;;) {
        switch (getopt (argc, argv, "vcw")) {
        case 'v': ++verbose;     continue;
        case 'w': ++write_flag;  continue;
        case 'c': ++config_flag; continue;
        default:
            usage ();
        case EOF:
            break;
        }
        break;
    }
    argc -= optind;
    argv += optind;
    if (write_flag + config_flag > 1) {
        fprintf (stderr, "Only one of -w or -c options is allowed.\n");
        usage();
    }

    if (write_flag) {
        // Restore image file to device.
        if (argc != 2)
            usage();

        radio_connect (argv[0], verbose);
        radio_load_image (argv[1]);
        radio_print_version (stdout);
        radio_upload();
        radio_disconnect();

    } else if (config_flag) {
        // Update device from text config file.
        if (argc != 2)
            usage();

        radio_connect (argv[0], verbose);
        radio_download();
        radio_print_version (stdout);
        radio_save_image ("save.img");
        radio_parse_config (argv[1]);
        radio_upload();
        radio_disconnect();

    } else {
        if (argc != 1)
            usage();

        if (is_file (argv[0])) {
            // Print configuration from image file.
            // Load image from file.
            radio_load_image (argv[0]);
            radio_print_version (stdout);
            radio_print_config (stdout);

        } else {
            // Dump device to image file.
            radio_connect (argv[0], verbose);
            radio_download();
            radio_print_version (stdout);
            radio_disconnect();
            radio_save_image ("device.img");

            // Print configuration to file.
            const char *filename = "device.cfg";
            printf ("Print configuration to file '%s'.\n", filename);
            FILE *cfg = fopen (filename, "w");
            if (! cfg) {
                perror (filename);
                exit (-1);
            }
            radio_print_version (cfg);
            radio_print_config (cfg);
            fclose (cfg);
        }
    }
    return (0);
}
