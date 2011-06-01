//
// Script #0: SCR_PROG_ENTRY, 39 bytes
//
fa              VPP_OFF
f7              MCLR_GND_ON
f9              VPP_PWM_ON
f5              BUSY_LED_ON
f3 00           SET_ICSP_PINS
e8 14           DELAY_LONG
f6              MCLR_GND_OFF
fb              VPP_ON
e7 17           DELAY_SHORT
fa              VPP_OFF
f7              MCLR_GND_ON
e7 2f           DELAY_SHORT
f2 b2           WRITE_BYTE_LITERAL
f2 c2           WRITE_BYTE_LITERAL
f2 12           WRITE_BYTE_LITERAL
f2 0a           WRITE_BYTE_LITERAL
f6              MCLR_GND_OFF
fb              VPP_ON
e7 eb           DELAY_SHORT
f3 02           SET_ICSP_PINS
bc 06 1f        JT2_SETMODE
bb 04           JT2_SENDCMD
bb 07           JT2_SENDCMD
ba 00           JT2_XFERDATA8_LIT

//
// Script #1: SCR_PROG_EXIT, 15 bytes
//
bc 05 1f        JT2_SETMODE
fa              VPP_OFF
f7              MCLR_GND_ON
f8              VPP_PWM_OFF
f3 06           SET_ICSP_PINS
f3 02           SET_ICSP_PINS
f3 03           SET_ICSP_PINS
e8 0a           DELAY_LONG
f4              BUSY_LED_OFF

//
// Script #2: SCR_RD_DEVID, 9 bytes
//
bb 04           JT2_SENDCMD
bb 01           JT2_SENDCMD
b9 00 00 00 00  JT2_XFERDATA32_LIT

//
// Script #3: SCR_PROGMEM_RD, 13 bytes
//
bb 0e           JT2_SENDCMD
b8 20 00 01 00  JT2_XFRFASTDAT_LIT
b7              JT2_XFRFASTDAT_BUF
b4              JT2_WAIT_PE_RESP
b5              JT2_GET_PE_RESP
e9 01 1f        LOOP

//
// Script #6: SCR_PROGMEM_WR_PREP, 6 bytes
//
bb 0e           JT2_SENDCMD
b7              JT2_XFRFASTDAT_BUF
e9 01 3f        LOOP

//
// Script #7: SCR_PROGMEM_WR, 5 bytes
//
b7              JT2_XFRFASTDAT_BUF
e9 01 3f        LOOP
b3              JT2_PE_PROG_RESP

//
// Script #22: SCR_ERASE_CHIP, 6 bytes
//
bb 07           JT2_SENDCMD
ba fc           JT2_XFERDATA8_LIT
e8 4a           DELAY_LONG
