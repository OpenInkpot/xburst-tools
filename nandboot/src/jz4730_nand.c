/*
 * jz4730_nand.c
 *
 * NAND read routine for JZ4730
 *
 * Copyright (c) 2005-2008 Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>

#ifdef CONFIG_JZ4730

#include <nand.h>
#include <jz4730.h>

/* NAND command/address/data port */
#define NAND_DATAPORT    	0xB4000000  /* read-write area */
#define NAND_CMDPORT     	0xB4040000  /* write only area */
#define NAND_ADDRPORT    	0xB4080000  /* write only area */

#define ECC_BLOCK		256 /* 3-bytes HW ECC per 256-bytes data */
#define ECC_POS			4   /* ECC offset to spare area */

#define __nand_enable()		(REG_EMC_NFCSR |= EMC_NFCSR_NFE | EMC_NFCSR_FCE)
#define __nand_disable()	(REG_EMC_NFCSR &= ~(EMC_NFCSR_NFE | EMC_NFCSR_FCE))
#define __nand_ecc_enable()	(REG_EMC_NFCSR |= EMC_NFCSR_ECCE | EMC_NFCSR_ERST)
#define __nand_ecc_disable()	(REG_EMC_NFCSR &= ~EMC_NFCSR_ECCE)
#define __nand_ready()		(REG_EMC_NFCSR & EMC_NFCSR_RB)
#define __nand_sync()		while (!__nand_ready())
#define __nand_ecc()		(REG_EMC_NFECC & 0x00ffffff)
#define __nand_cmd(n)		(REG8(NAND_CMDPORT) = (n))
#define __nand_addr(n)		(REG8(NAND_ADDRPORT) = (n))
#define __nand_data8()		REG8(NAND_DATAPORT)
#define __nand_data16()		REG16(NAND_DATAPORT)

/*--------------------------------------------------------------*/

static inline void nand_wait_ready(void)
{
	__nand_sync();
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
	u8 *databuf;
	u8 oob_buf[64];
	u32 calc_ecc[8];

	page_size = nandp->page_size;
	oob_size = nandp->oob_size;
	page_per_block = nandp->page_per_block;
	row_cycle = nandp->row_cycle;
	bus_width = nandp->bus_width;

	page_addr = page + block * page_per_block;

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
	databuf = dst;

	ecc_count = page_size / ECC_BLOCK;

	for (i = 0; i < ecc_count; i++) {

		/* Enable HW ECC */
		__nand_ecc_enable();

		/* Read data */
		nand_read_buf((void *)databuf, ECC_BLOCK, bus_width);

		/* Disable HW ECC */
		__nand_ecc_disable();

		/* Record the ECC */
		calc_ecc[i] = __nand_ecc();

		databuf += ECC_BLOCK;
	}

	/*
	 * Read oob data
	 */
	nand_read_oob(nandp, page_addr, oob_buf, oob_size);

	/*
	 * ECC correction
	 *
	 * Note: the ECC correction algorithm should be conformed to
	 * the encoding algorithm. It depends on what encoding algorithm
	 * is used? SW ECC? HW ECC?
	 */

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

	REG_EMC_SMCR3 = 0x04444400;
}

/*
 * Disable NAND controller
 */
void nand_disable(void)
{
	__nand_disable();
}

#endif /* CONFIG_JZ4730 */
