/*
 * Command line handling.
 *
 * This software is free.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "include.h"
#include "configs.h"
#include "nand_ecc.h"

#define printf_log(fp,fmt,args...)  fprintf(fp,fmt,## args)

static u8 nand_buf[(2048+64)*128];       //Max 128 pages!
static u8 check_buf[(2048+64)*128];
static u32 spage, epage, chip_num;
static u8 *filename,ops_t,idx,cs_index,args_num;
static np_data *npdata;
static FILE *log_fp;

#define USE_VALID_CHECK 

int nand_check_cmp(u8 *buf1,u8 *buf2,u32 len)
{
	u32 i;

	for (i = 0; i < len; i++)
	{
		if (buf1[i] != buf2[i])
		{
			printf("Check error! %x\n",i);
			return -1;
		}
	}
	return 0;
}

void dump_npdata(np_data *np)
{
	int i;

	printf("Process type:%d \n",np->pt);

	printf("NAND interface ps:%d bw:%d rc:%d ppb:%d os:%d bbp:%d bba:%d\n",
	       np->ps,np->bw,np->rc,np->ppb,np->os,np->bbp,np->bba);

	printf("ECC configration type:%d index:%d\n",np->et,np->ep);
	printf("ECC position:");
	for (i = 0;i < oob_64[np->ep].eccbytes;i++)
	{
		if (i % 9 == 0) printf("\n");
		printf("%d ",oob_64[np->ep].eccpos[i]);
	}

}

//a check sample for WINCE
//Modify this function according to your system
int check_invalid_block(u8 *buf,np_data *np)      
{
	int i,j;
	u8 *p = buf + np->ps ,* q ;

	q = buf + (( np->ps + np->os ) *  np->ppb - 1) - 10;

	if ( (*q) != 0xff )
	{
//		printf("A mark erase block! \n");
		return 0;
	}

	for ( i = 0; i < np->ppb; i ++ )
	{
		for (j = 0; j < np->os; j ++ )
		{
			if ( p[j] != 0xff )
			{
				return 1;
			}
		}
		p += np->ps;
	}
//	printf("A never use block! \n");
	return 0;
}

int do_read_flash(np_data *np)
{
	FILE *fp;
	u32 sp;
	int i,j,k;

	if ((fp = fopen((const char *)np->fname, "w+")) == NULL ) 
	{
		printf("Can not open source or object file!\n");
		return -1;
	}
	i = np->epage - np->spage;
	j = i / MAX_BUF_PAGE;
	k = i % MAX_BUF_PAGE;
	sp = np->spage;

	for (i=0;i<j;i++)
	{
		if (np->nand_check_block(sp/np->ppb))
		{		
			printf_log(log_fp,"Skip a old block at %x!\n",sp/np->ppb);
			sp += MAX_BUF_PAGE;
			printf("Skip a old block!\n");
			continue;
		}
		np->nand_read(nand_buf, sp, MAX_BUF_PAGE);
#ifdef USE_VALID_CHECK 
		if ( check_invalid_block(nand_buf,np) ) 
		{
#endif
			fwrite(nand_buf,1,MAX_BUF_SIZE,fp);
			printf("Read block %d finish\n",sp/np->ppb);
#ifdef USE_VALID_CHECK 
		}
		else printf("Skip a invalid block! %d \n",sp/np->ppb);
#endif
		sp += MAX_BUF_PAGE;
	}
	if (k)
	{
		if (np->nand_check_block(sp/np->ppb))
		{
			printf_log(log_fp,"Skip a old block at %x!\n",sp/np->ppb);
			printf("Skip a old block!\n");
		}
		else
		{
			np->nand_read(nand_buf, sp, k);
#ifdef USE_VALID_CHECK 
			if ( check_invalid_block(nand_buf,np) ) 
			{
#endif
				fwrite(nand_buf, 1, k*OOBPAGE_SIZE, fp);
#ifdef USE_VALID_CHECK 
			}
		else printf("Skip a invalid block! %d \n",sp/np->ppb);
#endif
		}
	}
	printf("Read nand flash finish!\n");
	fclose(fp);
	return 0;
}

int do_write_flash(np_data *np)
{
	FILE *fp;
	u32 sp,flen,offset;
	int i,j,k,r,error_flag=0;
	if ((fp = fopen((const char *)np->fname,"r")) == NULL ) 
	{
		printf("Can not open source or object file!\n");
		return -1;
	}
	fseek(fp,0,SEEK_END);
	flen = ftell(fp);
	i = flen / OOBPAGE_SIZE;
	if (flen % OOBPAGE_SIZE !=0)
	{
		printf("Source file length is not fit!\n");
		return -1;
	}
	sp = np->spage;
	if (sp % np->ppb !=0) 
	{
		printf("Start page number not blockaligned!\n");
		return -1;
	}
	//Erase object block first
	j = sp / np->ppb;
	k = flen/OOBPAGE_SIZE;
	if (k % np->ppb == 0) k = k / np->ppb;
	else k = k / np->ppb +1;
	np->nand_erase(k,j,0);
	j = i / MAX_BUF_PAGE;
	k = i % MAX_BUF_PAGE;
	offset = 0;
//	printf("j k %d %d %d %d\n",j,k,i,flen);

	for (i=0;i<j;i++)
	{
		fseek(fp,offset,SEEK_SET);
		fread(nand_buf,1,MAX_BUF_SIZE,fp);
BLOCK_BROKEN:
		for (r=0;r<MAX_RETRY;r++)
		{
			for (;sp <=np->epage - np->ppb;sp += np->ppb)
			{     //old bad block
				if (!np->nand_check_block(sp/np->ppb))
					break;
				printf("Skip a old bad blocks!\n");
				printf_log(log_fp,"Skip a old block at %x!\n",sp/np->ppb);
			}
			if (sp/np->ppb > np->epage /np->ppb) 
			{
				printf("Program end but not finish,due to bad block!\n");
				printf_log(log_fp,"Program end but not finish,due to bad block!\n");
				return -1;		
			}
			if (np->nand_program(nand_buf,sp,np->ppb))
			{    
				error_flag = 1;
				printf("Program error!\n");
				printf_log(log_fp,"Program error! %x\n",sp/np->ppb);
				break;
			}
			memset(check_buf,0,MAX_BUF_SIZE);
			np->nand_read(check_buf,sp,np->ppb);
			if (np->nand_check(nand_buf,check_buf,
					   MAX_BUF_SIZE) )
			{    //check error!
				error_flag = 1;
				printf("Error retry!\n");
				printf_log(log_fp,"Error retry!\n");
				continue;
			}
			else
			{
				error_flag = 0;
				break;
			}
		}
		if (error_flag)
		{           //block has broken!
			printf("Found a new bad block: %x!\n",sp/np->ppb);
			printf_log(log_fp,"Found a new bad block at %x!\n",sp/np->ppb);
			np->nand_erase(1,sp/np->ppb,0);                //erase before mark bad block!
			np->nand_block_markbad(sp /np->ppb);
			sp += np->ppb;
			goto BLOCK_BROKEN;
		}
		else
		{
			printf("Write block %d finish\n",sp/np->ppb);
			sp += np->ppb;
			offset += MAX_BUF_SIZE; 
		}
	}
	if (k)
	{
		fseek(fp,offset,SEEK_SET);
		fread(nand_buf,1,k * OOBPAGE_SIZE ,fp);
BLOCK_BROKEN1:
		for (r=0;r<MAX_RETRY;r++)
		{
			for (;sp <=np->epage - np->ppb;sp += np->ppb)
			{     //old bad block
				if (!np->nand_check_block(sp/np->ppb))
					break;
				printf("Skip a old bad blocks!\n");
				printf_log(log_fp,"Skip a old block at %x!\n",sp/np->ppb);
			}
			if (sp/np->ppb > np->epage/np->ppb) 
			{
				printf("Program end but not finish,due to bad block!\n");
				printf_log(log_fp,"Program end but not finish,due to bad block!\n");
				return 0;		
			}

			if (np->nand_program(nand_buf,sp,k))
			{    
				error_flag = 1;
				printf("Program error!\n");
				printf_log(log_fp,"Program error! %x\n",sp/np->ppb);
				break;
			}
			memset(check_buf,0,MAX_BUF_SIZE);
			np->nand_read(check_buf,sp,k);
			if (np->nand_check(nand_buf,check_buf,
					   k * OOBPAGE_SIZE) )
			{    //check error!
				error_flag = 1;
				printf("Error retry!\n");
				printf_log(log_fp,"Error retry!\n");
				continue;
			}
			else
			{
				error_flag = 0;
				break;
			}
		}
		if (error_flag)
		{           //block has broken!
			printf("Found a new bad block : %x!\n",sp/np->ppb);
			printf_log(log_fp,"Found a new bad block at %x!\n",sp/np->ppb);
			np->nand_erase(1,sp/np->ppb,0);                //erase before mark bad block!
			np->nand_block_markbad(sp /np->ppb);
			sp += np->ppb;
			goto BLOCK_BROKEN1;
		}

	}
	printf("Nand flash write finish!\n");
	return 0;
}

void show_usage()
{
	printf("Nand flash programmer.Version v1.0\n");
	printf("Usage: nandprog spage epage opration_type obj_file chip_index [config_index]\n\n");
	printf("  spage           operation start page number\n");
	printf("  epage           operation end page number\n");
	printf("  opration_type   operation type read or write\n");
	printf("  obj_file        source or object filename\n");
	printf("  chip_index      chip select index\n");
	printf("  config_index    optional,when chosen,\n");
	printf("                  will use one of these default configrations instead of load from CFG\n");

}

int cmdline(int argc, char *argv[], np_data *np)
{

	if (argc<6 || argc>7)
	{
		show_usage();
		return -1;
	}
	
	if (strlen(argv[1])>8)
	{
		printf("Start address page error!\n");
		return -1;
	}
	spage = atoi(argv[1]);
	if (spage > MAX_PAGE)
	{
		printf("Start address page error!\n");
		return -1;
	}

	if (strlen(argv[2])>8)
	{
		printf("End address page error!\n");
		return -1;
	}
	epage = atoi(argv[2]);
	if (epage > MAX_PAGE)
	{
		printf("End address page error!\n");
		return -1;
	}

	if (strlen(argv[3])>1)
	{
		printf("Operation type error!\n");
		return -1;
	}
	if (argv[3][0] == 'r') 
		ops_t = READ_FLASH;
	else if (argv[3][0] == 'w')
		ops_t = WRITE_FLASH;
	else
	{
		printf("Operation type error!\n");
		return -1;
	}

	if (strlen(argv[4])>20)
	{
		printf("Source or object file name error!\n");
		return -1;
	}
	filename = (unsigned char *)argv[4];

	if (strlen(argv[5])>2)
	{
		printf("Chip select number error!\n");
		return -1;
	}
	cs_index = atoi(argv[5]);

	if (epage <= spage)
	{
		printf("End page number must larger than start page number!\n");
		return -1;
	}

	if (argc == 7)
	{
		args_num = 7;
		if (strlen(argv[6])>3)
		{
			printf("Processor type error!\n");
			return -1;
		}
		idx = atoi(argv[6]);
		if (idx > 20)
		{
			printf("Processor type error!\n");
			return -1;
		}
	}
	else args_num = 6;

	printf("Deal command line: spage%d epage%d ops%d file:%s cs%d\n",
	       spage,epage,ops_t,filename,cs_index);

	return 0;
}

void init_funs(np_data *np)
{
	switch (np->pt)
	{
	case JZ4740:
		np->ebase = 0x13010000; 
		np->dport = 0x18000000; 
		np->gport = 0x10010000; 
		np->bm_ms = 0x100;
		np->pm_ms = 0x20000;
		np->gm_ms = 0x500;
		np->ap_offset = 0x10000;
		np->cp_offset = 0x8000;

		np->nand_init = nand_init_4740;
		np->nand_erase = nand_erase_4740;
		np->nand_program = nand_program_4740;
		np->nand_read = nand_read_4740_rs;
		np->nand_read_raw = nand_read_raw_4740;
		np->nand_read_oob = nand_read_oob_4740;
		np->nand_block_markbad = nand_block_markbad_4740;
		np->nand_check = nand_check_cmp;
		np->nand_check_block = nand_check_block_4740;
		if (np->et == HARDRS)
			np->nand_read = nand_read_4740_rs;
		else 
			np->nand_read = nand_read_4740_hm;

		break;
	case JZ4730:
		np->ebase = 0x13010000; 
		np->dport = 0x14000000; 
		np->gport = 0x0; 
		np->bm_ms = 0x100;
		np->pm_ms = 0xb0000;
		np->gm_ms = 0x0;
		np->ap_offset = 0x80000;
		np->cp_offset = 0x40000;

		np->nand_init = nand_init_4730;
		np->nand_erase = nand_erase_4730;
		np->nand_program = nand_program_4730;
		np->nand_read = nand_read_4730;
		np->nand_read_oob = nand_read_oob_4730;
		np->nand_block_markbad = nand_block_markbad;
		np->nand_check = nand_check_cmp;
		np->nand_check_block = nand_check_block;
		np->nand_select = chip_select_4730;
		break;
	case JZ4760:
		break;
	}

//	dump_npdata(np);
}

np_data * cmdinit()
{
	int fd;
	if (args_num>6)
	{
		npdata = &config_list[idx];
		if (npdata) 
			printf("Load configration index success!\n");
		else
		{
			printf("Load configration index fail!\n");
			return 0;
		}
	}
	else
	{
		npdata = load_cfg();
		if (npdata) 
			printf("Load configration file success!\n");
		else
		{
			printf("Load configration file fail!\n");
			return 0;
		}
	}
	if (!npdata) return 0;

	init_funs(npdata);
	npdata->spage = spage;
	npdata->epage = epage;
	npdata->fname = filename;
	npdata->ops = ops_t;
	npdata->cs = cs_index;

	if((fd=open("/dev/mem",O_RDWR|O_SYNC))==-1)
	{
		printf("Can not open memory file!\n");
		return 0;
	}

	npdata->base_map = mmap(NULL,npdata->bm_ms,PROT_READ | PROT_WRITE,MAP_SHARED,fd,npdata->ebase);
	if(npdata->base_map == MAP_FAILED) 
	{
		printf("Can not map EMC_BASE ioport!\n");
		return 0;
	}
	else printf("Map EMC_BASE success :%x\n",(u32)npdata->base_map);

	npdata->port_map=mmap(NULL,npdata->pm_ms ,PROT_READ | PROT_WRITE,MAP_SHARED,fd,npdata->dport);
	if(npdata->port_map== MAP_FAILED) 
	{
		printf("Can not map NAND_PORT ioport!\n");
		return 0;
	}	
	else printf("Map NAND_PORT success :%x\n",(u32)npdata->port_map);

	if (npdata->pt == JZ4740)
	{
		npdata->gpio_map=mmap(NULL,npdata->gm_ms ,PROT_READ | PROT_WRITE,MAP_SHARED,fd,npdata->gport);
		if(npdata->gpio_map== MAP_FAILED) 
		{
			printf("Can not map GPIO ioport!\n");
			return 0;
		}	
		else printf("Map GPIO_PORT success :%x\n",(u32)npdata->gpio_map);
	}

	close(fd);
 
	printf("Memory map all success!\n");
	npdata->nand_init(npdata);

	return npdata;
}

int cmdexcute(np_data *np)
{
	int ret;

	if ((log_fp=fopen(NUM_FILENAME,"a+"))==NULL ) 
	{
		printf("Can not open number file!\n");
		return -1;
	}
	fscanf(log_fp,"%d",&chip_num);
	fclose(log_fp);
	chip_num++;
	if ((log_fp=fopen(NUM_FILENAME,"w"))==NULL ) 
	{
		printf("Can not open number file!\n");
		return -1;
	}
	printf_log(log_fp,"%d",chip_num);
	fclose(log_fp);

	if ((log_fp=fopen(LOG_FILENAME,"a+"))==NULL ) 
	{
		printf("Can not open log file!\n");
		return -1;
	}
	printf_log(log_fp,"\nNo.%d :\n",chip_num);

	if (np->ops == READ_FLASH)
	{
		printf_log(log_fp,"Read nand flash!\n");	
		printf_log(log_fp,"Args:index=%d spage=%d epage=%d file=%s cs=%d\n",
		       idx,spage,epage,filename,cs_index);
		ret= do_read_flash(np);
	}
	else
	{
		printf_log(log_fp,"Write nand flash!\n");	
		printf_log(log_fp,"Args:index=%d spage=%d epage=%d file=%s cs=%d\n",
		       idx,spage,epage,filename,cs_index);
		ret= do_write_flash(np);
	}

	if (!ret) 	
		printf_log(log_fp,"Operation success!\n");
	else
		printf_log(log_fp,"Operation fail!\n");

	fclose(log_fp);
	return 0;
}

int cmdexit(np_data *np)
{
	munmap(np->base_map,np->bm_ms);
	munmap(np->port_map,np->pm_ms);
	if (np->pt == JZ4740)
		munmap(np->gpio_map,np->gm_ms);
	return 0;
}
