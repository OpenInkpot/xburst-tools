#ifndef __HAND_H__
#define __HAND_H__

struct fw_args_t {
	/* CPU ID */
	unsigned int  cpu_id;

	/* PLL args */
	unsigned char ext_clk;
	unsigned char cpu_speed;
	unsigned char phm_div;
	unsigned char use_uart;
	unsigned int  boudrate;

	/* SDRAM args */
	unsigned char bus_width;
	unsigned char bank_num;
	unsigned char row_addr;
	unsigned char col_addr;
	unsigned char is_mobile;
	unsigned char is_busshare;

	/* debug args */
	unsigned char debug_ops;
	unsigned char pin_num;
	unsigned int  start;
	unsigned int  size;

	/* for align */
	/* unsigned char align1; */
	/* unsigned char align2; */
} __attribute__((packed));

struct hand_t {

	/* nand flash info */
	int pt;             	/* cpu type */
	unsigned int nand_bw;		/* bus width */
	unsigned int nand_rc;		/* row cycle */
	unsigned int nand_ps;		/* page size */
	unsigned int nand_ppb;		/* page number per block */
	unsigned int nand_force_erase;
	unsigned int nand_pn;		/* page number in total */
	unsigned int nand_os;		/* oob size */
	unsigned int nand_eccpos;
	unsigned int nand_bbpage;
	unsigned int nand_bbpos;
	unsigned int nand_plane;
	unsigned int nand_bchbit;
	unsigned int nand_wppin;
	unsigned int nand_bpc;		/* block number per chip */

	struct fw_args_t fw_args;
} __attribute__((packed));

#endif /* __HAND_H__ */
