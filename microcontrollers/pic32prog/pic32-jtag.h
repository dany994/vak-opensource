/*
 * Microchip PIC32 jtag definitions.
 */

/*
 * TAP instructions.
 */
#define TAP_SW_MTAP     4       // Switch to MCHP TAP controller
#define TAP_SW_ETAP     5       // Switch to EJTAG TAP controller

/*
 * MTAP-specific instructions.
 */
#define MTAP_IDCODE     1       // Select chip identification register
#define MTAP_COMMAND    7       // Connect to MCHP command register

/*
 * ETAP-specific instructions.
 */
#define ETAP_ADDRESS    8       // Select Address register
#define ETAP_DATA       9       // Select Data register
#define ETAP_CONTROL    10      // Select EJTAG Control register
#define ETAP_EJTAGBOOT  12      // On reset, enter EJTAG break handler
#define ETAP_FASTDATA   14      // Select FastData register

/*
 * MCHP commands.
 */
#define MCHP_STATUS        0x00 // Return Status
#define MCHP_ASSERT_RST    0xD1 // Assert device reset
#define MCHP_DEASSERT_RST  0xD0 // Remove device reset
#define MCHP_ERASE         0xFC // Flash chip erase
#define MCHP_FLASH_ENABLE  0xFE // Enable access from CPU to flash
#define MCHP_FLASH_DISABLE 0xFD // Disable access from CPU to flash
