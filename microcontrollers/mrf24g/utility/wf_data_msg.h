/*
 * MRF24WG Universal Driver Tx/Rx Data
 *
 * This module contains data Tx/Rx definitions
 */
#ifndef __WF_DATA_MSG_H
#define __WF_DATA_MSG_H

#define SNAP_VAL        ((uint8_t)0xaa)
#define SNAP_CTRL_VAL   ((uint8_t)0x03)
#define SNAP_TYPE_VAL   ((uint8_t)0x00)

#define SNAP_SIZE       (6)

void SignalPacketRx(void);
bool isPacketRx(void);
void ClearPacketRx(void);
void WF_ProcessWiFiRxData(void);
void RxPacketCheck(void);

#endif /* __WF_DATA_MSG_H */
