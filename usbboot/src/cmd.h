/*
 * Copyright(C) 2009  Qi Hardware Inc.,
 * Authors: Xiangfu Liu <xiangfu@qi-hardware.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CMD_H__
#define __CMD_H__

#include "usb_boot_defines.h"

#define COMMAND_NUM 31
#define MAX_ARGC	10
#define MAX_COMMAND_LENGTH	100

int boot(char *stage1_path, char *stage2_path);
int init_nand_in();
int nand_prog(void);
int nand_query(void);
int nand_erase(struct nand_in *nand_in);
int debug_memory(int obj, unsigned int start, unsigned int size);
int debug_gpio(int obj, unsigned char ops, unsigned char pin);
int debug_go(void);

#endif  /* __CMD_H__ */
