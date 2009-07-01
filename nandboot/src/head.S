/*
 * head.S
 *
 * Entry of n-boot
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
#ifdef CONFIG_JZ4740
#include <jz4740.h>
#include <jz4740_board.h>
#endif

	.text
	.set noreorder
	.global startup
startup:
#ifdef CONFIG_JZ4740
	.word JZ4740_NANDBOOT_CFG	/* fetched by CPU during NAND Boot */
#endif
	/*
	 * Disable all interrupts
	 */
	la	$8, 0xB0001004		/* INTC_IMR */
	li	$9, 0xffffffff
	sw	$9, 0($8)

	/* 
	 * CU0=UM=EXL=IE=0, BEV=ERL=1, IP2~7=1
	 */
	li	$26, 0x0040FC04
	mtc0	$26, $12		/* CP0_STATUS */

	/* IV=1, use the specical interrupt vector (0x200) */
	li	$26, 0x00800000
	mtc0	$26, $13		/* CP0_CAUSE */

	/* Setup stack pointer */
	la	$29, 0x80004000

	/* Jump to the nand boot routine */
	j	nand_boot
	nop
	.set reorder
