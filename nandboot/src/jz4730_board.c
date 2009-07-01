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
#include <jz4730.h>
#include <jz4730_board.h>

#ifdef CONFIG_JZ4730

void gpio_init(void)
{
	__harb_usb0_uhc();
	__gpio_as_emc();
	__gpio_as_uart0();
	__gpio_as_uart3();
}

#endif /* CONFIG_JZ4730 */
