/*
 * board-pmp.c
 *
 * JZ4730-based PMP board routines.
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

void gpio_init(void)
{
	__gpio_as_uart0();
	__gpio_as_sdram_32bit();
}

#endif /* CONFIG_JZ4740 */
