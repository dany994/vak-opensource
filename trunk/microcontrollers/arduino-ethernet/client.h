#ifndef client_h
#define client_h

struct _client_t {
    uint8_t sock;
    uint8_t *ip;
    uint16_t port;
};
typedef struct _client_t client_t;

extern uint16_t client_srcport;

void client_begin (client_t *c, uint8_t *ip, uint16_t port);
void client_begin_sock (client_t *c, uint8_t sock);

uint8_t client_status (client_t *);
uint8_t client_connect (client_t *);
void client_putc (client_t *, uint8_t);
void client_puts (client_t *c, const char *str);
void client_write (client_t *c, const uint8_t *buf, size_t size);
int client_available (client_t *);
int client_getc (client_t *);
int client_read (client_t *c, uint8_t *buf, size_t size);
int client_peek (client_t *);
void client_flush (client_t *);
void client_stop (client_t *);
uint8_t client_connected (client_t *);

//uint8_t operator==(int);
//uint8_t operator!=(int);
//operator bool();

#endif
