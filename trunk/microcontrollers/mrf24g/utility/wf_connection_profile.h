/*
 * MRF24WG Universal Driver Connection Profile
 *
 * This module contains connection profile functions.
 */
#ifndef __WF_CONNECTION_PROFILE_H
#define __WF_CONNECTION_PROFILE_H

uint8_t GetCpid(void);

void SetHiddenSsid(bool hiddenSsid);
void SetAdHocMode(uint8_t mode);

t_wpaKeyInfo * GetWpsPassPhraseInfo(void);

#endif /* __WF_CONNECTION_PROFILE_H */
