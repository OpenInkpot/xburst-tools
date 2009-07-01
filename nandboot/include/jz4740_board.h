/*
 * jz4740_board.h
 *
 * JZ4740 board definitions.
 *
 * Copyright (c) 2005-2008 Ingenic Semiconductor Inc.
 *
 */
#ifndef __JZ4740_BOARD_H__
#define __JZ4740_BOARD_H__

/*-------------------------------------------------------------------
 * NAND Boot config code
 */
#define JZ4740_NANDBOOT_CFG	JZ4740_NANDBOOT_B8R3	/* NAND Boot config code */

/*-------------------------------------------------------------------
 * Frequency of the external OSC in Hz.
 */
#define CFG_EXTAL		12000000

/*-------------------------------------------------------------------
 * CPU speed.
 */
#define CFG_CPU_SPEED		336000000

/*-------------------------------------------------------------------
 * Serial console.
 */
#define CFG_UART_BASE		UART0_BASE

#define CONFIG_BAUDRATE		57600

/*-------------------------------------------------------------------
 * SDRAM info.
 */

// SDRAM paramters
#define CFG_SDRAM_BW16		0	/* Data bus width: 0-32bit, 1-16bit */
#define CFG_SDRAM_BANK4		1	/* Banks each chip: 0-2bank, 1-4bank */
#define CFG_SDRAM_ROW		13	/* Row address: 11 to 13 */
#define CFG_SDRAM_COL		9	/* Column address: 8 to 12 */
#define CFG_SDRAM_CASL		2	/* CAS latency: 2 or 3 */

// SDRAM Timings, unit: ns
#define CFG_SDRAM_TRAS		45	/* RAS# Active Time */
#define CFG_SDRAM_RCD		20	/* RAS# to CAS# Delay */
#define CFG_SDRAM_TPC		20	/* RAS# Precharge Time */
#define CFG_SDRAM_TRWL		7	/* Write Latency Time */
#define CFG_SDRAM_TREF		7812	/* Refresh period: 8192 refresh cycles/64ms */

/*-------------------------------------------------------------------
 * Linux kernel command line.
 */
#define CFG_CMDLINE	"mem=32M console=ttyS0,57600n8 ip=off rootfstype=yaffs2 root=/dev/mtdblock2 rw init=/etc/inittab"

#endif /* __JZ4740_BOARD_H__ */
