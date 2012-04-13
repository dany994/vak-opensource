#ifndef	_SOCKET_H_
#define	_SOCKET_H_

#include "w5100.h"

// Opens a socket(TCP or UDP or IP_RAW mode)
extern uint8_t socket (socket_t s, uint8_t protocol, uint16_t port, uint8_t flag);

// Close socket
extern void close (socket_t s);

// Establish TCP connection (Active connection)
extern uint8_t connect (socket_t s, uint8_t * addr, uint16_t port);

// disconnect the connection
extern void disconnect (socket_t s);

// Establish TCP connection (Passive connection)
extern uint8_t listen (socket_t s);

// Send data (TCP)
extern uint16_t send (socket_t s, const uint8_t * buf, uint16_t len);

// Receive data (TCP)
extern uint16_t recv (socket_t s, uint8_t * buf, uint16_t len);
extern uint16_t peek (socket_t s, uint8_t *buf);

// Send data (UDP/IP RAW)
extern uint16_t sendto (socket_t s, const uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port);

// Receive data (UDP/IP RAW)
extern uint16_t recvfrom (socket_t s, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port);

extern uint16_t igmpsend (socket_t s, const uint8_t * buf, uint16_t len);

#endif /* _SOCKET_H_ */
