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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb_boot_defines.h"
#include "ingenic_usb.h"
#include "cmd.h"
#include "xburst-tools_version.h"
 
extern struct nand_in nand_in;
extern struct sdram_in sdram_in;
extern unsigned char code_buf[4 * 512 * 1024];

int com_argc;
char com_argv[MAX_ARGC][MAX_COMMAND_LENGTH];

static const char COMMAND[][COMMAND_NUM]=
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
	"nmark",
	"nmake",
	"load",
	"memtest",
	"run"
};

static int handle_help(void)
{
	printf(" command support in current version:\n"
	/* " query" */
	/* " querya" */
	/* " erase" */
	/* " read" */
	/* " prog" */
	" nquery     query NAND flash info\n"
	" nerase     erase NAND flash\n"
	" nread      read NAND flash data with checking bad block and ECC\n"
	" nreadraw   read NAND flash data without checking bad block and ECC\n"
	" nreadoo    read NAND flash oob without checking bad block and ECC\n" /* index 10 */
	" nprog      program NAND flash with data and ECC\n"
	" help       print this help\n"
	" version    show current USB Boot software version\n"
	" go         execute program in SDRAM\n"
	" fconfig    set USB Boot config file(not implement)\n"
	" exit       quit from telnet session\n"
	" readnand   read data from nand flash and store to SDRAM\n"
	" gpios      set one GPIO to high level\n"
	" gpioc      set one GPIO to low level\n"
	" boot       boot device and make it in stage2\n" /* index 20 */
	" list       show current device number can connect(not implement)\n"
	/* " select" */
	/* " unselect" */
	/* " chip" */
	/* " unchip" */
	" nmark      mark a bad block in NAND flash\n"
	" nmake      read all data from nand flash and store to file(not implement)\n"
	" load       load file data to SDRAM\n"
	" memtest    do SDRAM test\n"
	" run        run command script in file(implement by -c args)\n"
	" sdprog     program SD card(not implement)\n"
	" sdread     read data from SD card(not implement)\n");
	return 1;
}

static int handle_version(void)
{
	printf(" USB Boot Software current version: %s\n", XBURST_TOOLS_VERSION);
	return 1;
}

/* need transfer two para :blk_num ,start_blk */
int handle_nerase(void)
{
	if (com_argc < 5) {
		printf(" Usage: nerase (1) (2) (3) (4)\n"
		       " 1:start block number\n"
		       " 2:block length\n"
		       " 3:device index number\n"
		       " 4:flash chip index number\n");
		return -1;
	}

	init_nand_in();

	nand_in.start = atoi(com_argv[1]);
	nand_in.length = atoi(com_argv[2]);
	nand_in.dev = atoi(com_argv[3]);
	if (atoi(com_argv[4]) >= MAX_DEV_NUM) {
		printf(" Flash index number overflow!\n");
		return -1;
	}
	(nand_in.cs_map)[atoi(com_argv[4])] = 1;

	if (nand_erase(&nand_in) < 1)
		return -1;

	return 1;
}

int handle_nmark(void)
{
	if (com_argc < 4) {
		printf(" Usage: nerase (1) (2) (3)\n"
		       " 1:bad block number\n"
		       " 2:device index number\n"
		       " 3:flash chip index number\n");
		return -1;
	}
	init_nand_in();

	nand_in.start = atoi(com_argv[1]);
	nand_in.dev = atoi(com_argv[2]);

	if (atoi(com_argv[3])>=MAX_DEV_NUM) {
		printf(" Flash index number overflow!\n");
		return -1;
	}
	(nand_in.cs_map)[atoi(com_argv[3])] = 1;

	nand_markbad(&nand_in);
	return 1;
}

int handle_memtest(void)
{
	unsigned int start, size;
	if (com_argc != 2 && com_argc != 4)
	{
		printf(" Usage: memtest (1) [2] [3]\n"
		       " 1:device index number\n"
		       " 2:SDRAM start address\n"
		       " 3:test size\n");
		return -1;
	}

	if (com_argc == 4) {
		start = strtoul(com_argv[2], NULL, 0);
		size = strtoul(com_argv[3], NULL, 0);
	} else {
		start = 0;
		size = 0;
	}
	debug_memory(atoi(com_argv[1]), start, size);
	return 1;
}

int handle_gpio(int mode)
{
	if (com_argc < 3) {
		printf(" Usage:"
		       " gpios (1) (2)\n"
		       " 1:GPIO pin number\n"
		       " 2:device index number\n");
		return -1;
	}

	debug_gpio(atoi(com_argv[2]), mode, atoi(com_argv[1]));
	return 1;
}

int handle_load(void)
{
	if (com_argc<4) {
		printf(" Usage:"
		       " load (1) (2) (3) \n"
		       " 1:SDRAM start address\n"
		       " 2:image file name\n"
		       " 3:device index number\n");

		return -1;
	}

	sdram_in.start=strtoul(com_argv[1], NULL, 0);
	printf(" start:::::: 0x%x\n", sdram_in.start);

	sdram_in.dev = atoi(com_argv[3]);
	sdram_in.buf = code_buf;
	sdram_load_file(&sdram_in, com_argv[2]);
	return 1;
}

int command_interpret(char * com_buf)
{
	char *buf = com_buf;
	int k, L, i = 0, j = 0;
	
	L = (int)strlen(buf);
	buf[L]=' ';

	if (buf[0] == '\n')
		return 0;

	for (k = 0; k <= L; k++) {
		if (*buf == ' ' || *buf == '\n') {
			while ( *(++buf) == ' ' );
			com_argv[i][j] = '\0';
			i++;
			if (i > MAX_ARGC)
				return COMMAND_NUM + 1;
			j = 0;
			continue;
		} else {
			com_argv[i][j] = *buf;
			j++;
			if (j > MAX_COMMAND_LENGTH)
				return COMMAND_NUM + 1;
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

	switch (cmd) {
	case 0:
		break;
	case 6:
		nand_query();
		break;
	case 7:	
		handle_nerase();
		break;
	case 8:	/* nread */
		nand_read(NAND_READ);
		break;
	case 9:	/* nreadraw */
		nand_read(NAND_READ_RAW);
		break;
	case 10: /* nreadoob */
		nand_read(NAND_READ_OOB);
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
	case 14:
		debug_go();
		break;
	case 16:		/* exit */
		printf(" exiting usbboot software\n");
		return -1;	/* return -1 to break the main.c while
				 * then run usb_ingenic_cleanup*/
		/*case 17:
		nand_read(NAND_READ_TO_RAM); */
		break;
	case 18:
		handle_gpio(2);
		break;
	case 19:
		handle_gpio(3);
		break;
	case 20:
		boot(STAGE1_FILE_PATH, STAGE2_FILE_PATH);
		break;
	case 26:
		handle_nmark();
		break;
	case 28:
		handle_load();
		break;
	case 29:
		handle_memtest();
		break;
	default:
		printf(" command not support or input error!\n");
		break;
	}

	return 1;
}
