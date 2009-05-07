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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include "cmd.h"
#include "ingenic_cfg.h"
#include "ingenic_usb.h"
#include "usb_boot_defines.h"

extern int com_argc;
extern char com_argv[MAX_ARGC][MAX_COMMAND_LENGTH];

struct ingenic_dev ingenic_dev;
struct hand_t hand;

static struct nand_in_t nand_in;
static struct nand_out_t nand_out;
unsigned int total_size;
unsigned char code_buf[4 * 512 * 1024];
unsigned char check_buf[4 * 512 * 1024];
unsigned char cs[16];
unsigned char ret[8];

static const char IMAGE_TYPE[][30] = {
	"with oob and ecc",
	"with oob and without ecc",
	"without oob",
};

static int load_file(struct ingenic_dev *ingenic_dev, const char *file_path)
{
	struct stat fstat;
	int fd, status, res = -1;

	if (ingenic_dev->file_buff)
		free(ingenic_dev->file_buff);

	ingenic_dev->file_buff = NULL;

	status = stat(file_path, &fstat);

	if (status < 0) {
		fprintf(stderr, "Error - can't get file size from '%s': %s\n",
			file_path, strerror(errno));
		goto out;
	}

	ingenic_dev->file_len = fstat.st_size;
	ingenic_dev->file_buff = malloc(ingenic_dev->file_len);

	if (!ingenic_dev->file_buff) {
		fprintf(stderr, "Error - can't allocate memory to read file '%s': %s\n", 
			file_path, strerror(errno));
		return -1;
	}

	fd = open(file_path, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "Error - can't open file '%s': %s\n", 
			file_path, strerror(errno));
		goto out;
	}

	status = read(fd, ingenic_dev->file_buff, ingenic_dev->file_len);

	if (status < ingenic_dev->file_len) {
		fprintf(stderr, "Error - can't read file '%s': %s\n", 
			file_path, strerror(errno));
		goto close;
	}

	memcpy(ingenic_dev->file_buff + 8, &hand.fw_args, sizeof(struct fw_args_t));

	res = 1;

close:
	close(fd);
out:
	return res;
}

int boot(char *stage1_path, char *stage2_path){

	int res = 0;
	int status;

	status = usb_get_ingenic_cpu(&ingenic_dev);
	switch (status)	{
	case 1:            /* Jz4740v1 */
		status = 0;
		hand.fw_args.cpu_id = 0x4740;
		break;
	case 2:            /* Jz4750v1 */
		status = 0;
		hand.fw_args.cpu_id = 0x4750;
		break;
	case 3:            /* Boot4740 */
		status = 1;
		hand.fw_args.cpu_id = 0x4740;
		break;
	case 4:            /* Boot4750 */
		status = 1;
		hand.fw_args.cpu_id = 0x4750;
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
	}

	res = 1;

cleanup:
	if (ingenic_dev.file_buff)
		free(ingenic_dev.file_buff);
out:
	return res;
}

/* nand function  */
int nand_markbad(struct nand_in_t *nand_in)
{
	/* if (Handle_Open(nand_in->dev)==-1) { */
	/* 	printf("\n Can not connect device!"); */
	/* 	return -1; */
	/* } */
	if (usb_get_ingenic_cpu(&ingenic_dev) < 3) {
		printf("\n Device unboot! Boot it first!");
		return -1;
	}
	/* printf("mark bad block : %d \n",nand_in->start); */
	usb_send_data_address_to_ingenic(&ingenic_dev, nand_in->start);
	usb_ingenic_nand_ops(&ingenic_dev, NAND_MARK_BAD);
	ingenic_dev.file_buff = ret;
	ingenic_dev.file_len = 8;
	usb_read_data_from_ingenic(&ingenic_dev);
	printf("\n Mark bad block at %d ",((ret[3] << 24) | 
					   (ret[2] << 16) | 
					   (ret[1] << 8) | 
					   (ret[0] << 0)) / hand.nand_ppb);
	/* Handle_Close(); */
	return 0;
}

int nand_program_check(struct nand_in_t *nand_in,
		       struct nand_out_t *nand_out,
		       unsigned int *start_page)
{
	unsigned int i,page_num,cur_page;
	unsigned short temp;
	unsigned char status_buf[32];

	if (nand_in->length > (unsigned int)MAX_TRANSFER_SIZE) {
		printf("\n Buffer size too long!");
		return -1;
	}

#if 0
	/* nand_out->status = (unsigned char *)malloc(nand_in->max_chip * sizeof(unsigned char)); */
	nand_out->status = status_buf;

	for (i = 0; i < nand_in->max_chip; i++)
		(nand_out->status)[i] = 0; /* set all status to fail */
#endif

	if (usb_get_ingenic_cpu(&ingenic_dev) < 3) {
		printf("\n Device unboot! Boot it first!");
		return -1;
	}
	ingenic_dev.file_buff = nand_in->buf;
	ingenic_dev.file_len = nand_in->length;
	usb_send_data_to_ingenic(&ingenic_dev);
	/* WriteFile(hDevice, nand_in->buf, nand_in->length , &nWritten, NULL); */
	/* dump_data(nand_in->buf, 100); */

	/* WriteFile(hDevice, nand_in->buf, nand_in->length * hand.nand_ps, &nWritten, NULL); */
	/* Send data to be program */
	/* Only send once! */
	for (i = 0; i < nand_in->max_chip; i++) {
		if ((nand_in->cs_map)[i]==0) continue;
			/* page_num = nand_in->length / hand.nand_ps +1; */
			if (nand_in->option == NO_OOB) {
				page_num = nand_in->length / hand.nand_ps;
				if ((nand_in->length % hand.nand_ps) !=0) 
					page_num++;
				/*temp = ((i<<4) & 0xff0) + NAND_PROGRAM; */
			} else {
				page_num = nand_in->length / (hand.nand_ps + hand.nand_os);
				if ((nand_in->length% (hand.nand_ps + hand.nand_os)) !=0) 
					page_num++;
				/* temp = ((i<<4) & 0xff0) + NAND_PROGRAM_OOB; */
			}
			temp = ((nand_in->option<<12) & 0xf000)  + ((i<<4) & 0xff0) + NAND_PROGRAM;	
			usb_send_data_address_to_ingenic(&ingenic_dev, nand_in->start);
			usb_send_data_length_to_ingenic(&ingenic_dev, page_num);
			usb_ingenic_nand_ops(&ingenic_dev, temp);
			ingenic_dev.file_buff = ret;
			ingenic_dev.file_len = 8;
			usb_read_data_from_ingenic(&ingenic_dev);
			/* ReadFile(hDevice, ret, 8, &nRead, NULL); needs change */
			printf(" Finish! ");
		switch (nand_in->option)
		{
		case OOB_ECC:
			usb_send_data_address_to_ingenic(&ingenic_dev, nand_in->start);
			/* Read back to check! */
			usb_send_data_length_to_ingenic(&ingenic_dev, page_num);
			temp = ((OOB_ECC<<12) & 0xf000) +((i<<4) & 0xff0) + NAND_READ;
			usb_ingenic_nand_ops(&ingenic_dev, temp);
			printf("Checking...");			
			ingenic_dev.file_buff = check_buf;
			ingenic_dev.file_len = page_num * (hand.nand_ps + hand.nand_os);
			usb_read_data_from_ingenic(&ingenic_dev);
			ingenic_dev.file_buff = ret;
			ingenic_dev.file_len = 8;
			usb_read_data_from_ingenic(&ingenic_dev);
			/* ReadFile(hDevice, check_buf,  */
			/* 	 page_num * (hand.nand_ps + hand.nand_os), &nRead, NULL); */
			/* ReadFile(hDevice, ret, 8, &nRead, NULL); need change */
			break;
		case OOB_NO_ECC:/* do not support data verify */
			usb_send_data_address_to_ingenic(&ingenic_dev, nand_in->start);
			usb_send_data_length_to_ingenic(&ingenic_dev, page_num);
			temp = ((OOB_NO_ECC << 12) & 0xf000) + ((i << 4) & 0xff0) + NAND_READ;
			usb_ingenic_nand_ops(&ingenic_dev, temp);
			printf("Checking...");			
			ingenic_dev.file_buff = check_buf;
			ingenic_dev.file_len = page_num * (hand.nand_ps + hand.nand_os);
			usb_read_data_from_ingenic(&ingenic_dev);
			ingenic_dev.file_buff = ret;
			ingenic_dev.file_len = 8;
			usb_read_data_from_ingenic(&ingenic_dev);
			/* ReadFile(hDevice, check_buf, page_num * (hand.nand_ps + hand.nand_os), &nRead, NULL); */
			/* ReadFile(hDevice, ret, 8, &nRead, NULL); needs change*/
			break;
		case NO_OOB:
			usb_send_data_address_to_ingenic(&ingenic_dev, nand_in->start);
			usb_send_data_length_to_ingenic(&ingenic_dev, page_num);
			temp = ((NO_OOB << 12) & 0xf000) + ((i << 4) & 0xff0) + NAND_READ;
			usb_ingenic_nand_ops(&ingenic_dev, temp);
			printf("Checking...");
			ingenic_dev.file_buff = check_buf;
			ingenic_dev.file_len = page_num * (hand.nand_ps + hand.nand_os);
			usb_read_data_from_ingenic(&ingenic_dev);
			ingenic_dev.file_buff = ret;
			ingenic_dev.file_len = 8;
			usb_read_data_from_ingenic(&ingenic_dev);
			/* ReadFile(hDevice, check_buf, page_num * hand.nand_ps , &nRead, NULL); */
			/* ReadFile(hDevice, ret, 8, &nRead, NULL); need change*/
			break;
		default:;
		}
#if 1
		if (nand_in->start < 1 && hand.nand_ps == 4096 && hand.fw_args.cpu_id == 0x4740) {
			/* (nand_out->status)[i] = 1; */
			printf(" no check!");
			cur_page = (ret[3]<<24)|(ret[2]<<16)|(ret[1]<<8)|(ret[0]<<0);
			printf(" End at %d ",cur_page);
			continue;
		}
#endif
			if (nand_in->check(nand_in->buf, check_buf, nand_in->length)) {
				/* (nand_out->status)[i] = 1; */
				printf(" pass!");
				cur_page = (ret[3]<<24)|(ret[2]<<16)|(ret[1]<<8)|(ret[0]<<0);
				printf(" End at %d ",cur_page);
			} else {
				/* (nand_out->status)[i] = 0; */
				printf(" fail!");
				struct nand_in_t bad;
				cur_page = (ret[3] << 24) | 
					(ret[2] << 16) |
					(ret[1] << 8) |
					(ret[0] << 0);
				printf(" End at %d ",cur_page);
				bad.start = (cur_page - 1) / hand.nand_ppb;
				if (cur_page % hand.nand_ppb==0)
					nand_markbad(&bad);
			}

	}
	/* handle_Close(); */
	*start_page = cur_page;
	return 0;
}

int nand_erase(struct nand_in_t *nand_in)
{
	unsigned int start_blk, blk_num, end_block;
	int i;

	start_blk = nand_in->start;
	blk_num = nand_in->length;
	if (start_blk > (unsigned int)NAND_MAX_BLK_NUM)  {
		printf("\n Start block number overflow!");
		return -1;
	}
	if (blk_num > (unsigned int)NAND_MAX_BLK_NUM) {
		printf("\n Length block number overflow!");
		return -1;
	}

	if (usb_get_ingenic_cpu(&ingenic_dev) < 3) {
		printf("\n Device unboot! Boot it first!");
		return -1;
	}

	for (i = 0; i < nand_in->max_chip; i++) {
		if ((nand_in->cs_map)[i]==0) 
			continue;
		printf("\n Erasing No.%d device No.%d flash......",
		       nand_in->dev, i);

		usb_send_data_address_to_ingenic(&ingenic_dev, start_blk);
		ingenic_dev.file_len = blk_num;
		usb_send_data_to_ingenic(&ingenic_dev);
		unsigned short temp = ((i << 4) & 0xff0) + NAND_ERASE;
		usb_ingenic_nand_ops(&ingenic_dev, temp);
		ingenic_dev.file_buff = ret;
		ingenic_dev.file_len = 8;
		usb_read_data_from_ingenic(&ingenic_dev);
		printf(" Finish!");
	}
	/* handle_Close(); need to change */
	end_block = ((ret[3] << 24) |
		     (ret[2] << 16) |
		     (ret[1] << 8)  | 
		     (ret[0] << 0)) / hand.nand_ppb;
	printf("\n Operation end position : %d ",end_block);
	if (!hand.nand_force_erase) {	/* not force erase, show bad block infomation */
		printf("\n There are marked bad blocks :%d ",end_block - start_blk - blk_num );
	} else {			/* force erase, no bad block infomation can show */
		printf("\n Force erase ,no bad block infomation !" );
	}
	return 1;
}

int nand_program_file(struct nand_in_t *nand_in,
		      struct nand_out_t *nand_out,
		      char *fname)
{

	int flen, m, j, k;
	unsigned int start_page = 0, page_num, code_len, offset, transfer_size;
	FILE *fp;
	unsigned char status_buf[32];
	struct nand_in_t n_in;
	struct nand_out_t n_out;

#if 0
	/* nand_out->status = (unsigned char *)malloc(nand_in->max_chip * sizeof(unsigned char)); */
	nand_out->status = status_buf;
	for (i=0; i<nand_in->max_chip; i++)
		(nand_out->status)[i] = 0; /* set all status to fail */
#endif
	fp = fopen(fname,"rb");
	if (fp == NULL) {
		printf("\n Can not open file !");
		return 0;
	}

	printf("\n Programing No.%d device...",nand_in->dev);
	fseek(fp,0,SEEK_END);
	flen=ftell(fp);
	n_in.start = nand_in->start / hand.nand_ppb; 
	if (nand_in->option == NO_OOB) {
		if (flen % (hand.nand_ppb * hand.nand_ps) == 0) 
			n_in.length = flen / (hand.nand_ps * hand.nand_ppb);
		else
			n_in.length = flen / (hand.nand_ps * hand.nand_ppb) + 1;
	} else {
		if (flen % (hand.nand_ppb * (hand.nand_ps + hand.nand_os)) == 0) 
			n_in.length = flen / ((hand.nand_ps + hand.nand_os) * hand.nand_ppb);
		else
			n_in.length = flen / ((hand.nand_ps + hand.nand_os) * hand.nand_ppb) + 1;
	}
	/* printf(" length %d flen %d \n",n_in.length,flen); */
	n_in.cs_map = nand_in->cs_map;
	n_in.dev = nand_in->dev;
	n_in.max_chip = nand_in->max_chip;
	if (nand_erase(&n_in) != 1)
		return -1;
	if (nand_in->option == NO_OOB)
		transfer_size = (hand.nand_ppb * hand.nand_ps);
	else
		transfer_size = (hand.nand_ppb * (hand.nand_ps + hand.nand_os));
	m = flen / transfer_size;
	j = flen % transfer_size;
	fseek(fp,0,SEEK_SET);	/* file point return to begin */
	offset = 0; 
	printf("\n Total size to send in byte is :%d", flen);
	printf("\n Image type : %s", IMAGE_TYPE[nand_in->option]);
	printf("\n It will cause %d times buffer transfer.", m + 1);
#if 0
	for (i = 0; i < nand_in->max_chip; i++)
		(nand_out->status)[i] = 1; /* set all status to success! */
#endif
	for (k = 0; k < m; k++)	{
		if (nand_in->option == NO_OOB)
			page_num = transfer_size / hand.nand_ps;
		else
			page_num = transfer_size / (hand.nand_ps + hand.nand_os);

		code_len = transfer_size;
		fread(code_buf, 1, code_len, fp); /* read code from file to buffer */
		printf("\n No.%d Programming...",k+1);
		nand_in->length = code_len; /* code length,not page number! */
		nand_in->buf = code_buf;
		if ( nand_program_check(nand_in, &n_out, &start_page) == -1)
			return -1;

		if ( start_page - nand_in->start > hand.nand_ppb ) 
			printf("\n Skip a old bad block !");
		nand_in->start = start_page;
#if 0
		for (i = 0; i < nand_in->max_chip; i++) {
			(nand_out->status)[i] = (nand_out->status)[i] * (n_out.status)[i];
		}
#endif
		/* offset += code_len - 1; */
		offset += code_len ;
		/* start_page += page_num; */
		/* nand_in->start += page_num; */
		fseek(fp,offset,SEEK_SET);
	}

	if (j) {
		j += hand.nand_ps - (j % hand.nand_ps);
		memset(code_buf, 0, j); /* set all to null */
		fread(code_buf, 1, j, fp); /* read code from file to buffer */
		nand_in->length = j;
		nand_in->buf = code_buf;
		printf("\n No.%d Programming...",k+1);

		if (nand_program_check(nand_in, &n_out, &start_page) == -1) 
			return -1;

		if (start_page - nand_in->start > hand.nand_ppb)
			printf(" Skip a old bad block !");
#if 0
		for (i=0; i < nand_in->max_chip; i++) {
			(nand_out->status)[i] = (nand_out->status)[i] * (n_out.status)[i];
		}
#endif
	}
	
	fclose(fp);
	return 1;
}

int nprog(void)
{
	unsigned int i;
	char *image_file;
	char *help = "\n Usage: nprog (1) (2) (3) (4) (5)"
		"\n (1)\tstart page number"
		"\n (2)\timage file name"
		"\n (3)\tdevice index number"
		"\n (4)\tflash index number"
		"\n (5) image type must be:"
		"\n \t-n:\tno oob"
		"\n \t-o:\twith oob no ecc"
		"\n \t-e:\twith oob and ecc";
	nand_in.buf = code_buf;
	nand_in.check = NULL;
	nand_in.dev = 0;
	nand_in.cs_map = cs;
	nand_in.max_chip =16;

	if (com_argc != 6) {
		printf("\n not enough argument.");
		printf("%s", help);
		return 0;
	}

	for (i = 0; i < MAX_DEV_NUM; i++)
		(nand_in.cs_map)[i] = 0;

	nand_in.start = atoi(com_argv[1]);
	image_file = com_argv[2];
	nand_in.dev = atoi(com_argv[3]);
	(nand_in.cs_map)[atoi(com_argv[4])] = 1;
	if (!strcmp(com_argv[5],"-e")) 
		nand_in.option = OOB_ECC;
	else if (!strcmp(com_argv[5],"-o")) 
		nand_in.option = OOB_NO_ECC;
	else if (!strcmp(com_argv[5],"-n")) 
		nand_in.option = NO_OOB;
	else
		printf("%s", help);

	if (hand.nand_plane > 1)
		;/* nand_program_file_planes(&nand_in,&nand_out, image_file); */
	else
		nand_program_file(&nand_in,&nand_out, image_file);

#if 0
	printf("\n Flash check result:");
	for (i = 0; i < 16; i++)
		printf(" %d", (nand_out.status)[i]);
#endif
	printf("\n nprog %d %s %d %d %d", nand_in.start,
	       image_file,
	       nand_in.dev,
	       (nand_in.cs_map)[atoi(com_argv[4])],
	       nand_in.option);
	printf("\n not implement yet!! just test");

	return 1;
}
