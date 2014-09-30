#define AT_CMD_ProgEn		0xAC530000ul // 10101100 01010011 xxxxxxxx xxxxxxxx
#define AT_CMD_ChipErase	0xAC800000ul // 10101100 100xxxxx xxxxxxxx xxxxxxxx
#define AT_CMD_HighPart		0x08000000ul
#define AT_CMD_ReadFlash	0x20000000ul // 00100000 aaaaaaaa bbbbbbbb oooooooo
#define AT_CMD_UploadPage	0x40000000ul // 01000000 aaaaaaaa bbbbbbbb iiiiiiii
#define AT_CMD_WritePage	0x4C000000ul // 01001100 aaaaaaaa bbxxxxxx iiiiiiii
#define AT_CMD_ReadEEPROM	0xA0000000ul // 10100000 aaaaaaaa bbbbbbbb oooooooo
#define AT_CMD_WriteEEPROM	0xC0000000ul // 11000000 aaaaaaaa bbbbbbbb iiiiiiii
#define AT_CMD_ReadLock		0x58000000ul // 01011000 xxxxxxxx xxxxxxxx oooooooo
#define AT_CMD_WriteLock	0xACE00000ul // 10101100 111xxiix xxxxxxxx iiiiiiii
#define AT_CMD_ReadSign		0x30000000ul // 00110000 xxxxxxxx xxxxxxbb oooooooo

#define AT_CMD_WriteFuse	0xACA00000ul // 10101100 10100000 xxxxxxxx iiiiiiii
#define AT_CMD_WriteFuseH	0xACA80000ul // 10101100 10101000 xxxxxxxx iiiiiiii
#define AT_CMD_WriteFuseE	0xACA40000ul // 10101100 10100100 xxxxxxxx iiiiiiii

#define AT_CMD_ReadFuse		0x50000000ul // 01010000 00000000 xxxxxxxx oooooooo
#define AT_CMD_ReadFuseH	0x58080000ul // 01011000 00001000 xxxxxxxx oooooooo
#define AT_CMD_ReadFuseE	0x50080000ul // 01010000 00001000 xxxxxxxx oooooooo
#define AT_CMD_ReadCalib	0x38000000ul // 00111000 00xxxxxx 00000000 oooooooo

#define AT_CMD_ProgEn_Test1 (AT_CMD_ProgEn | 0x0008ul) // LY: mini-test single 1 after serious 0
#define AT_CMD_ProgEn_Test2 (AT_CMD_ProgEn | 0xFFF7ul) // LY: mini-test single 0 after serious 1

struct _avr_t {
	prog_t 		*prog;
	char		*name;
	char		have_fuse;
	unsigned char	vendor_code;
	unsigned char	part_family;
	unsigned char	part_number;
	u_int32_t	flash_size;
	unsigned short	page_size;
	unsigned short	page_delay;
	u_int32_t	page_addr, last_addr;
	unsigned char	page_addr_fetched;
	int	last_cmd_byte;
	int cable_errors;
};
