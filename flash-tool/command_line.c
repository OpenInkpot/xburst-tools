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
 
static int com_argc;
static char com_argv[9][100];

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
	"nreadoob",
	"nprog",
	"help",
	"version",
	"go",
	"fconfig",
	"exit",
	"readnand",
	"gpios",
	"gpioc",
	"boot",
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

void handle_exit(int res)
{
	printf("\n exiting inflash software\n");
	exit(res);
}

int handle_nprog(void)
{
	printf("\n not implement");
	return 1;
}

int handle_help(void)
{
	printf("\n Command support in current version:");
	printf("\n help          print this help;");
	printf("\n boot          boot device and make it in stage2;");
	printf("\n list          show current device number can connect;");
	printf("\n fconfig       set USB Boot config file;");
	printf("\n nquery        query NAND flash info;");
	printf("\n nread         read NAND flash data with checking bad block and ECC;");
	printf("\n nreadraw      read NAND flash data without checking bad block and ECC;");
	printf("\n nreadoob      read NAND flash oob without checking bad block and ECC;");
	printf("\n nerase        erase NAND flash;");
	printf("\n nprog         program NAND flash with data and ECC;");
	printf("\n nmark         mark a bad block in NAND flash;");
	printf("\n go            execute program in SDRAM;");
	printf("\n version       show current USB Boot software version;");
	printf("\n exit          quit from telnet session;");
	printf("\n readnand      read data from nand flash and store to SDRAM;");
	printf("\n load          load file data to SDRAM;");
	printf("\n run           run command script in file;");
	printf("\n memtest       do SDRAM test;");
	printf("\n gpios         let one GPIO to high level;");
	printf("\n gpioc         let one GPIO to low level;");
	/* printf("\n nmake         read all data from nand flash and store to file(experimental);"); */
	return 1;
}

int handle_version(void)
{
	printf("\n USB Boot Software current version: %s", CURRENT_VERSION);	
	return 1;
}

int handle_fconfig(void)
{
	if (com_argc < 3) {
		printf("\n Usage:");
		printf(" fconfig (1) (2) ");
		printf("\n 1:configration file name \
			    \n 2:deivce index number ");
		return -1;
	}
	/* usb_infenic_config(atoi(com_argv[2]),com_argv[1]); */
	return 1;
}

int handle_boot(void)
{
	return boot(STAGE1_FILE_PATH, STAGE2_FILE_PATH, CONFIG_FILE_PATH);
}

int command_input(char *buf)
{
	char *cptr;
	cptr = fgets(buf, 256, stdin);

        if (cptr != NULL) 
		return 1;
	return 0;
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
			if (i > 9) {
				printf("\n Para is too much! About!");
				return 0;
			}
			j=0;
			continue;
		} else {
			com_argv[i][j] = *buf;
			j++;
			if (j > 100) {
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
	int cmd = command_interpret(buf);

	if (!cmd) return -1;
	switch (cmd) {
	case 11:
		handle_nprog();
		break;
	case 12:
		handle_help();
		break;
	case 13:
		handle_version();
		break;
	case 16:
		handle_exit(0);
		break;
	case 20:
		handle_boot();
		break;
	default:
		printf("\n Command not support!");
		result = -1;
		break;
	}

	return 1;
}
