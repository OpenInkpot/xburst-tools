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

#include "ingenic_usb.h"
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

struct nand_in nand_in;
struct nand_out nand_out;
struct fw_args_t fw_args;
unsigned int total_size;

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

	fw_args.cpu_id = 0x4740;
	if (fw_args.bus_width == 32)
		fw_args.bus_width = 0 ;
	else
		fw_args.bus_width = 1 ; 
	fw_args.bank_num = fw_args.bank_num / 4; 
	fw_args.cpu_speed = fw_args.cpu_speed / fw_args.ext_clk;

	total_size = (unsigned int)
		(2 << (fw_args.row_addr + fw_args.col_addr - 1)) * 2 
		* (fw_args.bank_num + 1) * 2 
		* (2 - fw_args.bus_width);

	return 1;
}

int check_dump_cfg()
{
	printf("\n Now checking whether all configure args valid: ");
	/* check PLL */
	if (fw_args.ext_clk > 27 || fw_args.ext_clk < 12 ) {
		printf("\n EXTCLK setting invalid!");
		return 0;
	}
	if (fw_args.phm_div > 32 || fw_args.ext_clk < 2 ) {
		printf("\n PHMDIV setting invalid!");
		return 0;
	}
	if ( (fw_args.cpu_speed * fw_args.ext_clk ) % 12 != 0 ) {
		printf("\n CPUSPEED setting invalid!");
		return 0;
	}

	/* check SDRAM */
	if (fw_args.bus_width > 1 ) {
		printf("\n SDRAMWIDTH setting invalid!");
		return 0;
	}
	if (fw_args.bank_num > 1 ) {
		printf("\n BANKNUM setting invalid!");
		return 0;
	}
	if (fw_args.row_addr > 13 && fw_args.row_addr < 11 ) {
		printf("\n ROWADDR setting invalid!");
		return 0;
	}
	if (fw_args.col_addr > 13 && fw_args.col_addr < 11 ) {
		printf("\n COLADDR setting invalid!");
		return 0;
	}

	/* check NAND */
	/* if ( Hand.nand_ps < 2048 && Hand.nand_os > 16 )
	{
		printf("\n PAGESIZE or OOBSIZE setting invalid!");
		return 0;
	}
	if ( Hand.nand_ps < 2048 && Hand.nand_ppb > 32 )
	{
		printf("\n PAGESIZE or PAGEPERBLOCK setting invalid!");
		return 0;
	}

	if ( Hand.nand_ps > 512 && Hand.nand_os <= 16 )
	{
		printf("\n PAGESIZE or OOBSIZE setting invalid!");
		return 0;
	}
	if ( Hand.nand_ps > 512 && Hand.nand_ppb < 64 )
	{
		printf("\n PAGESIZE or PAGEPERBLOCK setting invalid!");
		return 0;
		} */

	printf("\n Current device information:");
	printf(" CPU is Jz%x",fw_args.cpu_id);
	printf("\n Crystal work at %dMHz, the CCLK up to %dMHz and PMH_CLK up to %dMHz",
		fw_args.ext_clk,
		(unsigned int)fw_args.cpu_speed * fw_args.ext_clk,
		((unsigned int)fw_args.cpu_speed * fw_args.ext_clk) / fw_args.phm_div);

	printf("\n Total SDRAM size is %d MB, work in %d bank and %d bit mode",
		total_size / 0x100000, 2 * (fw_args.bank_num + 1), 16 * (2 - fw_args.bus_width));

	/* printf("\n Nand page size %d, ECC offset %d, ",
		Hand.nand_ps,Hand.nand_eccpos);

	printf("bad block ID %d, ",Hand.nand_bbpage);

	printf("use %d plane mode",Hand.nand_plane); */

	printf("\n");
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

	memcpy(ingenic_dev->file_buff + 8, &fw_args, sizeof(struct fw_args_t));

	res = 1;

close:
	close(fd);
out:
	return res;
}

int boot(char *stage1_path, char *stage2_path, char *config_path ){
	struct ingenic_dev ingenic_dev;

	int res = 0;
	int status;

	memset(&ingenic_dev, 0, sizeof(struct ingenic_dev));
	memset(&fw_args, 0, sizeof(struct fw_args_t));

	if (parse_configure(config_path) < 1)
		goto out;

	if (usb_ingenic_init(&ingenic_dev) < 1)
		goto out;

	status = usb_get_ingenic_cpu(&ingenic_dev);
	switch (status)	{
	case 1:            /* Jz4740v1 */
		status = 0;
		fw_args.cpu_id = 0x4740;
		break;
	case 2:            /* Jz4750v1 */
		status = 0;
		fw_args.cpu_id = 0x4750;
		break;
	case 3:            /* Boot4740 */
		status = 1;
		fw_args.cpu_id = 0x4740;
		break;
	case 4:            /* Boot4750 */
		status = 1;
		fw_args.cpu_id = 0x4750;
		break;
	default:
		goto out;
	}

	if (status) {
		printf("Booted");
		goto out;
	} else {
		printf("Unboot");
		printf("\n Now booting device");

		/* now we upload the usb boot stage1 */
		printf("\n Upload usb boot stage1");
		if (load_file(&ingenic_dev, stage1_path) < 1)
			goto out;

		if (usb_ingenic_upload(&ingenic_dev, 1) < 1)
			goto cleanup;

		/* now we upload the usb boot stage2 */
		sleep(1);
		printf("\n Upload usb boot stage2");
		if (load_file(&ingenic_dev, stage2_path) < 1)
			goto cleanup;

		if (usb_ingenic_upload(&ingenic_dev, 2) < 1)
			goto cleanup;
		check_dump_cfg();
	}

	res = 1;

cleanup:
	if (ingenic_dev.file_buff)
		free(ingenic_dev.file_buff);
out:
	usb_ingenic_cleanup(&ingenic_dev);
	return res;
}
int nprog(int com_argc, char **com_argv)
{
#if 0
	unsigned int i;
	if (com_argc < 6)
	{
		printf("\n Usage:"
		       " nprog (1) (2) (3) (4) (5) "
		       "\n 1:start page number"
		       "\n 2:image file name"
		       "\n 3:device index number"
		       "\n 4:flash index number"
		       "\n 5:image type  -n:no oob,-o:with oob no ecc,-e:with oob and ecc");

		return 0;
	}
	for (i = 0; i < MAX_DEV_NUM; i++)
		(nand_in.cs_map)[i] = 0;
	if (atoi(com_argv[4]) >= MAX_DEV_NUM) {
		printf("\n Flash index number overflow!");
		return -1;
	}
	(nand_in.cs_map)[atoi(com_argv[4])] = 1;
	nand_in.start = atoi(com_argv[1]);
	nand_in.dev = atoi(com_argv[3]);

	if (!strcmp(com_argv[5],"-e")) 
		nand_in.option = OOB_ECC;
	else if (!strcmp(com_argv[5],"-o")) 
		nand_in.option = OOB_NO_ECC;
	else 
		nand_in.option = NO_OOB;

	if (Hand.nand_plane > 1)
		/* API_Nand_Program_File_Planes(&nand_in,&nand_out,com_argv[2]); */
	else
		/* API_Nand_Program_File(&nand_in,&nand_out,com_argv[2]); */
#if 0
		printf("\n Flash check result:");
	for (i = 0; i < 16; i++)
		printf(" %d", (nand_out.status)[i]);
#endif
#endif 
	printf("\n not implement yet!!");
	return 1;
}
