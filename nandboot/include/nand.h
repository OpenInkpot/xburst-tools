/*
 * nand.h
 *
 * Standard NAND Flash definitions.
 */
#ifndef __NAND_H__
#define __NAND_H__

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_RNDOUT		5
#define NAND_CMD_PAGEPROG	0x10
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_RNDIN		0x85
#define NAND_CMD_READID		0x90
#define NAND_CMD_ERASE2		0xd0
#define NAND_CMD_RESET		0xff

/* Extended commands for large page devices */
#define NAND_CMD_READSTART	0x30
#define NAND_CMD_RNDOUTSTART	0xE0
#define NAND_CMD_CACHEDPROG	0x15

/* Status bits */
#define NAND_STATUS_FAIL	0x01
#define NAND_STATUS_FAIL_N1	0x02
#define NAND_STATUS_TRUE_READY	0x20
#define NAND_STATUS_READY	0x40
#define NAND_STATUS_WP		0x80

/*
 * NAND Flash Manufacturer ID Codes
 */
#define NAND_MFR_TOSHIBA	0x98
#define NAND_MFR_SAMSUNG	0xec
#define NAND_MFR_FUJITSU	0x04
#define NAND_MFR_NATIONAL	0x8f
#define NAND_MFR_RENESAS	0x07
#define NAND_MFR_STMICRO	0x20
#define NAND_MFR_HYNIX		0xad
#define NAND_MFR_MICRON		0x2c

/*
 * NAND parameter struct
 */
struct nand_param {
	unsigned int bus_width;		/* data bus width: 8-bit/16-bit */
	unsigned int row_cycle;		/* row address cycles: 2/3 */
	unsigned int page_size;		/* page size in bytes: 512/2048 */
	unsigned int oob_size;		/* oob size in bytes: 16/64 */
	unsigned int page_per_block;	/* pages per block: 32/64/128 */
	unsigned int bad_block_pos;	/* bad block pos in oob: 0/5 */
};

/*
 * NAND routines
 */
extern void nand_enable(void);
extern void nand_disable(void);
extern int block_is_bad(struct nand_param *nandp, int block);
extern int nand_read_page(struct nand_param *nandp, int block, int page, unsigned char *dst);

#endif /* __NAND_H__ */
