/*
 * MRF24WG Universal Driver Parameter Messages
 *
 * This module contains parameter message defintions
 */
#ifndef __WF_PARAM_MSG_H
#define __WF_PARAM_MSG_H

#define MSG_PARAM_START_DATA_INDEX          (6)
#define MULTICAST_ADDRESS                   (6)
#define ADDRESS_FILTER_DEACTIVATE           (0)

#define ENABLE_MRF24WB0M                    (1)

void WFEnableMRF24WB0MMode(void);
uint8_t GetFactoryMax(void);
void YieldPassPhraseToHost(void);
void SetPSK(uint8_t *psk);

#endif /* __WF_PARAM_MSG_H */
