#ifndef __CONFIGS_H__
#define __CONFIGS_H__

#include "include.h"

np_data config_list[]=
{
	{                         
		//No 0
		//The config for jz4730 uboot
		.pt = JZ4730,
		.et = HARDHM,      //HW HM ECC
		.ep = 0,           //ecc position index 0
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 64,
		.rc = 3,
		.bbp = 0,
		.bba = 0,

		.ebase = 0x13010000, 
		.dport = 0x14000000, 
		.gport = 0, 
		.bm_ms = 0x100,
		.pm_ms = 0xb0000,
		.gm_ms = 0,
		.ap_offset = 0x80000,
		.cp_offset = 0x40000,

		.nand_init = nand_init_4730,
		.nand_erase = nand_erase_4730,
		.nand_program = nand_program_4730,
		.nand_read = nand_read_4730,
		.nand_read_oob = nand_read_oob_4730,
		.nand_block_markbad = nand_block_markbad,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block,
		.nand_select = chip_select_4730,
	},

	{                    
		//No 1
		//The config for jz4730 linux fs

		.pt = JZ4730,
		.et = HARDHM,      //HW HM ECC
		.ep = 1,           //ecc position index 1
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 64,
		.rc = 3,
		.bbp = 0,
		.bba = 0,

		.ebase = 0x13010000, 
		.dport = 0x14000000, 
		.gport = 0, 
		.bm_ms = 0x100,
		.pm_ms = 0xb0000,
		.gm_ms = 0,
		.ap_offset = 0x80000,
		.cp_offset = 0x40000,

		.nand_init = nand_init_4730,
		.nand_erase = nand_erase_4730,
		.nand_program = nand_program_4730,
		.nand_read = nand_read_4730,
		.nand_read_oob = nand_read_oob_4730,
		.nand_block_markbad = nand_block_markbad,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block,
		.nand_select = chip_select_4730,

	},

	{       
		//No 2
		//The config for jz4730 ucos
		.pt = JZ4730,
		.et = HARDHM,      //HW HM ECC
		.ep = 1,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 64,
		.rc = 3,
		.bbp = 0,          //need modify
		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x14000000, 
		.gport = 0, 
		.bm_ms = 0x100,
		.pm_ms = 0xb0000,
		.gm_ms = 0,
		.ap_offset = 0x80000,
		.cp_offset = 0x40000,
		
		.nand_init = nand_init_4730,
		.nand_erase = nand_erase_4730,
		.nand_program = nand_program_4730,
		.nand_read = nand_read_4730,
		.nand_read_oob = nand_read_oob_4730,
		.nand_block_markbad = nand_block_markbad,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block,
		.nand_select = chip_select_4730,

	},
	{       
		//No 3
		//The config for jz4730 wince
		.pt = JZ4730,
		.et = HARDHM,      //HW HM ECC
		.ep = 1,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 64,
		.rc = 3,
		.bbp = 0,          //need modify
		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x14000000, 
		.gport = 0, 
		.bm_ms = 0x100,
		.pm_ms = 0xb0000,
		.gm_ms = 0,
		.ap_offset = 0x80000,
		.cp_offset = 0x40000,
		
		.nand_init = nand_init_4730,
		.nand_erase = nand_erase_4730,
		.nand_program = nand_program_4730,
		.nand_read = nand_read_4730,
		.nand_read_oob = nand_read_oob_4730,
		.nand_block_markbad = nand_block_markbad,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block,
		.nand_select = chip_select_4730,

	},

	{       
		//No 4
		//The config for jz4740 uboot use HW RS
		.pt = JZ4740,
		.et = HARDRS,      //HW HM ECC
		.ep = 2,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 128,
		.rc = 3,
		.bbp = 0,          //need modify
		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x18000000, 
		.gport = 0x10010000, 
		.bm_ms = 0x100,
		.pm_ms = 0x20000,
		.gm_ms = 0x500,
		.ap_offset = 0x10000,
		.cp_offset = 0x8000,
		
		.nand_init = nand_init_4740,
		.nand_erase = nand_erase_4740,
		.nand_program = nand_program_4740,
		.nand_read = nand_read_4740_rs,
		.nand_read_oob = nand_read_oob_4740,
		.nand_block_markbad = nand_block_markbad_4740,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block_4740,
		.nand_select = chip_select_4740,

	},

	{       
		//No 5
		//The config for jz4740 linux use HW RS
		.pt = JZ4740,
		.et = HARDRS,      //HW HM ECC
		.ep = 3,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 128,
		.rc = 3,
		.bbp = 1,          //need modify
		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x18000000, 
		.gport = 0x10010000, 
		.bm_ms = 0x100,
		.pm_ms = 0x20000,
		.gm_ms = 0x500,
		.ap_offset = 0x10000,
		.cp_offset = 0x8000,
		
		.nand_init = nand_init_4740,
		.nand_fini = nand_fini_4740,
		.nand_erase = nand_erase_4740,
		.nand_program = nand_program_4740,
		.nand_read = nand_read_4740_rs,
		.nand_read_oob = nand_read_oob_4740,
		.nand_block_markbad = nand_block_markbad_4740,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block_4740,
		.nand_select = chip_select_4740,

	},

	{       
		//No 6
		//The config for jz4740 linux use HW HM
		.pt = JZ4740,
		.et = HARDHM,      //HW HM ECC
		.ep = 1,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 128,
		.rc = 3,
		.bbp = 0,          //need modify
		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x18000000, 
		.gport = 0x10010000, 
		.bm_ms = 0x100,
		.pm_ms = 0x20000,
		.gm_ms = 0x500,
		.ap_offset = 0x10000,
		.cp_offset = 0x8000,

		.nand_init = nand_init_4740,
		.nand_erase = nand_erase_4740,
		.nand_program = nand_program_4740,
		.nand_read = nand_read_4740_hm,
		.nand_read_oob = nand_read_oob_4740,
		.nand_block_markbad = nand_block_markbad_4740,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block_4740,
		.nand_select = chip_select_4740,

	},

	{       
		//No 7
		//The config for jz4740 ucos use HW RS
		.pt = JZ4740,
		.et = HARDRS,      //HW HM ECC
//		.ep = 3,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 128,
		.rc = 3,
//		.bbp = 0,          //need modify
//		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x18000000, 
		.gport = 0x10010000, 
		.bm_ms = 0x100,
		.pm_ms = 0x20000,
		.gm_ms = 0x500,
		.ap_offset = 0x10000,
		.cp_offset = 0x8000,
		
		.nand_init = nand_init_4740,
		.nand_erase = nand_erase_4740,
		.nand_program = nand_program_4740,
		.nand_read = nand_read_4740_rs,
		.nand_read_oob = nand_read_oob_4740,
		.nand_block_markbad = nand_block_markbad_4740,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block_4740,
		.nand_select = chip_select_4740,

	},

	{       
		//No 8
		//The config for jz4740 ucos use HW HM
		.pt = JZ4740,
		.et = HARDHM,      //HW HM ECC
//		.ep = 3,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 128,
		.rc = 3,
//		.bbp = 0,          //need modify
//		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x18000000, 
		.gport = 0x10010000, 
		.bm_ms = 0x100,
		.pm_ms = 0x20000,
		.gm_ms = 0x500,
		.ap_offset = 0x10000,
		.cp_offset = 0x8000,
		
		.nand_init = nand_init_4740,
		.nand_erase = nand_erase_4740,
		.nand_program = nand_program_4740,
		.nand_read = nand_read_4740_hm,
		.nand_read_oob = nand_read_oob_4740,
		.nand_block_markbad = nand_block_markbad_4740,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block_4740,
		.nand_select = chip_select_4740,

	},

	{       
		//No 9
		//The config for jz4740 wince use HW RS
		.pt = JZ4740,
		.et = HARDRS,      //HW HM ECC
//		.ep = 3,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 128,
		.rc = 3,
//		.bbp = 0,          //need modify
//		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x18000000, 
		.gport = 0x10010000, 
		.bm_ms = 0x100,
		.pm_ms = 0x20000,
		.gm_ms = 0x500,
		.ap_offset = 0x10000,
		.cp_offset = 0x8000,
		
		.nand_init = nand_init_4740,
		.nand_erase = nand_erase_4740,
		.nand_program = nand_program_4740,
		.nand_read = nand_read_4740_rs,
		.nand_read_oob = nand_read_oob_4740,
		.nand_block_markbad = nand_block_markbad_4740,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block_4740,
		.nand_select = chip_select_4740,

	},

	{       
		//No 10
		//The config for jz4740 wince use HW RS
		.pt = JZ4740,
		.et = HARDHM,      //HW HM ECC
//		.ep = 3,           //need modify
		.bw = 8,
		.ps = 2048,
		.os = 64,
		.ppb = 128,
		.rc = 3,
//		.bbp = 0,          //need modify
//		.bba = 0,          //need modify

		//do not need modify
		.ebase = 0x13010000, 
		.dport = 0x18000000, 
		.gport = 0x10010000, 
		.bm_ms = 0x100,
		.pm_ms = 0x20000,
		.gm_ms = 0x500,
		.ap_offset = 0x10000,
		.cp_offset = 0x8000,
		
		.nand_init = nand_init_4740,
		.nand_erase = nand_erase_4740,
		.nand_program = nand_program_4740,
		.nand_read = nand_read_4740_hm,
		.nand_read_oob = nand_read_oob_4740,
		.nand_block_markbad = nand_block_markbad_4740,
		.nand_check = nand_check_cmp,
		.nand_check_block = nand_check_block_4740,
		.nand_select = chip_select_4740,

	},


};


#endif
