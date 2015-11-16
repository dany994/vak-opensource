# Pickit 2 adapter #
![http://www.voti.nl/pickit2faq/files/pickit2.jpg](http://www.voti.nl/pickit2faq/files/pickit2.jpg)

The PICkit 2 is an USB in-circuit programmer manufactured by [| http://www.voti.nl/pickit2faq/index.html](Microchip.md).

# Commands #

| **Command**              | **Encoding** | **Response**              | **Action**   |
|:-------------------------|:-------------|:--------------------------|:-------------|
| No Operation             | 0x5A         | --                        | Do nothing   |
| Get Version              | 0x76         | {major} {minor} {dot}     | Get firmware version |
| Boot Mode                | 0x42         | --                        | Enter Bootloader mode |
| Set VDD                  | 0xA0 {CCPL} {CCPH} {VDDLim} | --                        |              |
| Set VPP                  | 0xA1 {CCPR2L} {VPPADC} {VPPLim} | --                        |              |
| Read Status              | 0xA2         | {StsL} {StsH}             |              |
| Read Voltages            | 0xA3         | {VddL} {VddH} {VppL} {VppH} |              |
| Download Script          | 0xA4 {Script#} {ScriptLengthN} {Script1} {Script2} ... {ScriptN} | --                        | Store a script in the Script Buffer |
| Run Script               | 0xA5 {Script#} {iterations} | --                        | Run a script from the script buffer |
| Execute Script           | 0xA6 {ScriptLengthN} {Script1} {Script2} ... {ScriptN} | Immediately execute the included script |
| Clear Download Buffer    | 0xA7         | --                        | Empty the download buffer |
| Download Data            | 0xA8 {DataLength} {Data1} {Data2} ... {DataN} | --                        | Add data to download buffer |
| Clear Upload Buffer      | 0xA9         | --                        | Empty the upload buffer |
| Upload Data              | 0xAA         | {DataLengthN} {data1} {data2} ... {dataN} | Read data from upload buffer |
| Clear Script Buffer      | 0xAB         | --                        |              |
| Upload Data Nolen        | 0xAC         | {Data1} {Data2} ... {DataN} | Read data from upload buffer |
| End Of Buffer            | 0xAD         | --                        | Skip the rest of commands |
| Reset                    | 0xAE         | --                        | Reset        |
| Script Buffer Checksum   | 0xAF         | {LenSumL} {LenSumH} {BufSumL} {BufSumH} | Calculate checksums of the Script Buffer |
| Set Voltage Calibrations | 0xB0 {adc\_calfactorL} {adc\_calfactorH} {vdd\_offset} {calfactor} | --                        |              |
| Write Internal EEPROM    | 0xB1 {address} {datalength} {data1} {data2} ... {dataN} | --                        | Write data to PIC18F2550 EEPROM |
| Read Internal EEPROM     | 0xB2 {address} {datalength} | {data1} {data2} ... {dataN} | Read bytes from PIC18F2550 EEPROM |
| Enter UART Mode          | 0xB3         | --                        |              |
| Exit UART Mode           | 0xB4         | --                        | Exits the firmware from UART Mode |
| Enter Learn Mode         | 0xB5 {0x50} {0x4B} {0x32} {EEsize} | --                        | Puts the firmware in PK2GO Learn Mode |
| Exit Learn Mode          | 0xB6         | --                        | Ignore       |
| Enable Pk2go Mode        | 0xB7 {0x50} {0x4B} {0x32} {EEsize} | --                        | Puts the firmware in PK2GO Mode |
| Logic Analyzer Go        | 0xB8 {EdgeRising} {TrigMask} {TrigStates} {EdgeMask} {TrigCount} {PostTrigCountL} {PostTrigCountH} {SampleRateFactor} | {TrigLocL}{TrigLocH}      |              |
| Copy RAM Upload          | 0xB9 {StartAddrL} {StartAddrH} | --                        |              |

# Set VDD #
```
CCPH:CCPL = ((Vdd * 32) + 10.5) << 6
```
where Vdd is desired voltage.

```
VDDLim = (Vfault / 5) * 255
```
where Vdd < VFault is error.

# Set VPP #
CCPR2L = duty cycle.  Generally = 0x40.
```
VPPADC = Vpp * 18.61
```
where Vpp is desired voltage.
```
VPPlim = Vfault * 18.61
```
where Vdd < VFault is error.

# Status #

| **Bit** | **Mask** | **Description**     |
|:--------|:---------|:--------------------|
| 0       | 0x0001   | Vdd GND On          |
| 1       | 0x0002   | Vdd On              |
| 2       | 0x0004   | Vpp GND On          |
| 3       | 0x0008   | Vpp On              |
| 4       | 0x0010   | Vdd Error           |
| 5       | 0x0020   | Vpp Error           |
| 6       | 0x0040   | Button Pressed      |
| 7       | 0x0080   |                     |
| 8       | 0x0100   | Reset               |
| 9       | 0x0200   | UART Mode           |
| 10      | 0x0400   | ICD TimeOut         |
| 11      | 0x0800   | Upload Full         |
| 12      | 0x1000   | Download Empty      |
| 13      | 0x2000   | Empty Script        |
| 14      | 0x4000   | Script Buffer Overflow |
| 15      | 0x8000   | Download Overflow   |

# Scripts for PIC32 #

Enter programming mode, 39 bytes.
| fa           |   VPP\_OFF |
|:-------------|:-----------|
| f7           |   MCLR\_GND\_ON |
| f9           |   VPP\_PWM\_ON |
| f5           |   BUSY\_LED\_ON |
| f3 00        |   SET\_ICSP\_PINS |
| e8 14        |   DELAY\_LONG |
| f6           |   MCLR\_GND\_OFF |
| fb           |   VPP\_ON  |
| e7 17        |   DELAY\_SHORT |
| fa           |   VPP\_OFF |
| f7           |   MCLR\_GND\_ON |
| e7 2f        |   DELAY\_SHORT |
| f2 b2        |   WRITE\_BYTE\_LITERAL |
| f2 c2        |   WRITE\_BYTE\_LITERAL |
| f2 12        |   WRITE\_BYTE\_LITERAL |
| f2 0a        |   WRITE\_BYTE\_LITERAL |
| f6           |   MCLR\_GND\_OFF |
| fb           |   VPP\_ON  |
| e7 eb        |   DELAY\_SHORT |
| f3 02        |   SET\_ICSP\_PINS |
| bc 06 1f     |   JT2\_SETMODE |
| bb 04        |   JT2\_SENDCMD |
| bb 07        |   JT2\_SENDCMD |
| ba 00        |   JT2\_XFERDATA8\_LIT |

Exit programming mode, 15 bytes.
| bc 05 1f     |   JT2\_SETMODE |
|:-------------|:---------------|
| fa           |   VPP\_OFF     |
| f7           |   MCLR\_GND\_ON |
| f8           |   VPP\_PWM\_OFF |
| f3 06        |   SET\_ICSP\_PINS |
| f3 02        |   SET\_ICSP\_PINS |
| f3 03        |   SET\_ICSP\_PINS |
| e8 0a        |   DELAY\_LONG  |
| f4           |   BUSY\_LED\_OFF |

Read device id, 9 bytes.
| bb 04          | JT2\_SENDCMD |
|:---------------|:-------------|
| bb 01          | JT2\_SENDCMD |
| b9 00 00 00 00 | JT2\_XFERDATA32\_LIT |

Read program memory, 13 bytes.
| bb 0e          | JT2\_SENDCMD |
|:---------------|:-------------|
| b8 20 00 01 00 | JT2\_XFRFASTDAT\_LIT |
| b7             | JT2\_XFRFASTDAT\_BUF |
| b4             | JT2\_WAIT\_PE\_RESP |
| b5             | JT2\_GET\_PE\_RESP |
| e9 01 1f       | LOOP         |

Prepare writing program memory, 6 bytes.
| bb 0e          | JT2\_SENDCMD |
|:---------------|:-------------|
| b7             | JT2\_XFRFASTDAT\_BUF |
| e9 01 3f       | LOOP         |

Write program memory, 5 bytes.
| b7             | JT2\_XFRFASTDAT\_BUF |
|:---------------|:---------------------|
| e9 01 3f       | LOOP                 |
| b3             | JT2\_PE\_PROG\_RESP  |

Erase chip, 6 bytes.
| bb 07          | JT2\_SENDCMD |
|:---------------|:-------------|
| ba fc          | JT2\_XFERDATA8\_LIT |
| e8 4a          | DELAY\_LONG  |

# Script instructions #

|**Code**|**Used**|**Params**|**Name**|
|:-------|:-------|:---------|:-------|
| 0xB3   | +      |          | JT2\_PE\_PROG\_RESP   |
| 0xB4   | +      |          | JT2\_WAIT\_PE\_RESP   |
| 0xB5   | +      |          | JT2\_GET\_PE\_RESP    |
| 0xB6   |        |          | JT2\_XFERINST\_BUF   |
| 0xB7   | +      |          | JT2\_XFRFASTDAT\_BUF |
| 0xB8   | +      | 4        | JT2\_XFRFASTDAT\_LIT |
| 0xB9   | +      | 4        | JT2\_XFERDATA32\_LIT |
| 0xBA   | +      | 1        | JT2\_XFERDATA8\_LIT  |
| 0xBB   | +      | 1        | JT2\_SENDCMD        |
| 0xBC   | +      | 2        | JT2\_SETMODE        |
| 0xBD   |        |          | UNIO\_TX\_RX         |
| 0xBE   |        |          | UNIO\_TX            |
| 0xBF   |        |          | MEASURE\_PULSE      |
| 0xC0   |        |          | ICDSLAVE\_TX\_BUF\_BL |
| 0xC1   |        |          | ICDSLAVE\_TX\_LIT\_BL |
| 0xC2   |        |          | ICDSLAVE\_RX\_BL     |
| 0xC3   |        |          | SPI\_RDWR\_BYTE\_BUF  |
| 0xC4   |        |          | SPI\_RDWR\_BYTE\_LIT  |
| 0xC5   |        |          | SPI\_RD\_BYTE\_BUF    |
| 0xC6   |        |          | SPI\_WR\_BYTE\_BUF    |
| 0xC7   |        |          | SPI\_WR\_BYTE\_LIT    |
| 0xC8   |        |          | I2C\_RD\_BYTE\_NACK   |
| 0xC9   |        |          | I2C\_RD\_BYTE\_ACK    |
| 0xCA   |        |          | I2C\_WR\_BYTE\_BUF    |
| 0xCB   |        |          | I2C\_WR\_BYTE\_LIT    |
| 0xCC   |        |          | I2C\_STOP           |
| 0xCD   |        |          | I2C\_START          |
| 0xCE   |        |          | AUX\_STATE\_BUFFER   |
| 0xCF   |        |          | SET\_AUX            |
| 0xD0   |        |          | WRITE\_BITS\_BUF\_HLD |
| 0xD1   |        |          | WRITE\_BITS\_LIT\_HLD |
| 0xD2   |        |          | CONST\_WRITE\_DL     |
| 0xD3   |        |          | WRITE\_BUFBYTE\_W    |
| 0xD4   |        |          | WRITE\_BUFWORD\_W    |
| 0xD5   |        |          | RD2\_BITS\_BUFFER    |
| 0xD6   |        |          | RD2\_BYTE\_BUFFER    |
| 0xD7   |        |          | VISI24             |
| 0xD8   |        |          | NOP24              |
| 0xD9   |        |          | COREINST24         |
| 0xDA   |        |          | COREINST18         |
| 0xDB   |        |          | POP\_DOWNLOAD       |
| 0xDC   |        |          | ICSP\_STATES\_BUFFER |
| 0xDD   |        |          | LOOPBUFFER         |
| 0xDE   |        |          | ICDSLAVE\_TX\_BUF    |
| 0xDF   |        |          | ICDSLAVE\_TX\_LIT    |
| 0xE0   |        |          | ICDSLAVE\_RX        |
| 0xE1   |        |          | POKE\_SFR           |
| 0xE2   |        |          | PEEK\_SFR           |
| 0xE3   |        |          | EXIT\_SCRIPT        |
| 0xE4   |        |          | GOTO\_INDEX         |
| 0xE5   |        |          | IF\_GT\_GOTO         |
| 0xE6   |        |          | IF\_EQ\_GOTO         |
| 0xE7   | +      | 1        | DELAY\_SHORT        |
| 0xE8   | +      | 1        | DELAY\_LONG         |
| 0xE9   | +      | 2        | LOOP               |
| 0xEA   | +      | 1        | SET\_ICSP\_SPEED     |
| 0xEB   |        |          | READ\_BITS          |
| 0xEC   |        |          | READ\_BITS\_BUFFER   |
| 0xED   |        |          | WRITE\_BITS\_BUFFER  |
| 0xEE   |        |          | WRITE\_BITS\_LITERAL |
| 0xEF   |        |          | READ\_BYTE          |
| 0xF0   |        |          | READ\_BYTE\_BUFFER   |
| 0xF1   |        |          | WRITE\_BYTE\_BUFFER  |
| 0xF2   | +      | 1        | WRITE\_BYTE\_LITERAL |
| 0xF3   | +      | 1        | SET\_ICSP\_PINS      |
| 0xF4   | +      |          | BUSY\_LED\_OFF       |
| 0xF5   | +      |          | BUSY\_LED\_ON        |
| 0xF6   | +      |          | MCLR\_GND\_OFF       |
| 0xF7   | +      |          | MCLR\_GND\_ON        |
| 0xF8   | +      |          | VPP\_PWM\_OFF        |
| 0xF9   | +      |          | VPP\_PWM\_ON         |
| 0xFA   | +      |          | VPP\_OFF            |
| 0xFB   | +      |          | VPP\_ON             |
| 0xFC   | +      |          | VDD\_GND\_OFF        |
| 0xFD   | +      |          | VDD\_GND\_ON         |
| 0xFE   | +      |          | VDD\_OFF            |
| 0xFF   | +      |          | VDD\_ON             |