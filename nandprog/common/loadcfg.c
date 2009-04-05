/*
 * Configfile parsing.
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

#define CFG_FIELD_NUM 10

static np_data np;

extern struct nand_oobinfo oob_64[];

const char CFG_FIELD[][30]=
{
	"CPUTYPE",
	"BUSWIDTH",			
	"ROWCYCLES",		
	"PAGESIZE",		
	"PAGEPERBLOCK",		
	"OOBSIZE",
	"BADBLOCKPOS",
	"BADBLOCKPAGE",
	"ECCTYPE",
	"[END]",
};

np_data * load_cfg(void)
{
	FILE *fp;
	char line[100];
	unsigned short i,j;
	unsigned int d;

	if ((fp = fopen("nandprog.cfg", "r"))==NULL)
	{
		printf("Can not open configration file!\n");
		return 0;
	}
	
	while(!strstr(line, "[NANDPROG]"))     //find the nandprog section!
	{
		if (feof(fp))
		{
			printf("nand programmer configration file illege!\n");
			return 0;
		}
		fscanf(fp,"%s",line);
	}

	while(1)
	{
		if (feof(fp))
		{
			printf("nand programmer configration file illege!\n");
			return 0;
		}
		fscanf(fp,"%s",line);
		if (line[0]==';') 
		{
			line[0]='\0';
			continue;
		}

		for (i=0;i<CFG_FIELD_NUM;i++)
			if (strstr(line,CFG_FIELD[i])) break;
		
		switch (i)
		{
		case 0:		//CPUTYPE
			while (!feof(fp))
			{
				fscanf(fp,"%s",line);
				if (strstr(line,"JZ4730"))
				{
					np.pt = JZ4730;
					break;
				}
				else  if  (strstr(line,"JZ4740"))
				{
					np.pt = JZ4740;
					break;
				}
				else	continue;
			}
			break;
		case 1:		//BUSWIDTH
			while (!feof(fp))
			{
				fscanf(fp,"%d",&d);
				if (d!=8 && d!=16) continue;
				np.bw = d;
				break;
			}
			break;
		case 2:		//ROWCYCLES
			while (!feof(fp))
			{
				fscanf(fp,"%d",&d);
				if (d!=3 && d!=2) continue;
				np.rc = d;
				break;
			}
			break;
		case 3:		//PAGESIZE
			while (!feof(fp))
			{
				fscanf(fp,"%d",&d);
				if (d!=2048 && d!=512) continue;
				np.ps = d;
				break;
			}
			break;
		case 4:		//PAGEPERBLOCK
			while (!feof(fp))
			{
				fscanf(fp,"%d",&d);
				if (d!=128 && d!=64) continue;
				np.ppb = d;
				break;
			}
			break;
		case 5:		//OOBSIZE
			while (!feof(fp))
			{
				fscanf(fp,"%d",&d);
				if (d!=16 && d!=64) continue;
				np.os = d;
				break;
			}
			break;
		case 6:		//BADBLOCKPOS
			while (!feof(fp))
			{
				fscanf(fp,"%d",&d);
				if (d>2048) continue;
				np.bbp = d;
				break;
			}
			break;
		case 7:		//BADBLOCKPAGE
			while (!feof(fp))
			{
				fscanf(fp,"%d",&d);
				if (d>np.ppb) continue;
				np.bba = d;
				break;
			}

			break;
		case 8:		//ECCTYPE
			while (!feof(fp))
			{
				fscanf(fp,"%s",line);
				if (strstr(line,"RS"))
				{
					np.et = HARDRS;
					d = 36;     //36 bytes ecc
					oob_64[4].eccbytes = 36;
					np.ep = 4;
					break;
				}
				else  if  (strstr(line,"HM"))
				{
					np.et = HARDHM;
					d = 24;     //24 bytes ecc
					oob_64[4].eccbytes = 24;
					np.ep = 4;
					break;
				}
				else	continue;
			}
			while (!feof(fp))
			{
				fscanf(fp,"%s",line);
				if (strstr(line,"{")) break;
			}
			for (j = 0;j < d;j++)
			{
				if (feof(fp)) 
				{
					printf("nand programmer configration file illege!\n");
					return 0;
				}
				fscanf(fp,"%d",&d);
				if (d > np.os) 
				{
					printf("nand programmer configration file illege!\n");
					return 0;
				}
				oob_64[4].eccpos[j] = d;
			}

			while (!feof(fp))
			{
				fscanf(fp,"%s",line);
				if (strstr(line,"}")) break;
			}
			break;
		case 9:			
			return &np;
		default:
			;
		}

	}
}
