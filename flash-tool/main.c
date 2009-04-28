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
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "usb.h"
#include "config.h"
#include "command_line.h"

static void help(void)
{
	printf("Usage: inflash [options] ...(must run as root)\n"
		"  -h --help\t\t\tPrint this help message\n"
		"  -v --version\t\t\tPrint the version number\n"
		);
}

static void print_version(void)
{
	printf("inflash version: %s\n", CURRENT_VERSION);
}

static struct option opts[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'v' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
	printf("inflash - (C) 2009\n"
	       "This program is Free Software and has ABSOLUTELY NO WARRANTY\n\n");

	if ((getuid()) || (getgid())) {
		fprintf(stderr, "Error - you must be root to run '%s'\n", argv[0]);
		return -1;
	}

	while (1) {
		int c, option_index = 0;
		c = getopt_long(argc, argv, "hv", opts,
				&option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			help();
			exit(0);
			break;
		case 'v':
			print_version();
			exit(0);
			break;
		default:
			help();
			exit(2);
		}
	}

	char com_buf[256];
	printf("\n Welcome!");
	printf("\n USB Boot Host Software!");

	while (1) {
		printf("\n inflash :> ");
		if (command_input(com_buf)) 
			continue;
		command_handle(com_buf);
	}

	return 0;
}

