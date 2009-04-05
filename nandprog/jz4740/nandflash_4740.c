/*
 * Common NAND Flash operations for JZ4740.
 *
 * This software is free.
 */

#include "jz4740.h"
#include "include.h"

extern struct nand_oobinfo oob_64[];

#define __nand_enable()		(REG_EMC_NFCSR |= EMC_NFCSR_NFE1 | EMC_NFCSR_NFCE1)
#define __nand_disable()	(REG_EMC_NFCSR &= ~(EMC_NFCSR_NFCE1|EMC_NFCSR_NFE1 ))
#define __nand_ecc_rs_encoding()	(REG_EMC_NFECR = EMC_NFECR_ECCE | EMC_NFECR_ERST | EMC_NFECR_RS | EMC_NFECR_RS_ENCODING)
#define __nand_ecc_rs_decoding()	(REG_EMC_NFECR = EMC_NFECR_ECCE | EMC_NFECR_ERST | EMC_NFECR_RS | EMC_NFECR_RS_DECODING)
#define __nand_ecc_disable()	(REG_EMC_NFECR &= ~EMC_NFECR_ECCE)
#define __nand_ecc_encode_sync() while (!(REG_EMC_NFINTS & EMC_NFINTS_ENCF))
#define __nand_ecc_decode_sync() while (!(REG_EMC_NFINTS & EMC_NFINTS_DECF))
#define __nand_ecc_enable()    (REG_EMC_NFECR = EMC_NFECR_ECCE | EMC_NFECR_ERST )
#define __nand_ecc_disable()   (REG_EMC_NFECR &= ~EMC_NFECR_ECCE)

#define __nand_select_hm_ecc() (REG_EMC_NFECR &= ~EMC_NFECR_RS )
#define __nand_select_rs_ecc() (REG_EMC_NFECR |= EMC_NFECR_RS)

#define __nand_read_hm_ecc()   (REG_EMC_NFECC & 0x00ffffff)

#define __nand_ecc()		(REG_EMC_NFECC & 0x00ffffff)
#define __nand_cmd(n)		(REG8(cmdport+csn) = (n))
#define __nand_addr(n)		(REG8(addrport+csn) = (n))
#define __nand_data8()		REG8(dataport+csn)
#define __nand_data16()		REG16(dataport+csn)

#define CMD_READA	0x00
#define CMD_READB	0x01
#define CMD_READC	0x50
#define CMD_ERASE_SETUP	0x60
#define CMD_ERASE	0xD0
#define CMD_READ_STATUS 0x70
#define CMD_CONFIRM	0x30
#define CMD_SEQIN	0x80
#define CMD_PGPROG	0x10
#define CMD_READID	0x90

#define OOB_BAD_OFF	0x00
#define OOB_ECC_OFF	0x04

#define OP_ERASE	0
#define OP_WRITE	1
#define OP_READ		2

#define ECC_BLOCK	512
#define ECC_POS	        6
#define PAR_SIZE        9

static volatile unsigned char *gpio_base;
static volatile unsigned char *emc_base;
static volatile unsigned char *addrport;
static volatile unsigned char *dataport;
static volatile unsigned char *cmdport;
unsigned int	EMC_BASE;
unsigned int	GPIO_BASE;

static int bus = 8, row = 2, pagesize = 512, oobsize = 16, ppb = 32;
static u32 bad_block_pos = 0,bad_block_page=0, csn = 0;
static u8 badbuf[2048 + 64] = {0};
static u8 data_buf[2048] = {0};
static u8 oob_buf[128] = {0};
static struct nand_oobinfo *oob_pos;
static np_data *np;

static inline void __nand_sync(void)
{
	unsigned int timeout = 1000;
	while ((REG_GPIO_PXPIN(2) & 0x40000000) && timeout--);
	while (!(REG_GPIO_PXPIN(2) & 0x40000000));
}

static int read_oob(u8 *buf, u32 size, u32 pg);
static int nand_data_write8(unsigned char *buf, int count);
static int nand_data_write16(unsigned char *buf, int count);
static int nand_data_read8(unsigned char *buf, int count);
static int nand_data_read16(unsigned char *buf, int count);

static int (*write_proc)(unsigned char *, int) = 0;
static int (*read_proc)(unsigned char *, int) = 0;

extern void dumpbuf(u8 *p, int count);

unsigned int nand_query_4740(void)
{
	u16 vid, did;

	__nand_sync(); 
	__nand_cmd(CMD_READID);
	__nand_addr(0);

	vid = __nand_data8();
	did = __nand_data8();

	return (vid << 16) | did;
}

int chip_select_4740(u8 cs)
{
	csn = (u32)cs << 15;    // modify this number for your board
	return 0;
}

int nand_init_4740(np_data *npp)
{
	bus = npp->bw;
	row = npp->rc;
	pagesize = npp->ps;
	oobsize = npp->os;
	ppb = npp->ppb;
	bad_block_pos = npp->bbp;
	bad_block_page = npp->bba;
	gpio_base = (u8 *)npp->gpio_map;
	emc_base = (u8 *)npp->base_map;
	dataport = (u8 *)npp->port_map;
	addrport = (u8 *)((u32)dataport + npp->ap_offset);
	cmdport = (u8 *)((u32)dataport + npp->cp_offset);

	EMC_BASE = (u32)emc_base;
	GPIO_BASE = (u32)gpio_base;

	/* Initialize NAND Flash Pins */
//	__gpio_as_nand();
//	__nand_enable();

	chip_select_4740(npp->cs);
	if (bus == 8) {
		write_proc = nand_data_write8;
		read_proc = nand_data_read8;
	} else {
		write_proc = nand_data_write16;
		read_proc = nand_data_read16;
	}

	oob_pos = &oob_64[npp->ep];
//	REG_EMC_SMCR1 = 0x0fff7700;
	np = npp;
	return 0;
}

int nand_fini_4740(void)
{
	__nand_disable();
	return 0;
}

/*
 * Read oob <pagenum> pages from <startpage> page.
 * Don't skip bad block.
 * Don't use HW ECC.
 */
int nand_read_oob_4740(u8 *buf, u32 startpage, u32 pagenum)
{
	u32 cnt, cur_page;
	u8 *tmpbuf;

	tmpbuf = (u8 *)buf;

	cur_page = startpage;
	cnt = 0;
	while (cnt < pagenum) {
		read_oob((void *)tmpbuf, oobsize, cur_page);

		tmpbuf += oobsize;
		cur_page++;
		cnt++;
	}

	return 0;
}

int nand_check_block_4740(u32 block)
{
	u32 pg;

	pg = block * ppb + bad_block_page;
	read_oob(oob_buf, oobsize, pg);
	if (oob_buf[bad_block_pos] != 0xff)
		return -1;
	read_oob(oob_buf, oobsize, pg + 1);
	if (oob_buf[bad_block_pos] != 0xff)
		return -1;

	return 0;
}

/*
 * Mark a block bad.
 */
void nand_block_markbad_4740(u32 block)
{
	u32 i, rowaddr;

	for (i = 0; i < pagesize + oobsize; i++)
		badbuf[i] = 0x00;
	badbuf[pagesize + bad_block_pos] = 0; /* bad block flag */

	rowaddr = block * ppb + bad_block_page;
	//bad block ID locate No.bad_block_page page

	__nand_cmd(CMD_READA);
	__nand_cmd(CMD_SEQIN);

	__nand_addr(0);
	if (pagesize == 2048)
		__nand_addr(0);
	for (i = 0; i < row; i++) {
		__nand_addr(rowaddr & 0xff);
		rowaddr >>= 8;
	}

	write_proc((unsigned char *)badbuf, pagesize + oobsize);
	__nand_cmd(CMD_PGPROG);
	__nand_sync();
}

/*
 * Read data <pagenum> pages from <startpage> page.
 * Don't skip bad block.
 * Don't use HW ECC.
 */
int nand_read_raw_4740(u8 *buf, u32 startpage, u32 pagenum)
{
	u32 cnt, j;
	u32 cur_page, rowaddr;
	u8 *tmpbuf;

	tmpbuf = (u8 *)buf;

	cur_page = startpage;
	cnt = 0;
	while (cnt < pagenum) {
		__nand_sync();
		__nand_cmd(CMD_READA);
		__nand_addr(0);
		if (pagesize == 2048)
			__nand_addr(0);

		rowaddr = cur_page;
		for (j = 0; j < row; j++) {
			__nand_addr(rowaddr & 0xff);
			rowaddr >>= 8;
		}

		if (pagesize == 2048)
			__nand_cmd(CMD_CONFIRM);

		__nand_sync();
		read_proc(tmpbuf, pagesize);

		tmpbuf += pagesize;
		cur_page++;
		cnt++;
	}

	return 0;
}


int nand_erase_4740(int blk_num, int sblk, int force)
{
	int i, j;
	u32 cur, rowaddr;

	cur = sblk * ppb;
	for (i = 0; i < blk_num; i++) {
		rowaddr = cur;

		if (!force) {	/* if set, erase anything */
			/* test Badflag. */
			__nand_sync();

			__nand_cmd(CMD_READA);

			__nand_addr(0);
			if (pagesize == 2048)
				__nand_addr(0);
			for (j=0;j<row;j++) {
				__nand_addr(rowaddr & 0xff);
				rowaddr >>= 8;
			}

			if (pagesize == 2048)
				__nand_cmd(CMD_CONFIRM);

			__nand_sync();

			read_proc((u8 *)data_buf, pagesize);
			read_proc((u8 *)oob_buf, oobsize);

			if (oob_buf[0] != 0xff) { /* Bad block, skip */
				cur += ppb;
				continue;
			}
			rowaddr = cur;
		}

		__nand_cmd(CMD_ERASE_SETUP);

		for (j = 0; j < row; j++) {
			__nand_addr(rowaddr & 0xff);
			rowaddr >>= 8;
		}
		__nand_cmd(CMD_ERASE);

		__nand_sync();

		__nand_cmd(CMD_READ_STATUS);

		if (__nand_data8() & 0x01) 
		{
			/* Erase Error, mark it as bad block */
			nand_block_markbad(cur);
		
		} else ;

		cur += ppb;
	}

	return 0;
}

static int read_oob(u8 *buf, u32 size, u32 pg)
{
	u32 i, coladdr, rowaddr;

	if (pagesize == 512)
		coladdr = 0;
	else
		coladdr = pagesize;

	if (pagesize == 512)
		/* Send READOOB command */
		__nand_cmd(CMD_READC);
	else
		/* Send READ0 command */
		__nand_cmd(CMD_READA);

	/* Send column address */
	__nand_addr(coladdr & 0xff);
	if (pagesize != 512)
		__nand_addr(coladdr >> 8);

	/* Send page address */
	rowaddr = pg;
	for (i = 0; i < row; i++) {
		__nand_addr(rowaddr & 0xff);
		rowaddr >>= 8;
	}

	/* Send READSTART command for 2048 ps NAND */
	if (pagesize != 512)
		__nand_cmd(CMD_CONFIRM);

	/* Wait for device ready */
	__nand_sync();

	/* Read oob data */
	read_proc(buf, size);

	return 0;
}

/* Correct 1~9-bit errors in 512-bytes data */
static void rs_correct(unsigned char *dat, int idx, int mask)
{
	int i;

	idx--;

	i = idx + (idx >> 3);
	if (i >= 512)
		return;

	mask <<= (idx & 0x7);

	dat[i] ^= mask & 0xff;
	if (i < 511)
		dat[i+1] ^= (mask >> 8) & 0xff;
}

static int nand_hm_correct_data(u8 *dat, u8 *oob_s, u8 *calc_ecc,u8 p)
{
	u8 a, b, c, d1, d2, d3, add, bit, i;
	u8 *e1,*e2,*e3;

	e1 = &oob_s[oob_pos->eccpos[p+0]];
	e2 = &oob_s[oob_pos->eccpos[p+1]];
	e3 = &oob_s[oob_pos->eccpos[p+2]];
//	printf("read ecc :%x %x %x %d %d\n",*e1,*e2,*e3,
//	       oob_pos->eccpos[p+0],oob_pos->eccpos[p+1]);

	d1 = calc_ecc[0] ^ *e1;
	d2 = calc_ecc[1] ^ *e2;
	d3 = calc_ecc[2] ^ *e3;

	if ((d1 | d2 | d3) == 0) {
		/* No errors */
		return 0;
	}
	else {
		a = (d1 ^ (d1 >> 1)) & 0x55;
		b = (d2 ^ (d2 >> 1)) & 0x55;
		c = (d3 ^ (d3 >> 1)) & 0x54;

		/* Found and will correct single bit error in the data */
		if ((a == 0x55) && (b == 0x55) && (c == 0x54)) {
			c = 0x80;
			add = 0;
			a = 0x80;
			for (i=0; i<4; i++) {
				if (d1 & c)
					add |= a;
				c >>= 2;
				a >>= 1;
			}
			c = 0x80;
			for (i=0; i<4; i++) {
				if (d2 & c)
					add |= a;
				c >>= 2;
				a >>= 1;
			}
			bit = 0;
			b = 0x04;
			c = 0x80;
			for (i=0; i<3; i++) {
				if (d3 & c)
					bit |= b;
				c >>= 2;
				b >>= 1;
			}
			b = 0x01;
			a = dat[add];
			a ^= (b << bit);
			dat[add] = a;
			return 0;
		}
		else {
			i = 0;
			while (d1) {
				if (d1 & 0x01)
					++i;
				d1 >>= 1;
			}
			while (d2) {
				if (d2 & 0x01)
					++i;
				d2 >>= 1;
			}
			while (d3) {
				if (d3 & 0x01)
					++i;
				d3 >>= 1;
			}
			if (i == 1) {
				/* ECC Code Error Correction */
				*e1 = calc_ecc[0];
				*e2 = calc_ecc[1];
				*e3 = calc_ecc[2];
				return 0;
			}
			else {
				/* Uncorrectable Error */
//				printf("uncorrectable ECC error\n");
				return -1;
			}
		}
	}
	
	/* Should never happen */
	return -1;
}

 /*
 * Read data <pagenum> pages from <startpage> page.
 * HW ECC is used.
 */
int nand_read_4740_hm(u8 *buf, u32 startpage, u32 pagenum)
{
	u32 j, calc_ecc;
	u32 cur_page, cnt, rowaddr, ecccnt;
	u8 *tmpbuf;
	ecccnt = pagesize / 256;
	
	cur_page = startpage;
	cnt = 0;
	while (cnt < pagenum) {
		/* read oob first */
		read_oob(oob_buf, oobsize, cur_page);

		__nand_sync();
		__nand_cmd(CMD_READA);

		__nand_addr(0);
		if (pagesize == 2048)
			__nand_addr(0);

		rowaddr = cur_page;
		for (j = 0; j < row; j++) {
			__nand_addr(rowaddr & 0xff);
			rowaddr >>= 8;
		}

		if (pagesize == 2048)
			__nand_cmd(CMD_CONFIRM);
		__nand_sync();
		tmpbuf = (u8 *)((u32)buf + cnt * ( pagesize+oobsize));

		for (j = 0; j < ecccnt ; j++) 
		{
			__nand_ecc_enable();
			__nand_select_hm_ecc();
			read_proc(tmpbuf, 256);
			__nand_ecc_disable();
			calc_ecc = __nand_read_hm_ecc();
			if (oob_pos->eccname == LINUXHM)
				calc_ecc = ~calc_ecc | 0x00030000;

			nand_hm_correct_data(tmpbuf,oob_buf,(u8*)&calc_ecc,j*3);
			tmpbuf += 256;
		}

		for (j = 0; j < oobsize; j++)
			tmpbuf[j] = oob_buf[j];

		cur_page++;
		cnt++;

	}
	return 0;
}

 /*
 * Read data <pagenum> pages from <startpage> page.
 * HW ECC is used.
 */
int nand_read_4740_rs(u8 *buf, u32 startpage, u32 pagenum)
{
	u32 j, k;
	u32 cur_page, cnt, rowaddr, ecccnt;
	u8 *tmpbuf;
	ecccnt = pagesize / ECC_BLOCK;
	
	cur_page = startpage;
	cnt = 0;
	while (cnt < pagenum) {
		/* read oob first */
		read_oob(oob_buf, oobsize, cur_page);

		__nand_sync();
		__nand_cmd(CMD_READA);

		__nand_addr(0);
		if (pagesize == 2048)
			__nand_addr(0);

		rowaddr = cur_page;
		for (j = 0; j < row; j++) {
			__nand_addr(rowaddr & 0xff);
			rowaddr >>= 8;
		}

		if (pagesize == 2048)
			__nand_cmd(CMD_CONFIRM);
		__nand_sync();
		tmpbuf = (u8 *)((u32)buf + cnt * ( pagesize+oobsize));

		for (j = 0; j < ecccnt ; j++) {
			volatile u8 *paraddr = (volatile u8 *)EMC_NFPAR0;
			u32 stat;

			/* Read data */
			REG_EMC_NFINTS = 0x0;
			__nand_ecc_rs_decoding();
			read_proc(tmpbuf, ECC_BLOCK);

			/* Set PAR values */
			for (k = 0; k < PAR_SIZE; k++) {
				*paraddr++ = oob_buf[oob_pos->eccpos[j*PAR_SIZE + k]];
			}

			/* Set PRDY */
			REG_EMC_NFECR |= EMC_NFECR_PRDY;

			/* Wait for completion */
			__nand_ecc_decode_sync();
			__nand_ecc_disable();

			/* Check decoding */
			stat = REG_EMC_NFINTS;

			if (stat & EMC_NFINTS_ERR) {
//				printf("Error occured!\n");
				if (stat & EMC_NFINTS_UNCOR) {
					int t;
					for (t = 0; t < oob_pos->eccbytes; t++)
						if (oob_buf[oob_pos->eccpos[t]] != 0xff) break;
					if (t < oob_pos->eccbytes-1) {
//						printf("Uncorrectable error occurred\n");
					}
				}
				else {
					u32 errcnt = (stat & EMC_NFINTS_ERRCNT_MASK) >> EMC_NFINTS_ERRCNT_BIT;
					switch (errcnt) {
					case 4:
						rs_correct(tmpbuf, (REG_EMC_NFERR3 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, (REG_EMC_NFERR3 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
					case 3:
						rs_correct(tmpbuf, (REG_EMC_NFERR2 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, (REG_EMC_NFERR2 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
					case 2:
						rs_correct(tmpbuf, (REG_EMC_NFERR1 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, (REG_EMC_NFERR1 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
					case 1:
						rs_correct(tmpbuf, (REG_EMC_NFERR0 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, (REG_EMC_NFERR0 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
						break;
					default:
						break;
					}

				}
			}
			/* increment pointer */
			tmpbuf += ECC_BLOCK ;
		}

		for (j = 0; j < oobsize; j++)
			tmpbuf[j] = oob_buf[j];

		cur_page++;
		cnt++;
	}

	return 0;
}

int nand_program_4740(u8 *context, int spage, int pages)
{
	u32 i, j, cur, rowaddr;
	u8 *tmpbuf;

	tmpbuf = (u8 *)context;
	i = 0;
	cur = spage;

	while (i < pages) {

		for (j=0;j<np->os;j++)
		{
			if (tmpbuf[j+np->ps]!=0xff)
				break;
		}

		if (j==np->os) 
		{
			tmpbuf += np->ps+np->os;
			i ++;
			cur ++;
			continue;
		}
		if (pagesize != 2048)
			__nand_cmd(CMD_READA);

		__nand_cmd(CMD_SEQIN);

		__nand_addr(0);
		if (pagesize == 2048)
			__nand_addr(0);
		rowaddr = cur;
		for (j = 0; j < row; j++) {
			__nand_addr(rowaddr & 0xff);
			rowaddr >>= 8;
		}

		write_proc(tmpbuf, np->ps+np->os);
		tmpbuf += np->ps+np->os;

		/* send program confirm command */
		__nand_cmd(CMD_PGPROG);
		__nand_sync();

		__nand_cmd(CMD_READ_STATUS);
//		__nand_sync();

		if (__nand_data8() & 0x01) { /* page program error */
			return -1;
		} else ;

		i ++;
		cur ++;
	}
	return 0;
}

static int nand_data_write8(unsigned char *buf, int count)
{
	int i;
	u8 *p = (u8 *)buf;
	for (i=0;i<count;i++)
		__nand_data8() = *p++;
	return 0;
}

static int nand_data_write16(unsigned char *buf, int count)
{
	int i;
	u16 *p = (u16 *)buf;
	for (i=0;i<count/2;i++)
		__nand_data16() = *p++;
	return 0;
}

static int nand_data_read8(unsigned char *buf, int count)
{
	int i;
	u8 *p = (u8 *)buf;
	for (i=0;i<count;i++)
		*p++ = __nand_data8();
	return 0;
}

static int nand_data_read16(unsigned char *buf, int count)
{
	int i;
	u16 *p = (u16 *)buf;
	for (i=0;i<count/2;i++)
		*p++ = __nand_data16();
	return 0;
}

 
