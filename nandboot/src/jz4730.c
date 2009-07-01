/*
 * jz4730.c
 *
 * JZ4730 common routines
 *
 * Copyright (c) 2005-2007  Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>

#ifdef CONFIG_JZ4730

#include <jz4730.h>
#include <jz4730_board.h>

void pll_init(void)
{
	unsigned int nf, plcr1;

	nf = CFG_CPU_SPEED * 2 / CFG_EXTAL;
	plcr1 = ((nf-2) << CPM_PLCR1_PLL1FD_BIT) |
		(0 << CPM_PLCR1_PLL1RD_BIT) |	/* RD=0, NR=2, 1.8432 = 3.6864/2 */
		(0 << CPM_PLCR1_PLL1OD_BIT) |   /* OD=0, NO=1 */
		(0x20 << CPM_PLCR1_PLL1ST_BIT) | /* PLL stable time */
		CPM_PLCR1_PLL1EN;                /* enable PLL */          

	/* Clock divisors.
	 * 
	 * CFCR values: when CPM_CFCR_UCS(bit 28) is set, select external USB clock.
	 *
	 * 0x00411110 -> 1:2:2:2:2
	 * 0x00422220 -> 1:3:3:3:3
	 * 0x00433330 -> 1:4:4:4:4
	 * 0x00444440 -> 1:6:6:6:6
	 * 0x00455550 -> 1:8:8:8:8
	 * 0x00466660 -> 1:12:12:12:12
	 */
	REG_CPM_CFCR = 0x00422220 | (((CFG_CPU_SPEED/48000000) - 1) << 25);

	/* PLL out frequency */
	REG_CPM_PLCR1 = plcr1;
}

#define MEM_CLK (CFG_CPU_SPEED / 3)

/*
 * Init SDRAM memory.
 */
void sdram_init(void)
{
	register unsigned int dmcr0, dmcr, sdmode, tmp, ns;

	unsigned int cas_latency_sdmr[2] = {
		EMC_SDMR_CAS_2,
		EMC_SDMR_CAS_3,
	};

	unsigned int cas_latency_dmcr[2] = {
		1 << EMC_DMCR_TCL_BIT,	/* CAS latency is 2 */
		2 << EMC_DMCR_TCL_BIT	/* CAS latency is 3 */
	};

	REG_EMC_BCR = 0;	/* Disable bus release */
	REG_EMC_RTCSR = 0;	/* Disable clock for counting */

	/* Fault DMCR value for mode register setting*/
#define SDRAM_ROW0    11
#define SDRAM_COL0     8
#define SDRAM_BANK40   0

	dmcr0 = ((SDRAM_ROW0-11)<<EMC_DMCR_RA_BIT) |
		((SDRAM_COL0-8)<<EMC_DMCR_CA_BIT) |
		(SDRAM_BANK40<<EMC_DMCR_BA_BIT) |
		(CFG_SDRAM_BW16<<EMC_DMCR_BW_BIT) |
		EMC_DMCR_EPIN |
		cas_latency_dmcr[((CFG_SDRAM_CASL == 3) ? 1 : 0)];

	/* Basic DMCR value */
	dmcr = ((CFG_SDRAM_ROW-11)<<EMC_DMCR_RA_BIT) |
		((CFG_SDRAM_COL-8)<<EMC_DMCR_CA_BIT) |
		(CFG_SDRAM_BANK4<<EMC_DMCR_BA_BIT) |
		(CFG_SDRAM_BW16<<EMC_DMCR_BW_BIT) |
		EMC_DMCR_EPIN |
		cas_latency_dmcr[((CFG_SDRAM_CASL == 3) ? 1 : 0)];

	/* SDRAM timing */
	ns = 1000000000 / MEM_CLK;
	tmp = CFG_SDRAM_TRAS/ns;
	if (tmp < 4)
		tmp = 4;
	if (tmp > 11)
		tmp = 11;
	dmcr |= ((tmp-4) << EMC_DMCR_TRAS_BIT);
	tmp = CFG_SDRAM_RCD/ns;
	if (tmp > 3)
		tmp = 3;
	dmcr |= (tmp << EMC_DMCR_RCD_BIT);
	tmp = CFG_SDRAM_TPC/ns;
	if (tmp > 7)
		tmp = 7;
	dmcr |= (tmp << EMC_DMCR_TPC_BIT);
	tmp = CFG_SDRAM_TRWL/ns;
	if (tmp > 3)
		tmp = 3;
	dmcr |= (tmp << EMC_DMCR_TRWL_BIT);
	tmp = (CFG_SDRAM_TRAS + CFG_SDRAM_TPC)/ns;
	if (tmp > 14)
		tmp = 14;
	dmcr |= (((tmp + 1) >> 1) << EMC_DMCR_TRC_BIT);

	/* SDRAM mode value */
	sdmode = EMC_SDMR_BT_SEQ | 
		 EMC_SDMR_OM_NORMAL |
		 EMC_SDMR_BL_4 | 
		 cas_latency_sdmr[((CFG_SDRAM_CASL == 3) ? 1 : 0)];
	if (CFG_SDRAM_BW16)
		sdmode <<= 1;
	else
		sdmode <<= 2;

	/* Stage 1. Precharge all banks by writing SDMR with DMCR.MRSET=0 */
	REG_EMC_DMCR = dmcr;
	REG8(EMC_SDMR0|sdmode) = 0;
	REG8(EMC_SDMR1|sdmode) = 0;

	/* Wait for precharge, > 200us */
	tmp = (CFG_CPU_SPEED / 1000000) * 1000;
	while (tmp--);

	/* Stage 2. Enable auto-refresh */
	REG_EMC_DMCR = dmcr | EMC_DMCR_RFSH;

	tmp = CFG_SDRAM_TREF/ns;
	tmp = tmp/64 + 1;
	if (tmp > 0xff)	tmp = 0xff;
	REG_EMC_RTCOR = tmp;
	REG_EMC_RTCNT = 0;
	REG_EMC_RTCSR = EMC_RTCSR_CKS_64;	/* Divisor is 64, CKO/64 */

	/* Wait for number of auto-refresh cycles */
	tmp = (CFG_CPU_SPEED / 1000000) * 1000;
	while (tmp--);

	/* Stage 3. Mode Register Set */
	REG_EMC_DMCR = dmcr0 | EMC_DMCR_RFSH | EMC_DMCR_MRSET;
	REG8(EMC_SDMR0|sdmode) = 0;
	REG8(EMC_SDMR1|sdmode) = 0;

        /* Set back to the ture basic DMCR value */
	REG_EMC_DMCR = dmcr | EMC_DMCR_RFSH | EMC_DMCR_MRSET;

	/* everything is ok now */
}

#endif /* CONFIG_JZ4730 */
