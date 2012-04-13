#include "w5100.h"
#include "socket.h"

static uint16_t local_port;

/*
 * This Socket function initialize the channel in perticular mode,
 * and set the port and wait for W5100 done it.
 * Return 1 for success else 0.
 */
uint8_t socket (SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag)
{
    if ((protocol == SnMR_TCP) || (protocol == SnMR_UDP) ||
        (protocol == SnMR_IPRAW) || (protocol == SnMR_MACRAW) ||
        (protocol == SnMR_PPPOE))
    {
        close (s);
        w5100_writeSnMR (s, protocol | flag);
        if (port != 0) {
            w5100_writeSnPORT (s, port);
        } else {
            /* if don't set the source port, set local_port number. */
            local_port++;
            w5100_writeSnPORT (s, local_port);
        }

        w5100_execCmdSn (s, Sock_OPEN);
        return 1;
    }
    return 0;
}


/*
 * This function close the socket and parameter is "s" which represent
 * the socket number.
 */
void close (SOCKET s)
{
    w5100_execCmdSn (s, Sock_CLOSE);
    w5100_writeSnIR (s, 0xFF);
}

/*
 * This function established the connection for the channel in passive
 * (server) mode. This function waits for the request from the peer.
 * Return 1 for success else 0.
 */
uint8_t listen (SOCKET s)
{
    if (w5100_readSnSR (s) != SnSR_INIT)
        return 0;
    w5100_execCmdSn (s, Sock_LISTEN);
    return 1;
}

/*
 * This function established  the connection for the channel in Active
 * (client) mode.  This function waits for the untill the connection
 * is established.
 * Return 1 for success else 0.
 */
uint8_t connect (SOCKET s, uint8_t * addr, uint16_t port)
{
    if (((addr[0] == 0xFF) && (addr[1] == 0xFF) &&
         (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
        ((addr[0] == 0x00) && (addr[1] == 0x00) &&
         (addr[2] == 0x00) && (addr[3] == 0x00)) ||
        (port == 0))
      return 0;

    /* set destination IP */
    w5100_writeSnDIPR (s, addr);
    w5100_writeSnDPORT (s, port);
    w5100_execCmdSn (s, Sock_CONNECT);
    return 1;
}

/*
 * This function used for disconnect the socket and parameter is "s"
 * which represent the socket number
 * Return 1 for success else 0.
 */
void disconnect (SOCKET s)
{
    w5100_execCmdSn (s, Sock_DISCON);
}

/*
 * This function used to send the data in TCP mode
 * Return 1 for success else 0.
 */
uint16_t send (SOCKET s, const uint8_t * buf, uint16_t len)
{
    uint8_t status = 0;
    uint16_t ret = 0;
    uint16_t freesize = 0;

    if (len > w5100_SSIZE)
        ret = w5100_SSIZE; /* check size not to exceed MAX size. */
    else
        ret = len;

    /* if freebuf is available, start. */
    do {
        freesize = w5100_getTXFreeSize (s);
        status = w5100_readSnSR (s);
        if ((status != SnSR_ESTABLISHED) && (status != SnSR_CLOSE_WAIT)) {
            ret = 0;
            break;
        }
    } while (freesize < ret);

    /* copy data */
    w5100_send_data_processing (s, (uint8_t *)buf, ret);
    w5100_execCmdSn (s, Sock_SEND);

    /* +2008.01 bj */
    while ((w5100_readSnIR(s) & SnIR_SEND_OK) != SnIR_SEND_OK) {
        /* m2008.01 [bj] : reduce code */
        if (w5100_readSnSR(s) == SnSR_CLOSED) {
          close (s);
          return 0;
        }
    }
    /* +2008.01 bj */
    w5100_writeSnIR (s, SnIR_SEND_OK);
    return ret;
}

/*
 * This function is an application I/F function which is used to receive
 * the data in TCP mode.  It continues to wait for data as much as
 * the application wants to receive.
 * Return received data size for success else -1.
 */
uint16_t recv (SOCKET s, uint8_t *buf, uint16_t len)
{
    /* Check how much data is available */
    uint16_t ret = w5100_getRXReceivedSize(s);
    if (ret == 0) {
        /* No data available. */
        uint8_t status = w5100_readSnSR (s);

        if (s == SnSR_LISTEN || s == SnSR_CLOSED || s == SnSR_CLOSE_WAIT) {
            /* The remote end has closed its side of the connection,
             * so this is the eof state */
            ret = 0;
        } else {
            /* The connection is still up, but there's no data waiting
             * to be read */
            ret = -1;
        }
    } else if (ret > len) {
        ret = len;
    }

    if (ret > 0) {
        w5100_recv_data_processing (s, buf, ret);
        w5100_execCmdSn (s, Sock_RECV);
    }
    return ret;
}


/**
 * @brief	Returns the first byte in the receive queue (no checking)
 *
 * @return
 */
uint16_t peek(SOCKET s, uint8_t *buf)
{
  w5100_recv_data_processing(s, buf, 1, 1);

  return 1;
}


/**
 * @brief	This function is an application I/F function which is used to send the data for other then TCP mode.
 * 		Unlike TCP transmission, The peer's destination address and the port is needed.
 *
 * @return	This function return send data size for success else -1.
 */
uint16_t sendto(SOCKET s, const uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t port)
{
  uint16_t ret=0;

  /* check size not to exceed MAX size. */
  if (len > w5100_SSIZE) ret = w5100_SSIZE;
  else ret = len;

  if
    (
  ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
    ((port == 0x00)) ||(ret == 0)
    )
  {
    /* +2008.01 [bj] : added return value */
    ret = 0;
  }
  else
  {
    w5100_writeSnDIPR(s, addr);
    w5100_writeSnDPORT(s, port);

    /* copy data */
    w5100_send_data_processing(s, (uint8_t *)buf, ret);
    w5100_execCmdSn(s, Sock_SEND);

    /* +2008.01 bj */
    while ( (w5100_readSnIR(s) & SnIR_SEND_OK) != SnIR_SEND_OK )
    {
      if (w5100_readSnIR(s) & SnIR_TIMEOUT)
      {
        /* +2008.01 [bj]: clear interrupt */
        w5100_writeSnIR(s, (SnIR_SEND_OK | SnIR_TIMEOUT)); /* clear SEND_OK & TIMEOUT */
        return 0;
      }
    }

    /* +2008.01 bj */
    w5100_writeSnIR(s, SnIR_SEND_OK);
  }
  return ret;
}


/**
 * @brief	This function is an application I/F function which is used to receive the data in other then
 * 	TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well.
 *
 * @return	This function return received data size for success else -1.
 */
uint16_t recvfrom(SOCKET s, uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t *port)
{
  uint8_t head[8];
  uint16_t data_len=0;
  uint16_t ptr=0;

  if ( len > 0 )
  {
    ptr = w5100_readSnRX_RD(s);
    switch (w5100_readSnMR(s) & 0x07)
    {
    case SnMR_UDP :
/* removed unint8_t pointer cast in read_data due to pic32 pointer values are 32 bit and compiler errors out due to data loss from cast to 16-bit int -LSH */
      w5100_read_data(s, ptr, head, 0x08);
      ptr += 8;

      /* read peer's IP address, port number. */
      addr[0] = head[0];
      addr[1] = head[1];
      addr[2] = head[2];
      addr[3] = head[3];
      *port = head[4];
      *port = (*port << 8) + head[5];
      data_len = head[6];
      data_len = (data_len << 8) + head[7];
/* removed unint8_t pointer cast in read_data due to pic32 pointer values are 32 bit and compiler errors out due to data loss from cast to 16-bit int -LSH */
      w5100_read_data(s, ptr, buf, data_len); /* data copy. */
      ptr += data_len;

      w5100_writeSnRX_RD(s, ptr);
      break;

    case SnMR_IPRAW :
/* removed unint8_t pointer cast in read_data due to pic32 pointer values are 32 bit and compiler errors out due to data loss from cast to 16-bit int -LSH */
      w5100_read_data(s, ptr, head, 0x06);
      ptr += 6;

      addr[0] = head[0];
      addr[1] = head[1];
      addr[2] = head[2];
      addr[3] = head[3];
      data_len = head[4];
      data_len = (data_len << 8) + head[5];
/* removed unint8_t pointer cast in read_data due to pic32 pointer values are 32 bit and compiler errors out due to data loss from cast to 16-bit int -LSH */
      w5100_read_data(s, ptr, buf, data_len); /* data copy. */
      ptr += data_len;

      w5100_writeSnRX_RD(s, ptr);
      break;

    case SnMR_MACRAW:
/* removed unint8_t pointer cast in read_data due to pic32 pointer values are 32 bit and compiler errors out due to data loss from cast to 16-bit int -LSH */
      w5100_read_data(s, ptr,head,2);
      ptr+=2;
      data_len = head[0];
      data_len = (data_len<<8) + head[1] - 2;
/* removed unint8_t pointer cast in read_data due to pic32 pointer values are 32 bit and compiler errors out due to data loss from cast to 16-bit int -LSH */
      w5100_read_data(s, ptr,buf,data_len);
      ptr += data_len;
      w5100_writeSnRX_RD(s, ptr);
      break;

    default :
      break;
    }
    w5100_execCmdSn(s, Sock_RECV);
  }
  return data_len;
}


uint16_t igmpsend(SOCKET s, const uint8_t * buf, uint16_t len)
{
  uint8_t status=0;
  uint16_t ret=0;

  if (len > w5100_SSIZE)
    ret = w5100_SSIZE; /* check size not to exceed MAX size. */
  else
    ret = len;

  if (ret == 0)
    return 0;

  w5100_send_data_processing(s, (uint8_t *)buf, ret);
  w5100_execCmdSn(s, Sock_SEND);

  while ( (w5100_readSnIR(s) & SnIR_SEND_OK) != SnIR_SEND_OK )
  {
    status = w5100_readSnSR(s);
    if (w5100_readSnIR(s) & SnIR_TIMEOUT)
    {
      /* in case of igmp, if send fails, then socket closed */
      /* if you want change, remove this code. */
      close(s);
      return 0;
    }
  }

  w5100_writeSnIR(s, SnIR_SEND_OK);
  return ret;
}
