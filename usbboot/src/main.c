/*
 * Authors: Xiangfu Liu <xiangfu@qi-hardware.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include "xburst-tools_version.h"
#include "command_line.h"
#include "ingenic_usb.h"
#include "ingenic_cfg.h"

extern struct ingenic_dev ingenic_dev;
extern struct hand hand;

static void help(void)
{
	printf("Usage: usbboot [options] ...(must run as root)\n"
	       "  -h --help\t\t\tPrint this help message\n"
	       "  -v --version\t\t\tPrint the version number\n"
	       "  -c --command\t\t\tDirect run the commands, split by ';'\n"
	       "  <run without options to enter commands via usbboot prompt>\n\n"
	       "Report bugs to <xiangfu@qi-hardware.com>.\n"
		);
}

static void print_version(void)
{
	printf("usbboot version: %s\n", XBURST_TOOLS_VERSION);
}

static struct option opts[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'v' },
	{ "command", 1, 0, 'c' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
	int command = 0;
	char *cptr;
	char com_buf[256] = {0};
	char *cmdpt;

	printf("usbboot - Ingenic XBurst USB Boot Utility\n"
	       "(c) 2009 Ingenic Semiconductor Inc., Qi Hardware Inc., Xiangfu Liu, Marek Lindner\n"
	       "This program is Free Software and comes with ABSOLUTELY NO WARRANTY.\n\n");

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
			cmdpt = optarg;
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
		char *delim=";";
		char *p;
		p = strtok(cmdpt, delim);
		strcpy(com_buf, p);
		printf(" Execute command: %s \n",com_buf);
		command_handle(com_buf);

		while((p = strtok(NULL,delim))) {
			strcpy(com_buf, p);
			printf(" Execute command: %s \n",com_buf);
			command_handle(com_buf);
		}
		goto out;
	}

	while (1) {
		printf("usbboot :> ");
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
