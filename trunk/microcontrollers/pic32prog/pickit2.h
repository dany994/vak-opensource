/*
 * Обобщённый JTAG-адаптер. Программный интерфейс нижнего уровня.
 * Автор: С.Вакуленко.
 *
 * Этот файл распространяется в надежде, что он окажется полезным, но
 * БЕЗ КАКИХ БЫ ТО НИ БЫЛО ГАРАНТИЙНЫХ ОБЯЗАТЕЛЬСТВ; в том числе без косвенных
 * гарантийных обязательств, связанных с ПОТРЕБИТЕЛЬСКИМИ СВОЙСТВАМИ и
 * ПРИГОДНОСТЬЮ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
 *
 * Вы вправе распространять и/или изменять этот файл в соответствии
 * с условиями Генеральной Общественной Лицензии GNU (GPL) в том виде,
 * как она была опубликована Фондом Свободного ПО; либо версии 2 Лицензии
 * либо (по вашему желанию) любой более поздней версии. Подробности
 * смотрите в прилагаемом файле 'COPYING.txt'.
 */
#define CMD_NO_OPERATION           0x5A     // Do nothing
#define CMD_GET_VERSION            0x76     // {major} {minor} {dot}
                                            // Get firmware version
#define CMD_BOOT_MODE              0x42     // Enter Bootloader mode
#define CMD_SET_VDD                0xA0     // {CCPL} {CCPH} {VDDLim}
#define CMD_SET_VPP                0xA1     // {CCPR2L} {VPPADC} {VPPLim}
#define CMD_READ_STATUS            0xA2     // {StsL} {StsH}
#define CMD_READ_VOLTAGES          0xA3     // {VddL} {VddH} {VppL} {VppH}
#define CMD_DOWNLOAD_SCRIPT        0xA4     // {Script#} {ScriptLengthN} {Script1} {Script2} ... {ScriptN}
                                            // Store a script in the Script Buffer
#define CMD_RUN_SCRIPT             0xA5     // {Script#} {iterations}
                                            // Run a script from the script buffer
#define CMD_EXECUTE_SCRIPT         0xA6     // {ScriptLengthN} {Script1} {Script2} ... {ScriptN}
                                            // Immediately execute the included script
#define CMD_CLEAR_DOWNLOAD_BUFFER  0xA7     // Empty the download buffer
#define CMD_DOWNLOAD_DATA          0xA8     // {DataLength} {Data1} {Data2} ... {DataN}
                                            // Add data to download buffer
#define CMD_CLEAR_UPLOAD_BUFFER    0xA9     // Empty the upload buffer
#define CMD_UPLOAD_DATA            0xAA     // {DataLengthN} {data1} {data2} ... {dataN}
                                            // Read data from upload buffer
#define CMD_CLEAR_SCRIPT_BUFFER    0xAB
#define CMD_UPLOAD_DATA_NOLEN      0xAC     // {Data1} {Data2} ... {DataN}
                                            // Read data from upload buffer
#define CMD_END_OF_BUFFER          0xAD     // Skip the rest of commands
#define CMD_RESET                  0xAE     // Reset
#define CMD_SCRIPT_BUFFER_CSUM     0xAF     // {LenSumL} {LenSumH} {BufSumL} {BufSumH}
                                            // Calculate checksums of the Script Buffer
#define CMD_SET_VOLTAGE_CAL        0xB0     // {adc_calfactorL} {adc_calfactorH} {vdd_offset} {calfactor}
#define CMD_WRITE_INTERNAL_EEPROM  0xB1     // {address} {datalength} {data1} {data2} ... {dataN}
                                            // Write data to PIC18F2550 EEPROM
#define CMD_READ_INTERNAL_EEPROM   0xB2     // {address} {datalength}
                                            // {data1} {data2} ... {dataN}
                                            // Read bytes from PIC18F2550 EEPROM
#define CMD_ENTER_UART_MODE        0xB3
#define CMD_EXIT_UART_MODE         0xB4     // Exits the firmware from UART Mode
#define CMD_ENTER_LEARN_MODE       0xB5     // {0x50} {0x4B} {0x32} {EEsize}
                                            // Puts the firmware in PK2GO Learn Mode
#define CMD_EXIT_LEARN_MODE        0xB6     // Ignore
#define CMD_ENABLE_PK2GO_MODE      0xB7     // {0x50} {0x4B} {0x32} {EEsize}
                                            // Puts the firmware in PK2GO Mode
#define CMD_LOGIC_ANALYZER_GO      0xB8     // {EdgeRising} {TrigMask} {TrigStates} {EdgeMask} {TrigCount} {PostTrigCountL} {PostTrigCountH} {SampleRateFactor}
                                            // {TrigLocL} {TrigLocH}
#define CMD_COPY_RAM_UPLOAD        0xB9     // {StartAddrL} {StartAddrH}
