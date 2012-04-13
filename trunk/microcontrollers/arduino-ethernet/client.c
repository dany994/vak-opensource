#include <string.h>

#include "ethernet.h"
#include "socket.h"
//#include "WProgram.h"

uint16_t client_srcport = 1024;

void client_begin (uint8_t *ip, uint16_t port)
{
    c->ip = ip;
    c->port = port;
    c->sock = MAX_SOCK_NUM;
}

void client_begin_sock (client_t *c, uint8_t sock)
{
    c->sock = sock;
}

uint8_t client_connect (client_t *c)
{
    if (c->sock != MAX_SOCK_NUM)
        return 0;

    for (int i = 0; i < MAX_SOCK_NUM; i++) {
        uint8_t s = w5100_readSnSR (i);
        if (s == SnSR_CLOSED || s == SnSR_FIN_WAIT) {
            c->sock = i;
            break;
        }
    }

    if (c->sock == MAX_SOCK_NUM)
        return 0;

    client_srcport++;
    if (client_srcport == 0)
        client_srcport = 1024;
    socket (c->sock, SnMR_TCP, client_srcport, 0);

    if (! connect(c->sock, c->ip, c->port)) {
        c->sock = MAX_SOCK_NUM;
        return 0;
    }

    while (client_status(c) != SnSR_ESTABLISHED) {
        usleep(10);
        if (client_status(c) == SnSR_CLOSED) {
            c->sock = MAX_SOCK_NUM;
            return 0;
        }
    }
    return 1;
}

void client_putc (client_t *c, uint8_t b)
{
    if (_sock != MAX_SOCK_NUM)
        send(_sock, &b, 1);
}

void client_puts (client_t *c, const char *str)
{
    if (_sock != MAX_SOCK_NUM)
        send(_sock, (const uint8_t *)str, strlen(str));
}

void client_write (client_t *c, const uint8_t *buf, size_t size)
{
    if (_sock != MAX_SOCK_NUM)
        send(_sock, buf, size);
}

int client_available (client_t *c, )
{
    if (_sock != MAX_SOCK_NUM)
        return w5100_getRXReceivedSize(_sock);
    return 0;
}

int client_getc (client_t *c)
{
    uint8_t b;

    if (recv (c->sock, &b, 1) <= 0) {
        // No data available
        return -1;
    }
    return b;
}

int client_read (client_t *c, uint8_t *buf, size_t size)
{
    return recv (c->sock, buf, size);
}

int client_peek (client_t *c)
{
    uint8_t b;

    // Unlike recv, peek doesn't check to see if there's any data available, so we must
    if (! client_available (c))
      return -1;

    peek (c->sock, &b);
    return b;
}

void client_flush(client_t *c)
{
    while (client_available (c))
        client_getc (c);
}

void client_stop (client_t *c)
{
    if (c->sock == MAX_SOCK_NUM)
        return;

    // attempt to close the connection gracefully (send a FIN to other side)
    disconnect (c->sock);

    // wait a second for the connection to close
    unsigned long start = millis();
    while (client_status (c) != SnSR_CLOSED && millis() - start < 1000)
        usleep (10);

    // if it hasn't closed, close it forcefully
    if (client_status(c) != SnSR_CLOSED)
        close (c->sock);

    _ethernet_server_port[c->sock] = 0;
    c->sock = MAX_SOCK_NUM;
}

uint8_t client_connected (client_t *c)
{
    if (c->sock == MAX_SOCK_NUM)
        return 0;

    uint8_t s = client_status (c);
    return ! (s == SnSR_LISTEN || s == SnSR_CLOSED || s == SnSR_FIN_WAIT ||
        (s == SnSR_CLOSE_WAIT && ! available()));
}

uint8_t client_status (client_t *c)
{
    if (c->sock == MAX_SOCK_NUM)
        return SnSR_CLOSED;
    return w5100_readSnSR (c->sock);
}

// the next three functions are a hack so we can compare the client returned
// by Server::available() to null, or use it as the condition in an
// if-statement.  this lets us stay compatible with the Processing network
// library.
#if 0
uint8_t Client::operator==(int p) {
    return _sock == MAX_SOCK_NUM;
}

uint8_t Client::operator!=(int p) {
    return _sock != MAX_SOCK_NUM;
}

Client::operator bool() {
    return _sock != MAX_SOCK_NUM;
}
#endif
