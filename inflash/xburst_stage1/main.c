/*
 * Main routine of the firmware.
 *
 * Copyright (C) 2009 PI
 * Author:  Xiangfu Liu <xiangfu.z@gmail.com>
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
#include "jz4740.h"
#include "configs.h"
#include "usb_boot_defines.h"

struct fw_args *fw_args;
volatile u32 CPU_ID;
volatile u32 UART_BASE;
volatile u32 CONFIG_BAUDRATE;
volatile u8 SDRAM_BW16;
volatile u8 SDRAM_BANK4;
volatile u8 SDRAM_ROW;
volatile u8 SDRAM_COL;
volatile u8 CONFIG_MOBILE_SDRAM;
volatile u32 CFG_CPU_SPEED;
volatile u32 CFG_EXTAL;
volatile u8 PHM_DIV;
volatile u8 IS_SHARE;
extern int pllout2;
#if 0
void test_load_args(void)
{
	CPU_ID = 0x4740 ;
	CFG_EXTAL = 12000000 ;
	CFG_CPU_SPEED = 252000000 ;
	PHM_DIV = 3;
	fw_args->use_uart = 0;
	UART_BASE = UART0_BASE + fw_args->use_uart * 0x1000;
	CONFIG_BAUDRATE = 57600;
	SDRAM_BW16 = 16;
	SDRAM_BANK4 = 4;
	SDRAM_ROW = 13;
	SDRAM_COL = 9;
	CONFIG_MOBILE_SDRAM = 0;
	IS_SHARE = 1;

	fw_args->debug_ops = -1;
}
#endif

void load_args(void)
{
	fw_args = (struct fw_args *)0x80002008; /* get the fw args from memory */
	CPU_ID = fw_args->cpu_id ;
	CFG_EXTAL = (u32)fw_args->ext_clk * 1000000;
	CFG_CPU_SPEED = (u32)fw_args->cpu_speed * CFG_EXTAL ;
	if (CFG_EXTAL == 19000000) {
		CFG_EXTAL = 19200000;
		CFG_CPU_SPEED = 192000000;
	}
	PHM_DIV = fw_args->phm_div;
	if (fw_args->use_uart > 3) 
		fw_args->use_uart = 0;
	UART_BASE = UART0_BASE + fw_args->use_uart * 0x1000;
	CONFIG_BAUDRATE = fw_args->boudrate;
	SDRAM_BW16 = fw_args->bus_width == 0 ? 32 : 16;
	SDRAM_BANK4 = fw_args->bank_num * 4;
	SDRAM_ROW = fw_args->row_addr;
	SDRAM_COL = fw_args->col_addr;
	CONFIG_MOBILE_SDRAM = fw_args->is_mobile;
	IS_SHARE = fw_args->is_busshare;
}

void c_main(void)
{
	load_args();

	if (fw_args->debug_ops > 0) {
		do_debug();
		return ;
	}

	switch (CPU_ID) {
	case 0x4740:
		gpio_init_4740();
		pll_init_4740();
		serial_init();
		sdram_init_4740();
		break;
	case 0x4750:
		gpio_init_4750();
		pll_init_4750();
		serial_init();
		sdram_init_4750();
		break;
	default:
		return;
	}
#if 1
	serial_puts("Setup xburst CPU args as:\n");
	serial_put_hex(CPU_ID);
	serial_put_hex(CFG_EXTAL);
	serial_put_hex(CFG_CPU_SPEED);
	serial_put_hex(PHM_DIV);
	serial_put_hex(fw_args->use_uart);
	serial_put_hex(CONFIG_BAUDRATE);
	serial_put_hex(SDRAM_BW16);
	serial_put_hex(SDRAM_BANK4);
	serial_put_hex(SDRAM_ROW);
	serial_put_hex(SDRAM_COL);
	serial_put_hex(pllout2);
	serial_put_hex(REG_CPM_CPCCR);
#endif
	serial_puts("xburst stage1 run finish !\n");
	load_args();
}
