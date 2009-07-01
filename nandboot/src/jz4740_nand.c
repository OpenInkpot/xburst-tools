/*
 * jz4740_nand.c
 *
 * NAND read routine for JZ4740
 *
 * Copyright (c) 2005-2008 Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>

#ifdef CONFIG_JZ4740

#include <nand.h>
#include <jz4740.h>

#define NAND_DATAPORT		0xb8000000
#define NAND_ADDRPORT		0xb8010000
#define NAND_COMMPORT		0xb8008000

#define ECC_BLOCK		512
#define ECC_POS			6
#define PAR_SIZE		9

#define __nand_cmd(n)		(REG8(NAND_COMMPORT) = (n))
#define __nand_addr(n)		(REG8(NAND_ADDRPORT) = (n))
#define __nand_data8()		REG8(NAND_DATAPORT)
#define __nand_data16()		REG16(NAND_DATAPORT)

#define __nand_enable()		(REG_EMC_NFCSR |= EMC_NFCSR_NFE1 | EMC_NFCSR_NFCE1)
#define __nand_disable()	(REG_EMC_NFCSR &= ~(EMC_NFCSR_NFCE1))
#define __nand_ecc_rs_encoding() \
	(REG_EMC_NFECR = EMC_NFECR_ECCE | EMC_NFECR_ERST | EMC_NFECR_RS | EMC_NFECR_RS_ENCODING)
#define __nand_ecc_rs_decoding() \
	(REG_EMC_NFECR = EMC_NFECR_ECCE | EMC_NFECR_ERST | EMC_NFECR_RS | EMC_NFECR_RS_DECODING)
#define __nand_ecc_disable()	(REG_EMC_NFECR &= ~EMC_NFECR_ECCE)
#define __nand_ecc_encode_sync() while (!(REG_EMC_NFINTS & EMC_NFINTS_ENCF))
#define __nand_ecc_decode_sync() while (!(REG_EMC_NFINTS & EMC_NFINTS_DECF))

/*--------------------------------------------------------------*/

static inline void nand_wait_ready(void)
{
	unsigned int timeout = 1000;
	while ((REG_GPIO_PXPIN(2) & 0x40000000) && timeout--);
	while (!(REG_GPIO_PXPIN(2) & 0x40000000));
}

static inline void nand_read_buf16(void *buf, int count)
{
	int i;
	u16 *p = (u16 *)buf;

	for (i = 0; i < count; i += 2)
		*p++ = __nand_data16();
}

static inline void nand_read_buf8(void *buf, int count)
{
	int i;
	u8 *p = (u8 *)buf;

	for (i = 0; i < count; i++)
		*p++ = __nand_data8();
}

static inline void nand_read_buf(void *buf, int count, int bw)
{
	if (bw == 8)
		nand_read_buf8(buf, count);
	else
		nand_read_buf16(buf, count);
}

/*
 * Correct 1~9-bit errors in 512-bytes data 
 */
static void rs_correct(unsigned char *dat, int idx, int mask)
{
	int i, j;
	unsigned short d, d1, dm;

	i = (idx * 9) >> 3;
	j = (idx * 9) & 0x7;

	i = (j == 0) ? (i - 1) : i;
	j = (j == 0) ? 7 : (j - 1);

	if (i > 512) return;

	if (i == 512)
		d = dat[i - 1];
	else
		d = (dat[i] << 8) | dat[i - 1];

	d1 = (d >> j) & 0x1ff;
	d1 ^= mask;

	dm = ~(0x1ff << j);
	d = (d & dm) | (d1 << j);

	dat[i - 1] = d & 0xff;
	if (i < 512)
		dat[i] = (d >> 8) & 0xff;
}

/*
 * Read oob
 */
static int nand_read_oob(struct nand_param *nandp, int page_addr, u8 *buf, int size)
{
	int page_size, row_cycle, bus_width;
	int col_addr;

	page_size = nandp->page_size;
	row_cycle = nandp->row_cycle;
	bus_width = nandp->bus_width;

	if (page_size == 2048)
		col_addr = 2048;
	else
		col_addr = 0;

	if (page_size == 2048)
		/* Send READ0 command */
		__nand_cmd(NAND_CMD_READ0);
	else
		/* Send READOOB command */
		__nand_cmd(NAND_CMD_READOOB);

	/* Send column address */
	__nand_addr(col_addr & 0xff);
	if (page_size == 2048)
		__nand_addr((col_addr >> 8) & 0xff);

	/* Send page address */
	__nand_addr(page_addr & 0xff);
	__nand_addr((page_addr >> 8) & 0xff);
	if (row_cycle == 3)
		__nand_addr((page_addr >> 16) & 0xff);

	/* Send READSTART command for 2048 ps NAND */
	if (page_size == 2048)
		__nand_cmd(NAND_CMD_READSTART);

	/* Wait for device ready */
	nand_wait_ready();

	/* Read oob data */
	nand_read_buf(buf, size, bus_width);

	return 0;
}


/*
 * nand_read_page()
 *
 * Input:
 *
 *	nandp - pointer to nand info
 *	block - block number: 0, 1, 2, ...
 *	page - page number within a block: 0, 1, 2, ...
 *	dst - pointer to target buffer
 */
int nand_read_page(struct nand_param *nandp, int block, int page, u8 *dst)
{
	int page_size, oob_size, page_per_block;
	int row_cycle, bus_width, ecc_count;
	int page_addr, i, j;
	u8 *data_buf;
	u8 oob_buf[64];

	page_size = nandp->page_size;
	oob_size = nandp->oob_size;
	page_per_block = nandp->page_per_block;
	row_cycle = nandp->row_cycle;
	bus_width = nandp->bus_width;

	page_addr = page + block * page_per_block;

	/*
	 * Read oob data
	 */
	nand_read_oob(nandp, page_addr, oob_buf, oob_size);

	/*
	 * Read page data
	 */

	/* Send READ0 command */
	__nand_cmd(NAND_CMD_READ0);

	/* Send column address */
	__nand_addr(0);
	if (page_size == 2048)
		__nand_addr(0);

	/* Send page address */
	__nand_addr(page_addr & 0xff);
	__nand_addr((page_addr >> 8) & 0xff);
	if (row_cycle == 3)
		__nand_addr((page_addr >> 16) & 0xff);

	/* Send READSTART command for 2048 ps NAND */
	if (page_size == 2048)
		__nand_cmd(NAND_CMD_READSTART);

	/* Wait for device ready */
	nand_wait_ready();

	/* Read page data */
	data_buf = dst;

	ecc_count = page_size / ECC_BLOCK;

	for (i = 0; i < ecc_count; i++) {
		volatile u8 *paraddr = (volatile u8 *)EMC_NFPAR0;
		unsigned int stat;

		/* Enable RS decoding */
		REG_EMC_NFINTS = 0x0;
		__nand_ecc_rs_decoding();

		/* Read data */
		nand_read_buf((void *)data_buf, ECC_BLOCK, bus_width);

		/* Set PAR values */
		for (j = 0; j < PAR_SIZE; j++) {
			*paraddr++ = oob_buf[ECC_POS + i*PAR_SIZE + j];
		}

		/* Set PRDY */
		REG_EMC_NFECR |= EMC_NFECR_PRDY;

		/* Wait for completion */
		__nand_ecc_decode_sync();

		/* Disable decoding */
		__nand_ecc_disable();

		/* Check result of decoding */
		stat = REG_EMC_NFINTS;
		if (stat & EMC_NFINTS_ERR) {
			/* Error occurred */
			if (stat & EMC_NFINTS_UNCOR) {
				/* Uncorrectable error occurred */
			}
			else {
				unsigned int errcnt, index, mask;

				errcnt = (stat & EMC_NFINTS_ERRCNT_MASK) >> EMC_NFINTS_ERRCNT_BIT;
				switch (errcnt) {
				case 4:
					index = (REG_EMC_NFERR3 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT;
					mask = (REG_EMC_NFERR3 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT;
					rs_correct(data_buf, index, mask);
				case 3:
					index = (REG_EMC_NFERR2 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT;
					mask = (REG_EMC_NFERR2 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT;
					rs_correct(data_buf, index, mask);
				case 2:
					index = (REG_EMC_NFERR1 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT;
					mask = (REG_EMC_NFERR1 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT;
					rs_correct(data_buf, index, mask);
				case 1:
					index = (REG_EMC_NFERR0 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT;
					mask = (REG_EMC_NFERR0 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT;
					rs_correct(data_buf, index, mask);
					break;
				default:
					break;
				}
			}
		}

		data_buf += ECC_BLOCK;
	}

	return 0;
}

/*
 * Check bad block
 *
 * Note: the bad block flag may be store in either the first or the last
 * page of the block.
 */
int block_is_bad(struct nand_param *nandp, int block)
{
	int page_addr;
	u8 oob_buf[64];

	page_addr = block * nandp->page_per_block;
	nand_read_oob(nandp, page_addr, oob_buf, nandp->oob_size);
	if (oob_buf[nandp->bad_block_pos] != 0xff)
		return 1;

	page_addr = (block + 1) * nandp->page_per_block - 1;
	nand_read_oob(nandp, page_addr, oob_buf, nandp->oob_size);
	if (oob_buf[nandp->bad_block_pos] != 0xff)
		return 1;

	return 0;
}

/*
 * Enable NAND controller
 */
void nand_enable(void)
{
	__nand_enable();

	REG_EMC_SMCR1 = 0x04444400;
}

/*
 * Disable NAND controller
 */
void nand_disable(void)
{
	__nand_disable();
}

#endif /* CONFIG_JZ4740 */
