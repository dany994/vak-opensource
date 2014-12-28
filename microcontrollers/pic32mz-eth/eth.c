/*
 * Network driver for the PIC32 internal Ethernet controller with 8720 PHY.
 *
 * Author: Keith Vogel
 * Copyright (c) 2013-2014, Digilent <www.digilentinc.com>
 *
 * This program is free software; distributed under the terms of
 * BSD 3-clause license ("Revised BSD License", "New BSD License", or "Modified BSD License")
 */
#include "deIP.h"
#include "DEIPcK.h"

//
// MAC state.
//
#define STATE_IDLE                  0
#define STATE_INIT                  1
#define STATE_WAITLINK              2
#define STATE_WAITLINKINTERGAPTIME  3
#define STATE_LINKED                4
#define STATE_REST                  5

static int eth_state;                   // The state of the internal state machine
static int eth_is_up;                   // Used to determine if we need to set speed and duplex
static int eth_phy_id;                  // PHY id
static FFPT receive_queue;              // Queue of received packets
static FFPT transmit_queue;             // Queue of packet for transmit
static IPSTACK *eth_tx_packet;          // Current packet under transmit




#define VIRT_TO_PHYS(_v)            (((unsigned) (_v)) & 0x1FFFFFFF)
#define PHYS_TO_VIRT(_p)            ((void*) ((_p) | 0xA0000000))

#pragma pack(push,1)                    // we want to have control over this structure

typedef struct ETHDCPT_T {
    volatile union {
        volatile struct {
            unsigned                        : 7;
            unsigned        eOwn            : 1;
            unsigned        npv             : 1;
            unsigned                        : 7;
            unsigned        cbEDBuff        : 11;   // 1 - 2047 size of bytes this desciptor, set on trans, look on recv
            unsigned                        : 3;
            unsigned        eop             : 1;
            unsigned        sop             : 1;
        };
        uint32_t u32;
    } hdr;
    uint32_t                uEDBuff;        // ul-physical pointer to the data

    volatile union
    {
        // transmit values
        volatile struct TX_T
        {
            uint16_t            cbTransmitted;      // number of bytes trasmitted for this packet including collisions, could be a lot more than the packet size
            union
            {
                struct
                {
                    unsigned    controlFrame    : 1;
                    unsigned    pauseFrame      : 1;
                    unsigned    backPressure    : 1;
                    unsigned    vlanFrame       : 1;
                    unsigned                    : 4;
                };
                uint8_t         tsvHigh;
            };
            union
            {
                struct
                {
                    unsigned    user0           : 1;
                    unsigned    user1           : 1;
                    unsigned    user2           : 1;
                    unsigned    user3           : 1;
                    unsigned    user4           : 1;
                    unsigned    user5           : 1;
                    unsigned    user6           : 1;
                    unsigned    user7           : 1;
                };
                uint8_t         userFlags;
            };

            uint16_t            cbFrame;        // return value, how many byes in the frame were transmitted.
            union
            {
                struct
                {
                    unsigned    cCollision          : 4;
                    unsigned    crcError            : 1;
                    unsigned    lenCheckError       : 1;
                    unsigned    lenOutOfRange       : 1;
                    unsigned    done                : 1;
                    unsigned    multicast           : 1;
                    unsigned    broadcast           : 1;
                    unsigned    deferred            : 1;
                    unsigned    deferredExceeded    : 1;
                    unsigned    maxCollision        : 1;
                    unsigned    lateCollision       : 1;
                    unsigned    giantFrame          : 1;
                    unsigned    underRead           : 1;
                };
                uint16_t        tsv;            // transmit Status Vector
            };
        } tx;

        // RX Values
        volatile struct RX_T
        {
            uint16_t            checksum;
            union
            {
                struct
                {
                    unsigned    user0           : 1;
                    unsigned    user1           : 1;
                    unsigned    user2           : 1;
                    unsigned    user3           : 1;
                    unsigned    user4           : 1;
                    unsigned    user5           : 1;
                    unsigned    user6           : 1;
                    unsigned    user7           : 1;
                };
                uint8_t         userFlags;
            };
            union
            {
                struct
                {
                    unsigned    fRuntPkt        : 1;
                    unsigned    fNotUMCastMatch : 1;
                    unsigned    fHashMatch      : 1;
                    unsigned    fMagicPkt       : 1;
                    unsigned    fPatternMatch   : 1;
                    unsigned    fUnicastMatch   : 1;
                    unsigned    fBroadcast      : 1;
                    unsigned    fMulticastMatch : 1;
                };
                uint8_t         rxfRsv;         // Recieve Filter Status Vector
            };
            uint16_t            cbRcv;          // how many bytes came in on the frame
            union
            {
                struct
                {
                    unsigned    evLongDrop          : 1;
                    unsigned    evPrevSeen          : 1;
                    unsigned    evCarrierPrevSeen   : 1;
                    unsigned    codeViolation       : 1;
                    unsigned    crcError            : 1;
                    unsigned    lenCheckError       : 1;
                    unsigned    lenOutOfRange       : 1;
                    unsigned    ok                  : 1;
                    unsigned    multicast           : 1;
                    unsigned    broadcast           : 1;
                    unsigned    dribbleNibble       : 1;
                    unsigned    controlFrame        : 1;
                    unsigned    pauseFrame          : 1;
                    unsigned    unknownlFrame       : 1;
                    unsigned    vlanFrame           : 1;
                    unsigned                        : 1;
                };
                uint16_t        rsv;                // Receive Status Vector
            };
        } rx;
    };
//    uint32_t                pPhyDcptNext;           // physical address
} ETHDCPT;

// allocate static input buffer space
// flow control requires that we have 2*1536 at the high water mark
// therefore we must have at least 3 * 1536 bytes.
// but lets have a little more, say 4 * 1536 == 6144 == 6K
#define CBMAXETHPKT     (1536)
#define CMAXETHPKT      (4)
#define CBRXDCPTBUFF    (128)                           // This is tuned as a balance between the number of discriptors and size wasting empty space at the end of a discriptor
//#define IRXBUFSZ        (CBRXDCPTBUFF >> 4)           // this CBRXDCPTBUFF / 16; This is a magic number for the ethernet controller
#define CBRXBUFFTOTAL   (CMAXETHPKT * CBMAXETHPKT)      // how much buffer space we have for incoming frames
#define CRXDCPT         (CBRXBUFFTOTAL / CBRXDCPTBUFF)  // check that this works out to be even ie with a modulo of zero

#define INCR_RXDCPT_INDEX(_i) ((_i + 1) % CRXDCPT)

// on the transmit side we know what is coming at us...
// an IPStack has...
// 1:Frame, 2:IPHeader, 3:Transport hdr, and 4. payload.
// so to trasmitt we might need 4 descriptor and one to terminate == 5.
// now lets say we can handle 3 transmits at once...
// that is 15 descriptors.
#define CTXDCPPERIPSTACK    (5)

/*****************************************************************************/
/************************* Static Memory usage *******************************/
/*****************************************************************************/
static uint8_t rxBuffer[CBRXBUFFTOTAL];    // the rx buffer space
static ETHDCPT rxDcpt[CRXDCPT+1];          // the +1 is the terminating descriptor, or room for a loop back pointer
static ETHDCPT txDcpt[CTXDCPPERIPSTACK];   // just big enough for a transmit of an IPSTACK

static uint32_t receive_index = 0;         // The next descriptor to look at waiting for incoming data

// these are frame pointers into the current frame to read out of the DMA descriptor
static uint32_t read_index = 0;

static uint32_t obFmRead = 0;
static uint32_t cbFmFrame = 0;
static uint32_t cbFmRead = 0;

#pragma pack(pop)

static int32_t PHYReadReg(uint8_t phyID, uint8_t regID, uint32_t timeout)
{
    uint32_t tStart = SYSGetMilliSecond();

    // clear any commands
    EMAC1MCMD = 0;
    while(EMAC1MINDbits.MIIMBUSY)
    {
        // timeout error
        if (SYSGetMilliSecond() - tStart > timeout)
        {
            return(-1);
        }
    }

    // read the PHY status register
    // to see if we are stable.
    EMAC1MADR = PIC32_EMAC1MADR(phyID, regID);
    EMAC1MCMDbits.READ = 1;

    // wait to finish (this will execute our 3 cycles
    tStart = SYSGetMilliSecond();
    while(EMAC1MINDbits.MIIMBUSY)
    {
        // timeout error
        if (SYSGetMilliSecond() - tStart > timeout)
        {
            EMAC1MCMD = 0;
            return(-1);
        }
    }

    EMAC1MCMD = 0;

    // return the value
    return(EMAC1MRDDbits.MRDD);
}

static bool PHYScanReg(uint8_t phyID, uint8_t regID, uint16_t scanMask, bool fIsEqual, uint32_t timeout)
{
    uint32_t tStart = SYSGetMilliSecond();

    // clear any commands
    EMAC1MCMD = 0;
    while(EMAC1MINDbits.MIIMBUSY)
    {
        // timeout error
        if (SYSGetMilliSecond() - tStart > timeout)
        {
            return(false);
        }
    }

    // scan the PHY until it is ready
    EMAC1MADR = PIC32_EMAC1MADR(phyID, regID);
    EMAC1MCMDbits.SCAN      = 1;

    // wait for it to become valid
    tStart = SYSGetMilliSecond();
    while(EMAC1MINDbits.NOTVALID)
    {
        // timeout error
        if (SYSGetMilliSecond() - tStart > timeout)
        {
            return(false);
        }
    }

    // wait until we hit our mask
    tStart = SYSGetMilliSecond();

    while(((EMAC1MRDDbits.MRDD & scanMask) == scanMask) != fIsEqual)
    {
        // timeout error
        if (SYSGetMilliSecond() - tStart > timeout)
        {
            return(false);
        }
    }

    // kill the scan
    EMAC1MCMD = 0;
    tStart = SYSGetMilliSecond();
    while(EMAC1MINDbits.MIIMBUSY)
    {
        // timeout error
        if (SYSGetMilliSecond() - tStart > timeout)
        {
            return(false);
        }
    }
    return(true);
}

static bool PHYWriteReg(uint8_t phyID, uint8_t regID, uint16_t value, uint32_t timeout)
{
    uint32_t tStart = SYSGetMilliSecond();

    // clear any commands
    EMAC1MCMD = 0;
    while(EMAC1MINDbits.MIIMBUSY)
    {
        // timeout error
        if (SYSGetMilliSecond() - tStart > timeout)
        {
            return(false);
        }
    }

    EMAC1MADR = PIC32_EMAC1MADR(phyID, regID);
    EMAC1MWTDbits.MWTD      = value;

    // wait to finish (this will execute our 3 cycles
    tStart = SYSGetMilliSecond();
    while(EMAC1MINDbits.MIIMBUSY)
    {
        // timeout error
        if (SYSGetMilliSecond() - tStart > timeout)
        {
            return(false);
        }
    }
    return(true);
}

/*****************************************************************************/
/***************************** Reset the MAC *********************************/
/*****************************************************************************/
static void eth_reset_mac()
{
    // reset the MAC
    EMAC1CFG1bits.SOFTRESET = 1;

    // pull it out of reset
    EMAC1CFG1bits.SOFTRESET = 0;

    // believe this to be unneeded as SOFTRESET does it
    EMAC1CFG1bits.SIMRESET  = 0;
    EMAC1CFG1bits.RESETRMCS = 0;
    EMAC1CFG1bits.RESETRFUN = 0;
    EMAC1CFG1bits.RESETTMCS = 0;
    EMAC1CFG1bits.RESETTFUN = 0;

    // more configuration
    EMAC1CFG1bits.LOOPBACK  = 0;
    EMAC1CFG1bits.TXPAUSE   = 1;
    EMAC1CFG1bits.RXPAUSE   = 1;
    EMAC1CFG1bits.RXENABLE  = 1;
    //    EMAC1CFG1bits.PASSALL   = 0;  // don't understand this

    EMAC1CFG2bits.EXCESSDFR = 1;
    EMAC1CFG2bits.BPNOBKOFF = 1;
    EMAC1CFG2bits.NOBKOFF   = 0;
    EMAC1CFG2bits.LONGPRE   = 0;
    EMAC1CFG2bits.PUREPRE   = 0;
    EMAC1CFG2bits.AUTOPAD   = 1;
    EMAC1CFG2bits.VLANPAD   = 0;
    EMAC1CFG2bits.PADENABLE = 1;
    EMAC1CFG2bits.CRCENABLE = 1;
    EMAC1CFG2bits.DELAYCRC  = 0;
    EMAC1CFG2bits.HUGEFRM   = 0;
    EMAC1CFG2bits.LENGTHCK  = 1;
    //    EMAC1CFG2bits.FULLDPLX = ?;  // set in the is Linked function

    EMAC1IPGR = PIC32_EMAC1IPGR(12, 18);

    // this all default
    //    EMAC1CLRT = PIC32_EMAC1CLRT(55, 15);
    //    EMAC1MAXF = 1518;                     // max frame size in bytes
}

/*****************************************************************************/
/*******************************  RMII and MIIM reset  ***********************/
/*****************************************************************************/
static void eth_reset_mii()
{
    EMAC1SUPPbits.RESETRMII = 1;    // reset RMII
    EMAC1SUPPbits.RESETRMII = 0;

    // block reset the management protocol
    EMAC1MCFGbits.RESETMGMT = 1;    // reset the management fuctions
    EMAC1MCFGbits.RESETMGMT = 0;

    // As per table 35-3
    // SYSCLK / 4 == 0000 or 0001
    // SYSCLK / 6 == 0010
    // SYSCLK / 8 == 0011
    // SYSCLK / 10 == 0100
    // SYSCLK / 14 == 0101
    // SYSCLK / 20 == 0110
    // SYSCLK / 28 == 0111
    // SYSCLK / 40 == 1000
    // The IEEE 802.3 spec says no faster than 2.5MHz.
    // 80 / 40 = 2MHz; use 0b1000
    EMAC1MCFGbits.CLKSEL = 0b1000;
}

/*****************************************************************************/
/************************* Remotely reset the PHY via MIIM *******************/
/*****************************************************************************/
static bool eth_reset_phy(int phyID)
{
    uint16_t phyReg = 0;

    // if it is not see, lets attempt to set it
    if ((PHYReadReg(phyID, 18, 1000) & 0x000F) != phyID)
    {
        // maybe you want to do a hardware reset here
        // but not today

        // try zero, the most likely phy addres
        // but also an illegal one
        if (((phyReg = PHYReadReg(0, 18, 1000)) & 0x000F) == 0)
        {
            if (!PHYWriteReg(0, 18, ((phyReg & 0xFFF0) | phyID), 1000)) return(false);
        }
        else if (((phyReg = PHYReadReg(1, 18, 1000)) & 0x000F) == 1)
        {
            if (!PHYWriteReg(1, 18, ((phyReg & 0xFFF0) | phyID), 1000)) return(false);
        }
    }

    // now this should work
    if ((PHYReadReg(phyID, 18, 1000) & 0x000F) != phyID) return(false);

    // send a reset to the PHY
    if (!PHYWriteReg(phyID, 0, 0b1000000000000000, 1000)) return(false);

    // wait for the reset pin to autoclear, this says the reset is done
    if (!PHYScanReg(phyID, 0, 0b1000000000000000, false, 1000)) return(false);

    return(true);
}

static bool IsPHYLinked(uint8_t phyID, uint32_t timeout)
{
    bool fLinkIsUp = false;

    int32_t val = PHYReadReg(phyID, 1, timeout);

    if (val < 0)
    {
        return(false);
    }

    fLinkIsUp = ((val & 0x2C) == 0x2C);

    // set our link speed
    if (fLinkIsUp && !eth_is_up)
    {
        bool fFullDuplex    = false;
        bool f100Mbps       = false;
        bool fRXEN          = ETHCON1bits.RXEN;

        // must disable the Rx recieving while setting these parameters
        ETHCON1bits.RXEN    = 0;

        // get the speed
        val = PHYReadReg(phyID, 31, timeout);
        f100Mbps = ((val & 0x1008) == 0x1008);

        // get the duplex
        fFullDuplex = ((val & 0x0010) == 0x0010);

        // set speed
        EMAC1SUPPbits.SPEEDRMII = f100Mbps;

        // Section 35-25; set duplex
        EMAC1CFG2bits.FULLDPLX  = fFullDuplex;

        // set Gap size
        EMAC1IPGTbits.B2BIPKTGP = fFullDuplex ? 0x15 : 0x12;

        // return the Rx Enable back to what it was
        ETHCON1bits.RXEN    = fRXEN;
    }
    eth_is_up = fLinkIsUp;

    return(fLinkIsUp);
}

/*****************************************************************************/
/***************************** Start of Adaptor functions ********************/
/*****************************************************************************/

static void IntermalMACStateMachine(void)
{
    static uint32_t tStart = 0;

    switch(eth_state) {
    // we haven't even initialized yet.
    case STATE_IDLE:
        break;

    case STATE_INIT:
        break;

    case STATE_WAITLINK:
        if (IsPHYLinked(eth_phy_id, 1)) {
            tStart  = SYSGetMilliSecond();
            eth_state = STATE_WAITLINKINTERGAPTIME;
        }
        break;

    case STATE_WAITLINKINTERGAPTIME:
        if (! IsPHYLinked(eth_phy_id, 1)) {
            eth_state = STATE_WAITLINK;
        }
        else if (SYSGetMilliSecond() - tStart >= 1000) {
            eth_state = STATE_LINKED;
            ETHCON1bits.RXEN = 1;
        }
        break;

    case STATE_REST:
        if (SYSGetMilliSecond() - tStart >= 10) {
            if (IsPHYLinked(eth_phy_id, 1)) {
                eth_state = STATE_LINKED;
            } else {
                tStart  = SYSGetMilliSecond();
            }
        }
        break;

    case STATE_LINKED:
    default:
        if (! IsPHYLinked(eth_phy_id, 1)) {
            eth_state = STATE_REST;
            tStart  = SYSGetMilliSecond();
        }
        break;
    }
}

static bool IsFrameWaitingForRead(void)
{
    if (eth_state < STATE_WAITLINK) {
        return(false);
    }

    // There are no packets waiting
    else if (ETHSTATbits.BUFCNT == 0)
    {
        // update to what the DMA controller thinks
        // this should not be needed, but lets sync anyway
        // if we find this is never called we can remove this.
        // if it is called, we need to understand why
        // tests look good, this is never hit...

        // there is a timing window here
        // if between the BUFCNT == 0 above and here a packet comes in
        // and DMA occurs, the ETHRXST pointer will be updated while
        // seemingly BUFCNT == 0. So check the BUFCNT again after the pointer test
        if (ETHRXST != VIRT_TO_PHYS(&rxDcpt[receive_index]) && ETHSTATbits.BUFCNT == 0)
        {
            receive_index = (ETHRXST - VIRT_TO_PHYS(&rxDcpt[0])) / sizeof(ETHDCPT);
        }

        return(false);
    }

    // I better own this, or I am lost; this should not happen if BUFCNT > 0
    // I should own it! Not the DMA
    // this looks good and never seems to be hit
    else if (rxDcpt[receive_index].hdr.eOwn)
    {
        uint32_t i = 0;

        // lets find where we are suppose to be; this is a bug if we get here
        // as we should stay in sync with the DMA usage
        // this looks good too, we never hit this
        for(i=0; i<CRXDCPT; i++)
        {
            // if I own it, get out of the loop
            if (!rxDcpt[receive_index].hdr.eOwn)
            {
                break;
            }

            // go look at the next one to see if I own it
            receive_index = INCR_RXDCPT_INDEX(receive_index);
        }
    }

    // we should be at a SOP, if not we are lost
    // just drop the packets until we get to a start of frame
    // again, a bug if we get lost
    // looks good, does not see to be hit...
    if (!rxDcpt[receive_index].hdr.sop)
    {
        // if we own it, and it is not a start of frame; give it back to the DMA
        while(!rxDcpt[receive_index].hdr.eOwn && !rxDcpt[receive_index].hdr.sop)
        {
            // give it back to the DMA
            rxDcpt[receive_index].hdr.eOwn = true;
            ETHCON1bits.BUFCDEC = 1;                        // dec the BUFCNT
            receive_index = INCR_RXDCPT_INDEX(receive_index);
        }
    }

    // if I own it, and it is the start of packet, we have something
    return(!rxDcpt[receive_index].hdr.eOwn && rxDcpt[receive_index].hdr.sop);
}

static uint32_t read_packet(uint8_t *pBuff, uint32_t cbBuff)
{
    uint32_t cbCopy = 0;

    // we are not reading a frame
    if (cbFmFrame == 0 || cbFmRead == cbFmFrame)
    {
        return(0);
    }

    // make sure we own the descriptor, bad if we don't!
    while (! rxDcpt[read_index].hdr.eOwn && cbBuff > 0)
    {
        uint32_t    cbDcpt  = rxDcpt[read_index].hdr.cbEDBuff;
        uint32_t    cb      = min(cbDcpt - obFmRead, cbBuff);
        bool        fEOP    = rxDcpt[read_index].hdr.eop;

        memcpy(&pBuff[cbCopy], PHYS_TO_VIRT(rxDcpt[read_index].uEDBuff + obFmRead), cb);
        cbCopy      += cb;
        obFmRead    += cb;
        cbFmRead    += cb;
        cbBuff      -= cb;

        // if we read the whole descriptor page
        if (obFmRead == cbDcpt)
        {
            // set up for the next page
            obFmRead = 0;
            read_index = INCR_RXDCPT_INDEX(read_index);

            // if we are done, get out
            if (fEOP || cbFmRead == cbFmFrame)
            {
                break;
            }
        }
    }

    return(cbCopy);
}

static bool FreeFrame(void)
{
    // if we never called get size, or there is nothing to free
    if (cbFmFrame == 0)
    {
        return(false);
    }

    while(!rxDcpt[receive_index].hdr.eOwn)
    {
        bool fEOP = rxDcpt[receive_index].hdr.eop;

        rxDcpt[receive_index].hdr.eOwn = true;          // give up ownership
        ETHCON1bits.BUFCDEC = 1;                        // dec the BUFCNT
        receive_index = INCR_RXDCPT_INDEX(receive_index);  // check the next one

        // hit the end of packet
        if (fEOP)
            break;
    }

    // init our state variables
    read_index = 0;
    obFmRead = 0;
    cbFmFrame = 0;
    cbFmRead = 0;
    return(true);
}

/*****************************************************************************/
/***************************** Adaptor code  *********************************/
/*****************************************************************************/

int eth_is_linked()
{
    return eth_state == STATE_LINKED;
}

static IPSTACK *eth_read()
{
    IPSTACK *packet = get_packet(&receive_queue);

    return packet;
}

void eth_send(IPSTACK *packet)
{
    put_packet(&transmit_queue, packet);
}

static bool SendNextIpStack(void)
{
    if (!ETHCON1bits.TXRTS)
    {
        if (eth_tx_packet != 0){
            IPSRelease(eth_tx_packet);
            eth_tx_packet = 0;
        }

        // if we can send right now
        // is the adaptor up and do we have the transmit bit?
        if (eth_is_linked(NULL))
        {
            IPSTACK *   packet    = get_packet(&transmit_queue);
            int32_t     i           = 0;
            int32_t     j           = 0;

            if (packet != NULL)
            {
                // clear the tx buffer, but we don't have to
                // worry about the last one as it is just
                // a dummy that will always have the software
                // own to stop the DMA from running off the end
                memset(txDcpt, 0, sizeof(txDcpt) - sizeof(ETHDCPT));

                // always have a frame, alwasy FRAME II (we don't support 802.3 outgoing frames; this is typical)
                txDcpt[i].hdr.cbEDBuff = packet->cbFrame;
                txDcpt[i].uEDBuff = VIRT_TO_PHYS(packet->pFrameII);
                txDcpt[i].hdr.sop = 1;
                i++;

                // IP Header
                if (packet->cbIPHeader > 0)
                {
                    txDcpt[i].hdr.cbEDBuff = packet->cbIPHeader;
                    txDcpt[i].uEDBuff = VIRT_TO_PHYS(packet->pIPHeader);
                    i++;
                }

                // Transport Header (TCP/UDP)
                if (packet->cbTranportHeader > 0)
                {
                    txDcpt[i].hdr.cbEDBuff = packet->cbTranportHeader;
                    txDcpt[i].uEDBuff = VIRT_TO_PHYS(packet->pTransportHeader);
                    i++;
                }

                // Payload / ARP / ICMP
                if (packet->cbPayload > 0)
                {
                    txDcpt[i].hdr.cbEDBuff = packet->cbPayload;
                    txDcpt[i].uEDBuff = VIRT_TO_PHYS(packet->pPayload);
                    i++;
                }

                // put in eop; I is one past the last
                // entry; lets bump it down one
                i--;
                txDcpt[i].hdr.eop = 1;

                // set the ownership bits
                // the last descriptor is a dummy the software
                // always owns, do look at that.
                for(j = (CTXDCPPERIPSTACK - 1); j>=0; j--)
                {
                    // i is the last descriptor that is
                    // go to be transmitted
                    txDcpt[j].hdr.eOwn = (j <= i);
                }

                // set the descriptor table to be transmitted
                ETHTXST = VIRT_TO_PHYS(txDcpt);
                // transmit
                ETHCON1bits.TXRTS = 1;

                eth_tx_packet = packet;
            }
            return(true);
        }
    }
    return(false);
}

//
// Process the next received packet.
//
static void receive_packet()
{
    // this is the size of the frame as the Eth controller knows it
    // this may include the FCS at the end of the frame
    // and may included padding bytes to make a min packet size of 64 bytes
    // so it is likely this is longer than the payload length
    read_index = receive_index;
    cbFmFrame = rxDcpt[receive_index].rx.cbRcv;
    obFmRead = 0;
    cbFmRead = 0;

    if (cbFmFrame <= 0)
        return;

    IPSTACK *packet = Malloc(cbFmFrame + sizeof(IPSTACK));

    if (packet == NULL) {
        // Out of memory.
    } else {
        // Fill in info about the frame data.
        packet->pPayload  = ((uint8_t *) packet) + sizeof(IPSTACK);
        packet->cbPayload = cbFmFrame;

        read_packet(packet->pPayload, packet->cbPayload);
        put_packet(&receive_queue, packet);
    }

    // Always free the DMA.
    FreeFrame();
}

void eth_internal_periodic_tasks()
{
    IntermalMACStateMachine();

    // transmit any pending data
    SendNextIpStack();

// POTENTIALLY THIS COULD GO IN AN ISR
    if (IsFrameWaitingForRead()) {
        receive_packet();
    }
// END OF ISR
}

//
// Set DMA descriptors.
//
static void eth_init_dma()
{
    // set Rx discriptor list
    // all owned by the ethernet controller / DMA
    memset(rxDcpt, 0, sizeof(rxDcpt));
    for(i=0; i< CRXDCPT; i++)
    {
        rxDcpt[i].hdr.eOwn = 1;
        rxDcpt[i].hdr.npv   = 0;  // 0 = next in memory, 1 = use ED field
        rxDcpt[i].uEDBuff = VIRT_TO_PHYS(rxBuffer + (i * CBRXDCPTBUFF));
    }

    // loop the list back to the begining
    // this is a circular array descriptor list
    *((uint32_t *) (&rxDcpt[CRXDCPT])) = VIRT_TO_PHYS(rxDcpt);
    rxDcpt[CRXDCPT-1].hdr.npv   = 1;  // 0 = next in memory, 1 = use ED field

    // set us at the start of the list
    receive_index = 0;
    ETHRXST = VIRT_TO_PHYS(&rxDcpt[0]);

    // set up the transmitt descriptors all owned by
    // the software; clear it completely out
    memset(txDcpt, 0, sizeof(txDcpt));
    ETHTXST = VIRT_TO_PHYS(txDcpt);

    // init our frame reading values
    // used by read_packet
    read_index = 0;
    obFmRead = 0;
    cbFmFrame = 0;
    cbFmRead = 0;
}

//
// Reset the Ethernet Controller.
//
static void eth_reset()
{
    // Disable the ethernet interrupt.
    IECCLR(PIC32_IRQ_ETH >> 5) = 1 << (PIC32_IRQ_ETH & 31);

    // Turn the Ethernet cotroller OFF
    ETHCON1bits.ON = 0;    // THIS TURNS THE CONTROLLER OFF!
    ETHCON1bits.RXEN = 0;    // Rx enable; set to OFF
    ETHCON1bits.TXRTS = 0;    // Rx have something to send; set to OFF

    // wait for abort to finish
    while(ETHSTATbits.ETHBUSY)
        continue;

    // Clear the interrupt flag bit.
    IFSCLR(PIC32_IRQ_ETH >> 5) = 1 << (PIC32_IRQ_ETH & 31);

    // clear individual interrupt bits
    ETHIENCLR = 0x000063ef;
    ETHIRQCLR = 0x000063ef;

    // clear discriptor pointers; for now
    ETHTXST = 0;
    ETHRXST = 0;

    //
    // Init flow control.
    // Init RX filtering.
    //

    // manual flow control is OFF
    ETHCON1bits.MANFC = 0;

    // WE MAY WITH TO TURN FLOW CONTROL OFF! I can block the network for others
    // auto flow control is on
    ETHCON1bits.PTV = 1;    // the max number of pause timeouts
    ETHCON1bits.AUTOFC = 1;

    // High and low watermarks.
    int empty_watermark = CBMAXETHPKT / CBRXDCPTBUFF;
    int full_watermark  = CRXDCPT - (CBMAXETHPKT * 2) / CBRXDCPTBUFF;
    ETHRXWM = PIC32_ETHRXWM_FWM(full_watermark) |
              PIC32_ETHRXWM_EWM(empty_watermark);

    // set buffer size, descriptor buffer size in bytes / 16
    // got to do this with the ethernet controller OFF, so can't wait to
    // this with the
    ETHCON2bits.RXBUF_SZ = CBRXDCPTBUFF >> 4;

    // clear everything, and then set our Rx filters
    ETHRXFCCLR = 0x0000DFFF;
    ETHRXFCbits.CRCOKEN = 1; // CRC must checkout
    ETHRXFCbits.UCEN = 1; // our MAC address will match
    ETHRXFCbits.BCEN = 1; // match on broadcast

    // hash table; not using
    ETHHT0CLR = 0xFFFFFFFF;
    ETHHT1CLR = 0xFFFFFFFF;

    // pattern match, not used
    ETHPMM1CLR = 0xFFFFFFFF;
    ETHPMM1CLR = 0xFFFFFFFF;

    // byte in TCP like checksum pattern calculation.
    ETHPMCSbits.PMCS = 0;

    // turn on the ethernet controller
    // this is a point of inescapable dispute with
    // documentation. The docs say don't do this yet
    // but if you done, you can't talk to the MIIM management functions
    // the controller has to be running to talk to the PHY
    ETHCON1bits.ON = 1;
}

void eth_get_macaddr(char *mac_addr)
{
    // Fetch our MAC address.
    mac_addr[0] = EMAC1SA2;
    mac_addr[1] = EMAC1SA2 >> 8;
    mac_addr[2] = EMAC1SA1;
    mac_addr[3] = EMAC1SA1 >> 8;
    mac_addr[4] = EMAC1SA0;
    mac_addr[5] = EMAC1SA0 >> 8;
}

void eth_init()
{
    int i;

    // Setup for PIC32MZ EC Starter Kit board.
    TRISHSET = 1<<8;                    // Set RH8 as input - ERXD0
    TRISHSET = 1<<5;                    // Set RH5 as input - ERXD1
    TRISHSET = 1<<4;                    // Set RH4 as input - ERXERR

    // Default PHY address is 0 on LAN8720 PHY daughter board.
    eth_phy_id = 0;

    // Link is down.
    eth_is_up = 0;

    // Keep the internal MAC state machine at idle
    eth_state = STATE_IDLE;

    // Clear packet queues.
    eth_tx_packet = 0;
    memset(&receive_queue, 0, sizeof(receive_queue));
    memset(&transmit_queue, 0, sizeof(transmit_queue));

    // As per section 35.4.10 of the Pic32 Family Ref Manual
    eth_reset();
    eth_reset_mac();
    eth_reset_mii();

    if (! eth_reset_mii(eth_phy_id)) {
        printf("Ethernet configuration failed\n");
        return;
    }

    // Set DMA descriptors.
    eth_init_dma();

    // Wait for auto link to finish.
    eth_state = STATE_WAITLINK;
}
