/*
 * Interface to serial port.
 *
 * Copyright (C) 2015 Serge Vakulenko
 *
 * This file is distributed under the terms of the Apache License, Version 2.0.
 * See http://opensource.org/licenses/Apache-2.0 for details.
 */

/*
 * Open the serial port.
 * Return -1 on error.
 */
int serial_open (const char *devname, int baud_rate);

/*
 * Change the serial baud rate
 * Return -1 on error.
 */
int serial_baud (int baud_rate);

/*
 * Close the serial port.
 */
void serial_close (void);

/*
 * Send data to device.
 * Return number of bytes, or -1 on error.
 */
int serial_write (unsigned char *data, int len);

/*
 * Receive data from device.
 * Return number of bytes, or -1 on error.
 */
int serial_read (unsigned char *data, int len);

/*
 * Check whether the given speed in bits per second
 * is supported by the system.
 * Return 0 when not supported.
 */
int serial_speed_valid (int bps);
