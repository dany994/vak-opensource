#ifndef ethernet_h
#define ethernet_h

#include <stdint.h>
#include "client.h"
//#include "server.h"

#define MAX_SOCK_NUM 4

extern uint8_t _ethernet_state [MAX_SOCK_NUM];

extern uint16_t _ethernet_server_port [MAX_SOCK_NUM];

void ethernet_begin (uint8_t *mac, uint8_t *ip, uint8_t *gateway, uint8_t *subnet);

#endif
