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
#include <getopt.h>
#include "config.h"
#include "command_line.h"
#include "ingenic_usb.h"
#include "ingenic_cfg.h"

extern struct ingenic_dev ingenic_dev;
extern struct hand hand;

static void help(void)
{
	printf("Usage: inflash [options] ...(must run as root)\n"
	       "  -h --help\t\t\tPrint this help message\n"
	       "  -v --version\t\t\tPrint the version number\n"
	       "  -c --command\t\t\tDirect run the command\n"
	       "Report bugs to <xiangfu.z@gmail.com>."
		);
}

static void print_version(void)
{
	printf("inflash version: %s\n", CURRENT_VERSION);
}

static struct option opts[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'v' },
	{ "command", 1, 0, 'c' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
	printf(" inflash - (C) 2009"
	       "\n Ingenic Tools Software!"
	       "\n This program is Free Software and has ABSOLUTELY NO WARRANTY\n");

	int command = 0;
	char *cptr;
	char com_buf[256];
	memset(com_buf, 0, 256);

	while(1) {
		int c, option_index = 0;
		c = getopt_long(argc, argv, "hvc:", opts,
				&option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			help();
			exit(EXIT_SUCCESS);
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 'c':
			command = 1;
			strcpy(com_buf, optarg);
			break;
		default:
			help();
			exit(2);
		}
	}

	if ((getuid()) || (getgid())) {
		fprintf(stderr, "Error - you must be root to run '%s'\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (usb_ingenic_init(&ingenic_dev) < 1)
	 	return EXIT_FAILURE;

	if (parse_configure(&hand, CONFIG_FILE_PATH) < 1)
		return EXIT_FAILURE;

	if (command) {		/* direct run command */
		command_handle(com_buf);
		printf("\n");
		goto out;
	}

	while (1) {
		printf("\n inflash :> ");
		cptr = fgets(com_buf, 256, stdin);
		if (cptr == NULL) 
			continue;

		if (command_handle(com_buf) == -1 )
			break;
	}

out:
	usb_ingenic_cleanup(&ingenic_dev);
	return EXIT_SUCCESS;
}
