/*
 * MRF24WG Universal Driver Power Control
 *
 * This module contains definitions for power control
*/
#ifndef __WF_POWER_H
#define __WF_POWER_H

enum {
    WF_LOW_POWER_MODE_OFF = 0,
    WF_LOW_POWER_MODE_ON  = 1,
};

void WFConfigureLowPowerMode(uint8_t action);
void EnsureWFisAwake(void);
bool isPsPollNeedReactivate(void);
void ClearPsPollReactivate(void);

#endif /* __WF_POWER_H */
