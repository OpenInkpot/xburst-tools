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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


int load_file(struct ingenic_dev *ingenic_dev, const char *file_path)
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

	if (usb_ingenic_init(&ingenic_dev) < 1)
		goto out;

	if (usb_get_ingenic_cpu(&ingenic_dev) < 1)
		goto out;

	if (load_file(&ingenic_dev, STAGE1_FILE_PATH) < 1)
		goto out;

	if (usb_ingenic_upload(&ingenic_dev, 1) < 1)
		goto cleanup;

	res = EXIT_SUCCESS;

cleanup:
	if (ingenic_dev.file_buff)
		free(ingenic_dev.file_buff);
out:
	usb_ingenic_cleanup(&ingenic_dev);
	return res;
}

