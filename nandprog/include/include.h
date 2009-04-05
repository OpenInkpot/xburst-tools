#ifndef __INCLUDE_H__
#define __INCLUDE_H__
//#include "nand_ecc.h"

#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
#define MAX_PAGE 0xffffffff
#define PAGE_SIZE np->ps
#define OOB_SIZE np->os
#define OOBPAGE_SIZE (PAGE_SIZE + OOB_SIZE)
#define MAX_BUF_PAGE np->ppb
#define MAX_BUF_SIZE OOBPAGE_SIZE*MAX_BUF_PAGE    
#define MAX_RETRY 3
#define LAST_PAGE 65536
#define LOG_FILENAME "nprog.log"
#define NUM_FILENAME "number.log"

enum
{
	JZ4730CPU,
	LINUXHM,
	JZ4740CPU,
	LINUXRS,
	USERSPEC,
};

struct nand_oobinfo 
{
	int eccname;
	unsigned int eccbytes;
	unsigned int eccpos[64];
};

enum
{
	SOFTHM,
	SOFTRS,
	HARDHM,
	HARDRS
};

enum
{
	JZ4730,
	JZ4740,
	JZ4760
};

enum
{
	READ_FLASH,
	WRITE_FLASH
};

typedef struct _NP_DATA
{
	u8 pt;    //processor type jz4730/jz4740/jz4760....
	u8 et;    //ECC type software HM/RS or hardware HM/RS
	u8 ep;    //ECC position index
	u8 ops;   //opration type read/write
	u8 cs;    //chip select index number
 	u8 *fname;   //Source or object file name
	u32 spage;   //opration start page number of nand flash
	u32 epage;   //opration end page number of nand flash

	u32 bw;    //nand flash bus width
	u32 ps;    //nand flash page size
	u32 os;    //nand flash oob size
	u32 ppb;   //nand flash page per block
	u32 rc;    //nand flash row syscle
	u32 bbp;   //nand flash bad block ID position
	u32 bba;   //nand flash bad block ID page position

	u32 ebase;        //EMC base PHY address
	void *base_map;   //EMC base mapped address
	u32 bm_ms;        // EMC base mapped size

	u32 dport;        //Nand flash port base PHY address
	void *port_map;   //nand port mapped address
	u32 pm_ms;        // EMC base mapped size

	u32 gport;        //GPIO base PHY address
	void *gpio_map;   //GPIO mapped address
	u32 gm_ms;        // EMC base mapped size

	u32 ap_offset;    //addrport offset
	u32 cp_offset;    //cmdportoffset

	int (*nand_init)(struct _NP_DATA *);
	int (*nand_fini)(void);
	u32 (*nand_query)(void);
	int (*nand_erase)(int, int, int);
	int (*nand_program)(u8 *, int, int );
	int (*nand_read)(u8 *, u32, u32);
	int (*nand_read_raw)(u8 *, u32, u32);
	int (*nand_read_oob)(u8 *, u32, u32);
	int (*nand_check_block) (u32);
	int (*nand_check) (u8 *,u8 *,u32 );
	void (*nand_block_markbad) (u32);
	int (*nand_select) (u8);

}np_data;

//jz4730 functions 
extern int nand_init_4730(np_data *);
extern int nand_fini_4730(void);
extern unsigned int nand_query_4730(void);
extern int nand_erase_4730(int blk_num, int sblk, int force);
extern int nand_program_4730(u8 *buf, int startpage, int pagenum);
extern int nand_read_4730(u8 *buf, u32 startpage, u32 pagenum);
extern int nand_read_raw_4730(u8 *buf, u32 startpage, u32 pagenum);
extern int nand_read_oob_4730(u8 *buf, u32 startpage, u32 pagenum);
extern int nand_check_block(u32);
extern void nand_block_markbad(u32);
extern int chip_select_4730(u8 cs);

//jz4740 functions 
extern int nand_init_4740(np_data *);
extern int nand_fini_4740(void);
extern unsigned int nand_query_4740(void);
extern int nand_erase_4740(int blk_num, int sblk, int force);
extern int nand_program_4740(u8 *buf, int startpage, int pagenum);
extern int nand_read_4740_hm(u8 *buf, u32 startpage, u32 pagenum);
extern int nand_read_4740_rs(u8 *buf, u32 startpage, u32 pagenum);
extern int nand_read_raw_4740(u8 *buf, u32 startpage, u32 pagenum);
extern int nand_read_oob_4740(u8 *buf, u32 startpage, u32 pagenum);
extern int nand_check_block_4740(u32);
extern void nand_block_markbad_4740(u32);
extern int chip_select_4740(u8 cs);

//common functions
extern int cmdline(int,char **,np_data *);
extern np_data * cmdinit();
extern int cmdexcute(np_data *);
extern int cmdexit(np_data *);
extern int nand_check_cmp(u8 *buf1,u8 *buf2,u32 len);
extern np_data * load_cfg();

#endif
