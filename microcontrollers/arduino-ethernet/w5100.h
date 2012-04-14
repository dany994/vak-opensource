/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * Updated  August/3/2011 by Lowell Scott Hanson to be compatable with chipKIT boards
 * Updated  April/13/2012 by Serge Vakulenko for RetroBSD project
 */
#ifndef	W5100_H_INCLUDED
#define	W5100_H_INCLUDED

//#include <SPI.h>
#include <stdint.h>

#define MAX_SOCK_NUM 4

#define IDM_OR          0x8000
#define IDM_AR0         0x8001
#define IDM_AR1         0x8002
#define IDM_DR          0x8003

#define SnMR_CLOSE      0x00
#define SnMR_TCP        0x01
#define SnMR_UDP        0x02
#define SnMR_IPRAW      0x03
#define SnMR_MACRAW     0x04
#define SnMR_PPPOE      0x05
#define SnMR_ND         0x20
#define SnMR_MULTI      0x80

#define Sock_OPEN       0x01
#define Sock_LISTEN     0x02
#define Sock_CONNECT    0x04
#define Sock_DISCON     0x08
#define Sock_CLOSE      0x10
#define Sock_SEND       0x20
#define Sock_SEND_MAC   0x21
#define Sock_SEND_KEEP  0x22
#define Sock_RECV       0x40

#define SnIR_SEND_OK    0x10
#define SnIR_TIMEOUT    0x08
#define SnIR_RECV       0x04
#define SnIR_DISCON     0x02
#define SnIR_CON        0x01

#define SnSR_CLOSED     0x00
#define SnSR_INIT       0x13
#define SnSR_LISTEN     0x14
#define SnSR_SYNSENT    0x15
#define SnSR_SYNRECV    0x16
#define SnSR_ESTABLISHED 0x17
#define SnSR_FIN_WAIT   0x18
#define SnSR_CLOSING    0x1A
#define SnSR_TIME_WAIT  0x1B
#define SnSR_CLOSE_WAIT 0x1C
#define SnSR_LAST_ACK   0x1D
#define SnSR_UDP        0x22
#define SnSR_IPRAW      0x32
#define SnSR_MACRAW     0x42
#define SnSR_PPPOE      0x5F

#define IPPROTO_IP      0
#define IPPROTO_ICMP    1
#define IPPROTO_IGMP    2
#define IPPROTO_GGP     3
#define IPPROTO_TCP     6
#define IPPROTO_PUP     12
#define IPPROTO_UDP     17
#define IPPROTO_IDP     22
#define IPPROTO_ND      77
#define IPPROTO_RAW     255

#define W5100_SSIZE     2048    // Max Tx buffer size
#define W5100_RSIZE     2048    // Max Rx buffer size

#define CH_BASE         0x0400
#define CH_SIZE         0x0100

typedef uint8_t socket_t;

class W5100Class {

public:
  void init();

  /**
   * @brief	This function is being used for copy the data form Receive buffer of the chip to application buffer.
   *
   * It calculate the actual physical address where one has to read
   * the data from Receive buffer. Here also take care of the condition while it exceed
   * the Rx memory uper-bound of socket.
   */
  /* Removed unint8_t pointer cast in read_data (2 argument) due to pic32 pointer values are 32 bit and
  pic 32 compiler errors out due to data loss from cast from 32-bit pointer to a 16-bit int. These changes are
  made to this method in sockets.cpp where it is used -LSH */
  void read_data(socket_t s, uint16_t src, volatile uint8_t * dst, uint16_t len);

  /**
   * @brief	 This function is being called by send() and sendto() function also.
   *
   * This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
   * register. User should read upper byte first and lower byte later to get proper value.
   */
  void send_data_processing(socket_t s, uint8_t *data, uint16_t len);

  /**
   * @brief	This function is being called by recv() also.
   *
   * This function read the Rx read pointer register
   * and after copy the data from receive buffer update the Rx write pointer register.
   * User should read upper byte first and lower byte later to get proper value.
   */
  void recv_data_processing(socket_t s, uint8_t *data, uint16_t len, uint8_t peek = 0);




  inline void setGatewayIp(uint8_t *_addr);
  inline void getGatewayIp(uint8_t *_addr);

  inline void setSubnetMask(uint8_t *_addr);
  inline void getSubnetMask(uint8_t *_addr);

  inline void setMACAddress(uint8_t * addr);
  inline void getMACAddress(uint8_t * addr);

  inline void setIPAddress(uint8_t * addr);
  inline void getIPAddress(uint8_t * addr);

  inline void setRetransmissionTime(uint16_t timeout);
  inline void setRetransmissionCount(uint8_t _retry);

  void execCmdSn(socket_t s, int cmd);

  uint16_t getTXFreeSize(socket_t s);
  uint16_t getRXReceivedSize(socket_t s);


  // W5100 Registers
  // ---------------
private:
  static uint8_t write(uint16_t _addr, uint8_t _data);
  static uint16_t write(uint16_t addr, uint8_t *buf, uint16_t len);
  static uint8_t read(uint16_t addr);
  static uint16_t read(uint16_t addr, uint8_t *buf, uint16_t len);

#define __GP_REGISTER8(name, address)             \
  static inline void write##name(uint8_t _data) { \
    write(address, _data);                        \
  }                                               \
  static inline uint8_t read##name() {            \
    return read(address);                         \
  }
#define __GP_REGISTER16(name, address)            \
  static void write##name(uint16_t _data) {       \
    write(address,   _data >> 8);                 \
    write(address+1, _data & 0xFF);               \
  }                                               \
  static uint16_t read##name() {                  \
    uint16_t res = read(address);                 \
    res = (res << 8) + read(address + 1);         \
    return res;                                   \
  }
#define __GP_REGISTER_N(name, address, size)      \
  static uint16_t write##name(uint8_t *_buff) {   \
    return write(address, _buff, size);           \
  }                                               \
  static uint16_t read##name(uint8_t *_buff) {    \
    return read(address, _buff, size);            \
  }

public:
  __GP_REGISTER8 (MR,     0x0000);    // Mode
  __GP_REGISTER_N(GAR,    0x0001, 4); // Gateway IP address
  __GP_REGISTER_N(SUBR,   0x0005, 4); // Subnet mask address
  __GP_REGISTER_N(SHAR,   0x0009, 6); // Source MAC address
  __GP_REGISTER_N(SIPR,   0x000F, 4); // Source IP address
  __GP_REGISTER8 (IR,     0x0015);    // Interrupt
  __GP_REGISTER8 (IMR,    0x0016);    // Interrupt Mask
  __GP_REGISTER16(RTR,    0x0017);    // Timeout address
  __GP_REGISTER8 (RCR,    0x0019);    // Retry count
  __GP_REGISTER8 (RMSR,   0x001A);    // Receive memory size
  __GP_REGISTER8 (TMSR,   0x001B);    // Transmit memory size
  __GP_REGISTER8 (PATR,   0x001C);    // Authentication type address in PPPoE mode
  __GP_REGISTER8 (PTIMER, 0x0028);    // PPP LCP Request Timer
  __GP_REGISTER8 (PMAGIC, 0x0029);    // PPP LCP Magic Number
  __GP_REGISTER_N(UIPR,   0x002A, 4); // Unreachable IP address in UDP mode
  __GP_REGISTER16(UPORT,  0x002E);    // Unreachable Port address in UDP mode

#undef __GP_REGISTER8
#undef __GP_REGISTER16
#undef __GP_REGISTER_N

  // W5100 Socket registers
  // ----------------------
private:
  static inline uint8_t readSn(socket_t _s, uint16_t _addr);
  static inline uint8_t writeSn(socket_t _s, uint16_t _addr, uint8_t _data);
  static inline uint16_t readSn(socket_t _s, uint16_t _addr, uint8_t *_buf, uint16_t len);
  static inline uint16_t writeSn(socket_t _s, uint16_t _addr, uint8_t *_buf, uint16_t len);

#define __SOCKET_REGISTER8(name, address)                    \
  static inline void write##name(socket_t _s, uint8_t _data) { \
    writeSn(_s, address, _data);                             \
  }                                                          \
  static inline uint8_t read##name(socket_t _s) {              \
    return readSn(_s, address);                              \
  }
#define __SOCKET_REGISTER16(name, address)                   \
  static void write##name(socket_t _s, uint16_t _data) {       \
    writeSn(_s, address,   _data >> 8);                      \
    writeSn(_s, address+1, _data & 0xFF);                    \
  }                                                          \
  static uint16_t read##name(socket_t _s) {                    \
    uint16_t res = readSn(_s, address);                      \
    res = (res << 8) + readSn(_s, address + 1);              \
    return res;                                              \
  }
#define __SOCKET_REGISTER_N(name, address, size)             \
  static uint16_t write##name(socket_t _s, uint8_t *_buff) {   \
    return writeSn(_s, address, _buff, size);                \
  }                                                          \
  static uint16_t read##name(socket_t _s, uint8_t *_buff) {    \
    return readSn(_s, address, _buff, size);                 \
  }

public:
  __SOCKET_REGISTER8(SnMR,        0x0000)        // Mode
  __SOCKET_REGISTER8(SnCR,        0x0001)        // Command
  __SOCKET_REGISTER8(SnIR,        0x0002)        // Interrupt
  __SOCKET_REGISTER8(SnSR,        0x0003)        // Status
  __SOCKET_REGISTER16(SnPORT,     0x0004)        // Source Port
  __SOCKET_REGISTER_N(SnDHAR,     0x0006, 6)     // Destination Hardw Addr
  __SOCKET_REGISTER_N(SnDIPR,     0x000C, 4)     // Destination IP Addr
  __SOCKET_REGISTER16(SnDPORT,    0x0010)        // Destination Port
  __SOCKET_REGISTER16(SnMSSR,     0x0012)        // Max Segment Size
  __SOCKET_REGISTER8(SnPROTO,     0x0014)        // Protocol in IP RAW Mode
  __SOCKET_REGISTER8(SnTOS,       0x0015)        // IP TOS
  __SOCKET_REGISTER8(SnTTL,       0x0016)        // IP TTL
  __SOCKET_REGISTER16(SnTX_FSR,   0x0020)        // TX Free Size
  __SOCKET_REGISTER16(SnTX_RD,    0x0022)        // TX Read Pointer
  __SOCKET_REGISTER16(SnTX_WR,    0x0024)        // TX Write Pointer
  __SOCKET_REGISTER16(SnRX_RSR,   0x0026)        // RX Free Size
  __SOCKET_REGISTER16(SnRX_RD,    0x0028)        // RX Read Pointer
  __SOCKET_REGISTER16(SnRX_WR,    0x002A)        // RX Write Pointer (supported?)

#undef __SOCKET_REGISTER8
#undef __SOCKET_REGISTER16
#undef __SOCKET_REGISTER_N

};

extern W5100Class W5100;

uint8_t W5100Class::readSn(socket_t _s, uint16_t _addr) {
  return read(CH_BASE + _s * CH_SIZE + _addr);
}

uint8_t W5100Class::writeSn(socket_t _s, uint16_t _addr, uint8_t _data) {
  return write(CH_BASE + _s * CH_SIZE + _addr, _data);
}

uint16_t W5100Class::readSn(socket_t _s, uint16_t _addr, uint8_t *_buf, uint16_t _len) {
  return read(CH_BASE + _s * CH_SIZE + _addr, _buf, _len);
}

uint16_t W5100Class::writeSn(socket_t _s, uint16_t _addr, uint8_t *_buf, uint16_t _len) {
  return write(CH_BASE + _s * CH_SIZE + _addr, _buf, _len);
}

void W5100Class::getGatewayIp(uint8_t *_addr) {
  readGAR(_addr);
}

void W5100Class::setGatewayIp(uint8_t *_addr) {
  writeGAR(_addr);
}

void W5100Class::getSubnetMask(uint8_t *_addr) {
  readSUBR(_addr);
}

void W5100Class::setSubnetMask(uint8_t *_addr) {
  writeSUBR(_addr);
}

void W5100Class::getMACAddress(uint8_t *_addr) {
  readSHAR(_addr);
}

void W5100Class::setMACAddress(uint8_t *_addr) {
  writeSHAR(_addr);
}

void W5100Class::getIPAddress(uint8_t *_addr) {
  readSIPR(_addr);
}

void W5100Class::setIPAddress(uint8_t *_addr) {
  writeSIPR(_addr);
}

void W5100Class::setRetransmissionTime(uint16_t _timeout) {
  writeRTR(_timeout);
}

void W5100Class::setRetransmissionCount(uint8_t _retry) {
  writeRCR(_retry);
}

#endif
