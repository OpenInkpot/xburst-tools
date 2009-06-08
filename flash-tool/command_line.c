/*
 * "Ingenic flash tool" - flash the Ingenic CPU via USB
 *
 * (C) Copyright 2009
 * Author: Xiangfu Liu <xiangfu.z@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb_boot_defines.h"
#include "ingenic_usb.h"
#include "cmd.h"
#include "config.h"
 
extern struct nand_in nand_in;
int com_argc;
char com_argv[MAX_ARGC][MAX_COMMAND_LENGTH];

static const char COMMAND[][30]=
{
	"",
	"query",
	"querya",
	"erase",
	"read",
	"prog",
	"nquery",
	"nerase",
	"nread",
	"nreadraw",
	"nreadoob", /* index 10 */
	"nprog",
	"help",
	"version",
	"go",
	"fconfig",
	"exit",
	"readnand",
	"gpios",
	"gpioc",
	"boot", /* index 20 */
	"list",
	"select",
	"unselect",
	"chip",
	"unchip",
	"nmake",
	"load",
	"memtest",
	"run"
};

static int handle_help(void)
{
	printf("\n Command support in current version:"
	       "\n help          print this help;"
	       "\n boot          boot device and make it in stage2;"
	       "\n list          show current device number can connect;"
	       "\n fconfig       set USB Boot config file;"
	       "\n nquery        query NAND flash info;"
	       "\n nread         read NAND flash data with checking bad block and ECC;"
	       "\n nreadraw      read NAND flash data without checking bad block and ECC;"
	       "\n nreadoob      read NAND flash oob without checking bad block and ECC;"
	       "\n nerase        erase NAND flash;"
	       "\n nprog         program NAND flash with data and ECC;"
	       "\n nmark         mark a bad block in NAND flash;"
	       "\n go            execute program in SDRAM;"
	       "\n version       show current USB Boot software version;"
	       "\n exit          quit from telnet session;"
	       "\n readnand      read data from nand flash and store to SDRAM;"
	       "\n load          load file data to SDRAM;"
	       "\n run           run command script in file;"
	       "\n memtest       do SDRAM test;"
	       "\n gpios         let one GPIO to high level;"
	       "\n gpioc         let one GPIO to low level;");
	/* printf("\n nmake         read all data from nand flash and store to file(experimental);"); */
	return 1;
}

static int handle_version(void)
{
	printf("\n USB Boot Software current version: %s", CURRENT_VERSION);	
	return 1;
}

static int handle_fconfig(void)
{
	if (com_argc < 3) {
		printf("\n Usage:"
		       " fconfig (1) (2) "
		       "\n 1:configration file name"
		       "\n 2:deivce index number");
		return -1;
	}
	/* usb_infenic_config(atoi(com_argv[2]),com_argv[1]); */
	return 1;
}

/* need transfer two para :blk_num ,start_blk */
int handle_nerase(void)
{
	int i;
	if (com_argc < 5) {
		printf("\n Usage:");
		printf(" nerase (1) (2) (3) (4) ");
		printf("\n 1:start block number"
		       "\n 2:block length"
		       "\n 3:device index number"
		       "\n 4:flash chip index number");
		return -1;
	}

	init_nand_in();

	nand_in.start = atoi(com_argv[1]);
	nand_in.length = atoi(com_argv[2]);
	nand_in.dev = atoi(com_argv[3]);
	if (atoi(com_argv[4]) >= MAX_DEV_NUM) {
		printf("\n Flash index number overflow!");
		return -1;
	}
	(nand_in.cs_map)[atoi(com_argv[4])] = 1;

	if (nand_erase(&nand_in) < 1)
		return -1;

	return 1;
}

int command_interpret(char * com_buf)
{
	char *buf = com_buf;
	int k, L, i = 0, j = 0;
	
	L = (int)strlen(buf);
	buf[L]=' ';
	for (k = 0; k <= L; k++) {
		if (*buf == ' ' || *buf == '\n') {
			while ( *(++buf) == ' ' );
			com_argv[i][j] = '\0';
			i++;
			if (i > MAX_ARGC) {
				printf("\n Para is too much! About!");
				return 0;
			}
			j=0;
			continue;
		} else {
			com_argv[i][j] = *buf;
			j++;
			if (j > MAX_COMMAND_LENGTH) {
				printf("\n Para is too long! About!");
				return 0;
			}
		}
		buf++;
	}

	com_argc = i;

	for (i = 1; i <= COMMAND_NUM; i++) 
		if (!strcmp(COMMAND[i], com_argv[0])) 
			return i;
	return COMMAND_NUM + 1;
}

int command_handle(char *buf)
{
	int cmd = command_interpret(buf); /* get the command index */

	if (!cmd)
		return -1;

	switch (cmd) {
	case 6:
		nand_query();
		break;
	case 7:	
		handle_nerase();
		break;
	case 11:
		nand_prog();
		break;
	case 12:
		handle_help();
		break;
	case 13:
		handle_version();
		break;
	case 16:		/* exit */
		printf("\n exiting inflash software\n");
		return -1;	/* return -1 to break the main.c while
				 * then run usb_ingenic_cleanup*/
	case 20:
		boot(STAGE1_FILE_PATH, STAGE2_FILE_PATH);
		break;
	default:
		printf("\n Command not support!");
		break;
	}

	return 1;
}
