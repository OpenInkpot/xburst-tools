/*
 * nand_boot.c
 *
 * NAND boot routine.
 *
 * Then nand boot loader can load the zImage type to execute.
 * To get the zImage, build the kernel by 'make zImage'.
 *
 * Copyright (c) 2005-2008  Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>
#include <nand.h>

#ifdef CONFIG_JZ4730
#include <jz4730.h>
#include <jz4730_board.h>
#endif

#ifdef CONFIG_JZ4740
#include <jz4740.h>
#include <jz4740_board.h>
#endif

/*
 * NAND Flash parameters (must be conformed to the NAND being used)
 */
static struct nand_param nand_p = {
	.bus_width 	= 8,		/* data bus width: 8-bit/16-bit */
	.row_cycle 	= 3,		/* row address cycles: 2/3 */
	.page_size 	= 2048,		/* page size in bytes: 512/2048 */
	.oob_size 	= 64,		/* oob size in bytes: 16/64 */
	.page_per_block	= 128,		/* pages per block: 32/64/128 */
	.bad_block_pos	= 0		/* bad block pos in oob: 0/5 */
};

#define SIZE_KB (1 * 1024)
#define SIZE_MB (1 * 1024 * 1024)

/*
 * Kernel image
 */
#define CFG_KERNEL_OFFS		(256 * SIZE_KB)	/* NAND offset of kernel image being loaded, has to be aligned to a block address! */
#define CFG_KERNEL_SIZE		(2 * SIZE_MB)	/* Size of kernel image, has to be integer multiply of a block size! */
#define CFG_KERNEL_DST		0x80600000	/* Load kernel to this addr */
#define CFG_KERNEL_START	0x80600000	/* Start kernel from this addr	*/

/*
 * Kernel parameters
 */
#define PARAM_BASE		0x80004000

/*
 * Local variables
 */
static u32 *param_addr = 0;
static u8 *tmpbuf = 0;

static u8 cmdline[256] = CFG_CMDLINE;

extern void gpio_init(void);
extern int serial_init (void);
extern void pll_init(void);
extern void sdram_init(void);

/*
 * Load kernel image from NAND into RAM
 */
static int nand_load(struct nand_param *nandp, int offs, int kernel_size, u8 *dst)
{
	int page_size, page_per_block;
	int block;
	int block_size;
	int blockcopy_count;
	int page;

	page_size = nandp->page_size;
	page_per_block = nandp->page_per_block;

	/*
	 * Enable NANDFlash controller
	 */
	nand_enable();

	/*
	 * offs has to be aligned to a block address!
	 */
	block_size = page_size * page_per_block;
	block = offs / block_size;
	blockcopy_count = 0;

	while (blockcopy_count < (kernel_size / block_size)) {
		for (page = 0; page < page_per_block; page++) {
			if (page == 0) {
				/*
				 * New block
				 */
				if (block_is_bad(nandp, block)) {
					block++;

					/*
					 * Skip bad block
					 */
					continue;
				}
			}

			nand_read_page(nandp, block, page, dst);

			dst += page_size;
		}

		block++;
		blockcopy_count++;
	}

	/*
	 * Disable NANDFlash controller
	 */
	nand_disable();

	return 0;
}

/*
 * NAND Boot routine
 */
void nand_boot(void)
{
	unsigned int boot_sel, i;

	void (*kernel)(int, char **, char *);

	/*
	 * Init gpio, serial, pll and sdram
	 */
	gpio_init();
	serial_init();

	serial_puts("\n\nNAND Secondary Program Loader\n\n");

	pll_init();
	sdram_init();

#ifdef CONFIG_JZ4740
	/*
	 * JZ4740 can detect some NAND parameters from the boot select
	 */
	boot_sel = REG_EMC_BCR >> 30;
	if (boot_sel == 0x2)
		nand_p.page_size = 512;
	else
		nand_p.page_size = 2048;
#endif

#ifdef CONFIG_JZ4730
	/*
	 * JZ4730 can detect some NAND parameters from the boot select
	 */
	boot_sel = (REG_EMC_NFCSR & 0x70) >> 4;

	nand_p.bus_width = (boot_sel & 0x1) ? 16 : 8;
	nand_p.page_size = (boot_sel & 0x2) ? 2048 : 512;
	nand_p.row_cycle = (boot_sel & 0x4) ? 3 : 2;
#endif

	/*
	 * Load kernel image from NAND into RAM
	 */
	nand_load(&nand_p, CFG_KERNEL_OFFS, CFG_KERNEL_SIZE, (u8 *)CFG_KERNEL_DST);

	serial_puts("Starting kernel ...\n\n");

	/*
	 * Prepare kernel parameters and environment
	 */
	param_addr = (u32 *)PARAM_BASE;
	param_addr[0] = 0;	/* might be address of ascii-z string: "memsize" */
	param_addr[1] = 0;	/* might be address of ascii-z string: "0x01000000" */
	param_addr[2] = 0;
	param_addr[3] = 0;
	param_addr[4] = 0;
	param_addr[5] = PARAM_BASE + 32;
	param_addr[6] = CFG_KERNEL_START;
	tmpbuf = (u8 *)(PARAM_BASE + 32);

	for (i = 0; i < 256; i++)
		tmpbuf[i] = cmdline[i];  /* linux command line */

	kernel = (void (*)(int, char **, char *))CFG_KERNEL_START;

	/*
	 * Flush caches
	 */
	flush_cache_all();

	/*
	 * Jump to kernel image
	 */
	(*kernel)(2, (char **)(PARAM_BASE + 16), (char *)PARAM_BASE);
}
