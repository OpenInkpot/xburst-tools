/*
 * "Ingenic flash tool" - flash the Ingenic CPU via USB
 *
 * (C) Copyright 2009
 * Author: Marek Lindner <lindner_marek@yahoo.de>
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



#include "main.h"
#include "usb.h"
#include "usb_boot_defines.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <confuse.h>

fw_args_t fw_args;

static int parse_configure(char * file_path)
{
	cfg_opt_t opts[] = {
		CFG_SIMPLE_INT("EXTCLK", &fw_args.ext_clk),
		CFG_SIMPLE_INT("CPUSPEED", &fw_args.cpu_speed),
		CFG_SIMPLE_INT("PHMDIV", &fw_args.phm_div),
		CFG_SIMPLE_INT("BOUDRATE", &fw_args.boudrate),
		CFG_SIMPLE_INT("USEUART", &fw_args.use_uart),

		CFG_SIMPLE_INT("BUSWIDTH", &fw_args.bus_width),
		CFG_SIMPLE_INT("BANKS", &fw_args.bank_num),
		CFG_SIMPLE_INT("ROWADDR", &fw_args.row_addr),
		CFG_SIMPLE_INT("COLADDR", &fw_args.col_addr),

		CFG_SIMPLE_INT("ISMOBILE", &fw_args.is_mobile),
		CFG_SIMPLE_INT("ISBUSSHARE", &fw_args.is_busshare),
		CFG_SIMPLE_INT("DEBUGOPS", &fw_args.debug_ops),
		CFG_SIMPLE_INT("PINNUM", &fw_args.pin_num),
		CFG_SIMPLE_INT("START", &fw_args.start),
		CFG_SIMPLE_INT("SIZE", &fw_args.size),

		CFG_END()
	};

	cfg_t *cfg;
	cfg = cfg_init(opts, 0);
	if (cfg_parse(cfg, file_path) == CFG_PARSE_ERROR)
		return -1;
	cfg_free(cfg);

	total_size = (unsigned int)(2 << (fw_args.row_addr + fw_args.col_addr - 1)) * 2 
		* (fw_args.bank_num + 1) * 2 
		* (2 - fw_args.bus_width);

	return 1;
}

static int load_file(struct ingenic_dev *ingenic_dev, const char *file_path)
{
	struct stat fstat;
	int fd, status, res = -1;

	if (ingenic_dev->file_buff)
		free(ingenic_dev->file_buff);

	ingenic_dev->file_buff = NULL;

	status = stat(file_path, &fstat);

	if (status < 0) {
		fprintf(stderr, "Error - can't get file size from '%s': %s\n", file_path, strerror(errno));
		goto out;
	}

	ingenic_dev->file_len = fstat.st_size;
	ingenic_dev->file_buff = malloc(ingenic_dev->file_len);

	if (!ingenic_dev->file_buff) {
		fprintf(stderr, "Error - can't allocate memory to read file '%s': %s\n", file_path, strerror(errno));
		return -1;
	}

	fd = open(file_path, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "Error - can't open file '%s': %s\n", file_path, strerror(errno));
		goto out;
	}

	status = read(fd, ingenic_dev->file_buff, ingenic_dev->file_len);

	if (status < ingenic_dev->file_len) {
		fprintf(stderr, "Error - can't read file '%s': %s\n", file_path, strerror(errno));
		goto close;
	}

	memcpy(ingenic_dev->file_buff + 8, &fw_args, sizeof(fw_args_t));

	res = 1;

close:
	close(fd);
out:
	return res;
}

int main(int argc, char **argv)
{
	struct ingenic_dev ingenic_dev;

	int res = EXIT_FAILURE;

	if ((getuid()) || (getgid())) {
		fprintf(stderr, "Error - you must be root to run '%s'\n", argv[0]);
		goto out;
	}

	memset(&ingenic_dev, 0, sizeof(struct ingenic_dev));
	memset(&fw_args, 0, sizeof(fw_args_t));

	if (parse_configure(CONFIG_FILE_PATH) < 1)
		goto out;

	if (usb_ingenic_init(&ingenic_dev) < 1)
		goto out;

	if (usb_get_ingenic_cpu(&ingenic_dev) < 1)
		goto out;

	/* now we upload the usb boot stage1 */
	if (load_file(&ingenic_dev, STAGE1_FILE_PATH) < 1)
		goto out;

	if (usb_ingenic_upload(&ingenic_dev, 1) < 1)
		goto cleanup;
#if 0
	/* now we upload the usb boot stage2 */
	sleep(1);
	if (load_file(&ingenic_dev, STAGE2_FILE_PATH) < 1)
		goto cleanup;

	if (usb_ingenic_upload(&ingenic_dev, 2) < 1)
		goto cleanup;

#endif
	res = EXIT_SUCCESS;

cleanup:
	if (ingenic_dev.file_buff)
		free(ingenic_dev.file_buff);
out:
	usb_ingenic_cleanup(&ingenic_dev);
	return res;
}

