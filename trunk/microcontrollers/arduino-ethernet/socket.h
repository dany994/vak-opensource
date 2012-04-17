#ifndef	_SOCKET_H_
#define	_SOCKET_H_

#include "w5100.h"

/*
 * Opens a socket(TCP or UDP or IP_RAW mode)
 */
unsigned socket_init (socket_t s, unsigned protocol, unsigned port, unsigned flag);

/*
 * Close socket
 */
void socket_close (socket_t s);

/*
 * Establish TCP connection (Active connection)
 */
unsigned socket_connect (socket_t s, uint8_t *addr, unsigned port);

/*
 * disconnect the connection
 */
void socket_disconnect (socket_t s);

/*
 * Establish TCP connection (Passive connection)
 */
unsigned socket_listen (socket_t s);

/*
 * Send data (TCP)
 */
unsigned socket_send (socket_t s, const uint8_t *buf, unsigned len);

/*
 * Receive data (TCP)
 */
unsigned socket_recv (socket_t s, uint8_t *buf, unsigned len);
unsigned socket_peek (socket_t s, uint8_t *buf);

/*
 * Send data (UDP/IP RAW)
 */
unsigned socket_sendto (socket_t s, const uint8_t *buf, unsigned len, uint8_t *addr, unsigned port);

/*
 * Receive data (UDP/IP RAW)
 */
unsigned socket_recvfrom (socket_t s, uint8_t *buf, unsigned len, uint8_t *addr, unsigned *port);

unsigned socket_igmpsend (socket_t s, const uint8_t *buf, unsigned len);

#endif /* _SOCKET_H_ */
