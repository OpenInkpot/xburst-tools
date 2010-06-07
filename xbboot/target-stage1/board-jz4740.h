//
// Authors: Xiangfu Liu <xiangfu@sharism.cc>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 3 of the License, or (at your option) any later version.
//
#ifndef __BOARD_JZ4740_H__
#define __BOARD_JZ4740_H__

void gpio_init_4740();
void pll_init_4740();
void serial_init_4740();
void sdram_init_4740();
void nand_init_4740();

// tbd: do they have to be copied into globals? or just reference STAGE1_ARGS_ADDR?
volatile u32	ARG_EXTAL;
volatile u32	ARG_CPU_SPEED;
volatile u8	ARG_PHM_DIV;
volatile u32	ARG_UART_BASE;
volatile u32	ARG_UART_BAUD;
volatile u8	ARG_BUS_WIDTH_16;
volatile u8	ARG_BANK_ADDR_2BIT;
volatile u8	ARG_ROW_ADDR;
volatile u8	ARG_COL_ADDR;
volatile u32	ARG_CPU_ID;

#endif
