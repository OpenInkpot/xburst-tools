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
 
#define MAX_ARGC	10
#define MAX_ARGV_LENGTH	100
static int com_argc;
static char com_argv[MAX_ARGC][MAX_ARGV_LENGTH];

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

int handle_help(void)
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

int handle_version(void)
{
	printf("\n USB Boot Software current version: %s", CURRENT_VERSION);	
	return 1;
}

int handle_fconfig(void)
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
			if (i > MAX_ARGC) {
				printf("\n Para is too much! About!");
				return 0;
			}
			j=0;
			continue;
		} else {
			com_argv[i][j] = *buf;
			j++;
			if (j > MAX_ARGV_LENGTH) {
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
		return nprog(com_argc, com_argv);
	case 12:
		handle_help();
		break;
	case 13:
		handle_version();
		break;
	case 16:		/* exit */
		printf("\n exiting inflash software\n");
		exit(EXIT_SUCCESS);
	case 20:
		return boot(STAGE1_FILE_PATH, STAGE2_FILE_PATH, CONFIG_FILE_PATH);
	default:
		printf("\n Command not support!");
		return -1;
	}

	return 1;
}
