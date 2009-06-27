/*
 * Authors: Marek Lindner <lindner_marek@yahoo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <errno.h>
#include <confuse.h>
#include <unistd.h>   
#include <string.h>
#include "ingenic_cfg.h"
#include "usb_boot_defines.h"

extern unsigned int total_size;

int hand_init_def(struct hand *hand)
{
	/* nand flash info */
	/* hand.nand_start=0; */ /* important !!!! */
	hand->pt = JZ4740;    /* cpu type  */
	hand->nand_bw = 8;
	hand->nand_rc = 3;
	hand->nand_ps = 2048;
	hand->nand_os = 64;
	hand->nand_ppb = 64;
	hand->nand_eccpos = 6;
	hand->nand_bbpage = 0;
	hand->nand_bbpos  = 0;
	hand->nand_force_erase = 0;
	/* hand.nand_ids=0;  */ /* vendor_id & device_id */
	hand->fw_args.cpu_id = 0x4740;
	hand->fw_args.ext_clk = 12;
	hand->fw_args.cpu_speed = 225 / hand->fw_args.ext_clk;
	hand->fw_args.phm_div = 3;
	hand->fw_args.use_uart = 0;
	hand->fw_args.boudrate = 57600;
	hand->fw_args.bus_width = 0;
	hand->fw_args.bank_num = 1;
	hand->fw_args.row_addr = 13;
	hand->fw_args.col_addr = 9;
	hand->fw_args.is_mobile = 0;
	hand->fw_args.is_busshare = 1;

	return 1;
}

int check_dump_cfg(struct hand *hand)
{
	printf("Now checking whether all configure args valid:");
	/* check PLL */
	if (hand->fw_args.ext_clk > 27 || hand->fw_args.ext_clk < 12) {
		printf("\n EXTCLK setting invalid!");
		return 0;
	}
	if (hand->fw_args.phm_div > 32 || hand->fw_args.ext_clk < 2) {
		printf("\n PHMDIV setting invalid!");
		return 0;
	}
	if ((hand->fw_args.cpu_speed * hand->fw_args.ext_clk ) % 12 != 0) {
		printf("\n CPUSPEED setting invalid!");
		return 0;
	}

	/* check SDRAM */
	if (hand->fw_args.bus_width > 1 ) {
		printf("\n SDRAMWIDTH setting invalid!");
		return 0;
	}
	if (hand->fw_args.bank_num > 1 ) {
		printf("\n BANKNUM setting invalid!");
		return 0;
	}
	if (hand->fw_args.row_addr > 13 && hand->fw_args.row_addr < 11 ) {
		printf("\n ROWADDR setting invalid!");
		return 0;
	}
	if (hand->fw_args.col_addr > 13 && hand->fw_args.col_addr < 11 ) {
		printf("\n COLADDR setting invalid!");
		return 0;
	}

	/* check NAND */
	if ( hand->nand_ps < 2048 && hand->nand_os > 16 ) {
		printf("\n PAGESIZE or OOBSIZE setting invalid!");
		printf("\n PAGESIZE is %d,\t OOBSIZE is %d", 
		       hand->nand_ps, hand->nand_os);
		return 0;
	}
	if ( hand->nand_ps < 2048 && hand->nand_ppb > 32 ) {
		printf("\n PAGESIZE or PAGEPERBLOCK setting invalid!");
		return 0;
	}

	if ( hand->nand_ps > 512 && hand->nand_os <= 16 ) {
		printf("\n PAGESIZE or OOBSIZE setting invalid!");
		printf("\n PAGESIZE is %d,\t OOBSIZE is %d", 
		       hand->nand_ps, hand->nand_os);
		return 0;
	}
	if ( hand->nand_ps > 512 && hand->nand_ppb < 64 ) {
		printf("\n PAGESIZE or PAGEPERBLOCK setting invalid!");
		return 0;
	}
	printf(" YES\n");

	printf("Current device information:\n");
	printf("CPU type is Ingenic XBurst Jz%x\n",hand->fw_args.cpu_id);
	printf("Crystal work at %dMHz, the CCLK up to %dMHz and PMH_CLK up to %dMHz\n",
		hand->fw_args.ext_clk,
		(unsigned int)hand->fw_args.cpu_speed * hand->fw_args.ext_clk,
		((unsigned int)hand->fw_args.cpu_speed * hand->fw_args.ext_clk) / hand->fw_args.phm_div);

	printf("SDRAM Total size is %d MB, work in %d bank and %d bit mode\n",
		total_size / 0x100000, 2 * (hand->fw_args.bank_num + 1), 
	       16 * (2 - hand->fw_args.bus_width));

	printf("Nand page size %d, "
	       "ECC offset in OOB %d, "
	       "bad block offset in OOB %d, "
	       "bad block page %d, "
	       "use %d plane mode\n",
	       hand->nand_ps,
	       hand->nand_eccpos,
	       hand->nand_bbpos,
	       hand->nand_bbpage,
	       hand->nand_plane);
	return 1;
}

int parse_configure(struct hand *hand, char * file_path)
{
	if (access(file_path, F_OK)) {
		fprintf(stderr, "Error - can't read configure file %s.\n",
			file_path);
		return -1;
	}

	hand_init_def(hand);

	cfg_opt_t opts[] = {
		CFG_INT("BOUDRATE", 57600, CFGF_NONE),
		CFG_SIMPLE_INT("EXTCLK", &hand->fw_args.ext_clk),
		CFG_SIMPLE_INT("CPUSPEED", &hand->fw_args.cpu_speed),
		CFG_SIMPLE_INT("PHMDIV", &hand->fw_args.phm_div),
		CFG_SIMPLE_INT("USEUART", &hand->fw_args.use_uart),

		CFG_SIMPLE_INT("BUSWIDTH", &hand->fw_args.bus_width),
		CFG_SIMPLE_INT("BANKS", &hand->fw_args.bank_num),
		CFG_SIMPLE_INT("ROWADDR", &hand->fw_args.row_addr),
		CFG_SIMPLE_INT("COLADDR", &hand->fw_args.col_addr),

		CFG_SIMPLE_INT("ISMOBILE", &hand->fw_args.is_mobile),
		CFG_SIMPLE_INT("ISBUSSHARE", &hand->fw_args.is_busshare),
		CFG_SIMPLE_INT("DEBUGOPS", &hand->fw_args.debug_ops),
		CFG_SIMPLE_INT("PINNUM", &hand->fw_args.pin_num),
		CFG_SIMPLE_INT("START", &hand->fw_args.start),
		CFG_SIMPLE_INT("SIZE", &hand->fw_args.size),

		CFG_SIMPLE_INT("NAND_BUSWIDTH", &hand->nand_bw),
		CFG_SIMPLE_INT("NAND_ROWCYCLES", &hand->nand_rc),
		CFG_SIMPLE_INT("NAND_PAGESIZE", &hand->nand_ps),
		CFG_SIMPLE_INT("NAND_PAGEPERBLOCK", &hand->nand_ppb),
		CFG_SIMPLE_INT("NAND_FORCEERASE", &hand->nand_force_erase),
		CFG_SIMPLE_INT("NAND_OOBSIZE", &hand->nand_os),
		CFG_SIMPLE_INT("NAND_ECCPOS", &hand->nand_eccpos),
		CFG_SIMPLE_INT("NAND_BADBLOCKPOS", &hand->nand_bbpos),
		CFG_SIMPLE_INT("NAND_BADBLOCKPAGE", &hand->nand_bbpage),
		CFG_SIMPLE_INT("NAND_PLANENUM", &hand->nand_plane),
		CFG_SIMPLE_INT("NAND_BCHBIT", &hand->nand_bchbit),
		CFG_SIMPLE_INT("NAND_WPPIN", &hand->nand_wppin),
		CFG_SIMPLE_INT("NAND_BLOCKPERCHIP", &hand->nand_bpc),

		CFG_END()
	};

	cfg_t *cfg;
	cfg = cfg_init(opts, 0);
	if (cfg_parse(cfg, file_path) == CFG_PARSE_ERROR)
		return -1;

	hand->fw_args.boudrate = cfg_getint(cfg, "BOUDRATE");
	cfg_free(cfg);

	hand->fw_args.cpu_id = 0x4740;
	if (hand->fw_args.bus_width == 32)
		hand->fw_args.bus_width = 0 ;
	else
		hand->fw_args.bus_width = 1 ; 
	hand->fw_args.bank_num = hand->fw_args.bank_num / 4; 
	hand->fw_args.cpu_speed = hand->fw_args.cpu_speed / hand->fw_args.ext_clk;
	
	total_size = (unsigned int)
		(2 << (hand->fw_args.row_addr + hand->fw_args.col_addr - 1)) * 2 
		* (hand->fw_args.bank_num + 1) * 2 
		* (2 - hand->fw_args.bus_width);

	if (check_dump_cfg(hand) < 1)
		return -1;

	return 1;
}
