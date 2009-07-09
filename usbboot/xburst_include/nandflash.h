/*
 * Copyright (C) 2009 Qi Hardware Inc.,
 * Author:  Xiangfu Liu <xiangfu@qi-hardware.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA
 */
#ifndef __NANDLIB_H__
#define __NANDLIB_H__

#include "xburst_types.h"

#ifndef NULL
#define NULL	0
#endif

/*  Jz4740 nandflash interface */
unsigned int nand_query_4740(u8 *);
int nand_init_4740(int bus_width, int row_cycle, int page_size, int page_per_block,
		   int,int,int,int);
int nand_fini_4740(void);
u32 nand_program_4740(void *context, int spage, int pages, int option);
//int nand_program_oob_4740(void *context, int spage, int pages, void (*notify)(int));
u32 nand_erase_4740(int blk_num, int sblk, int force);
u32 nand_read_4740(void *buf, u32 startpage, u32 pagenum,int option);
u32 nand_read_oob_4740(void *buf, u32 startpage, u32 pagenum);
u32 nand_read_raw_4740(void *buf, u32 startpage, u32 pagenum,int);
u32 nand_mark_bad_4740(int bad);
void nand_enable_4740(u32 csn);
void nand_disable_4740(u32 csn);

/*  Jz4750 nandflash interface */
unsigned int nand_query_4750(u8 *);
//int nand_init_4750(int bus_width, int row_cycle, int page_size, int page_per_block,
//		   int,int,int,int);

int nand_init_4750(int bus_width, int row_cycle, int page_size, int page_per_block,
		   int bch_bit, int ecc_pos, int bad_pos, int bad_page, int force);

int nand_fini_4750(void);
u32 nand_program_4750(void *context, int spage, int pages, int option);
//int nand_program_oob_4740(void *context, int spage, int pages, void (*notify)(int));
u32 nand_erase_4750(int blk_num, int sblk, int force);
u32 nand_read_4750(void *buf, u32 startpage, u32 pagenum,int option);
u32 nand_read_oob_4750(void *buf, u32 startpage, u32 pagenum);
u32 nand_read_raw_4750(void *buf, u32 startpage, u32 pagenum,int);
u32 nand_mark_bad_4750(int bad);

void nand_enable_4750(u32 csn);
void nand_disable_4750(u32 csn);

#endif
