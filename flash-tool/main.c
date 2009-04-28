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

static void help(void)
{
	printf("Usage: inflash [options] ...(must run as root)\n"
		"  -h --help\t\t\tPrint this help message\n"
		"  -v --version\t\t\tPrint the version number\n"
		"  -c --cfg \t\t\tSpecify the Configuration of inflash device\n"
		"  -o --stageone \t\tSpecify the stage one file(default ./bw.bin)\n"
		"  -t --stagetwo \t\tSpecify the stage two file(default ./usb_boot.bin)\n"
		);
}

static void print_version(void)
{
	printf("inflash version \n");
}

static struct option opts[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'v' },
	{ "cfg", 1, 0, 'c' },
	{ "stageone", 1, 0, 'o' },
	{ "stagetwo", 1, 0, 't' },
	{ NULL, 0, 0, NULL }
};

int main(int argc, char **argv)
{
	printf("inflash - (C) 2009\n"
	       "This program is Free Software and has ABSOLUTELY NO WARRANTY\n\n");

	if ((getuid()) || (getgid())) {
		fprintf(stderr, "Error - you must be root to run '%s'\n", argv[0]);
		return -1;
	}

	char *stage1_path = STAGE1_FILE_PATH;
	char *stage2_path = STAGE2_FILE_PATH;
	char *config_path = CONFIG_FILE_PATH;

	while (1) {
		int c, option_index = 0;
		c = getopt_long(argc, argv, "hvc:o:t:", opts,
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
		case 'c':
			/* Configuration */
			config_path = optarg;
			break;
		case 'o':
			stage1_path = optarg;
			break;
		case 't':
			stage2_path = optarg;
			break;
		default:
			help();
			exit(2);
		}
	}

#if 0
	while (1) {
		printf("\n USBBoot :> ");
		if (!command_input(com_buf)) continue;
		command_handle(com_buf);
	}
#endif

	boot(stage1_path, stage2_path, config_path);

	return 0;
}

