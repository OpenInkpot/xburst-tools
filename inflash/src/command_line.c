/*
 * Authors: Xiangfu Liu <xiangfu.z@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb_boot_defines.h"
#include "ingenic_usb.h"
#include "cmd.h"
#include "inflash_version.h"
 
extern struct nand_in nand_in;
int com_argc;
char com_argv[MAX_ARGC][MAX_COMMAND_LENGTH];
const char HEX_NUM[17]={'0','1','2','3','4','5','6','7','8','9',
			'a','b','c','d','e','f',' '};

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
	       " help          print this help;\n"
	       " boot          boot device and make it in stage2;\n"
	       " list          show current device number can connect;\n"
	       " fconfig       set USB Boot config file;\n"
	       " nquery        query NAND flash info;\n"
	       " nread         read NAND flash data with checking bad block and ECC;\n"
	       " nreadraw      read NAND flash data without checking bad block and ECC;\n"
	       " nreadoob      read NAND flash oob without checking bad block and ECC;\n"
	       " nerase        erase NAND flash;\n"
	       " nprog         program NAND flash with data and ECC;\n"
	       " nmark         mark a bad block in NAND flash;\n"
	       " go            execute program in SDRAM;\n"
	       " version       show current USB Boot software version;\n"
	       " exit          quit from telnet session;\n"
	       " readnand      read data from nand flash and store to SDRAM;\n"
	       " load          load file data to SDRAM;\n"
	       " run           run command script in file;\n"
	       " memtest       do SDRAM test;\n"
	       " gpios         let one GPIO to high level;\n"
	       " gpioc         let one GPIO to low level;\n");
	/* printf(" nmake         read all data from nand flash and store to file(experimental);\n"); */
	return 1;
}

unsigned int hex2dec(char *s)
{
	int i,L=(int)strlen(s),j;
	unsigned int temp=0;
	if (L>8) L=8;
	for (i = 0; i < L; i++) {
		for (j = 0; j < 16; j++)
			if (s[i] == HEX_NUM[j]) 
				break;

		if (j == 16) 
			return 0;
		temp = temp * 16 + j;
	}
	return temp;
}

static int handle_version(void)
{
	printf(" USB Boot Software current version: %s\n", INFLASH_VERSION);
	return 1;
}

/* need transfer two para :blk_num ,start_blk */
int handle_nerase(void)
{
	if (com_argc < 5) {
		printf(" Usage:\n");
		printf(" nerase (1) (2) (3) (4) ");
		printf(" 1:start block number\n"
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
	int i;
	if (com_argc < 4) {
		printf(" Usage:\n");
		printf(" nerase (1) (2) (3) ");
		printf(" 1:bad block number\n"
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
		printf(" Usage:\n");
		printf(" memtest (1) [2] [3] ");
		printf(" 1:device index number\n"
		       " 2:SDRAM start address\n"
		       " 3:test size\n");
		return -1;
	}

	if (com_argc == 4) {
		if (com_argv[2][0]=='0'&&com_argv[2][1]=='x') 
			start=hex2dec(&com_argv[2][2]);
		else start=atol(com_argv[2]);

		if (com_argv[3][0]=='0'&&com_argv[3][1]=='x') 
			size = hex2dec(&com_argv[3][2]);
		else size = atol(com_argv[3]);
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
		printf(" Usage:\n"
		       " gpios (1) (2) "
		       " 1:GPIO pin number\n"
		       " 2:device index number\n");
		return -1;
	}

	debug_gpio(atoi(com_argv[2]), mode, atoi(com_argv[1]));
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
	case 16:		/* exit */
		printf(" exiting inflash software\n");
		return -1;	/* return -1 to break the main.c while
				 * then run usb_ingenic_cleanup*/
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
	case 29:
		handle_memtest();
		break;
	default:
		printf(" command not support or input error!\n");
		break;
	}

	return 1;
}
