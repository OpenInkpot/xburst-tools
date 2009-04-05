#ifndef _CONFIGS_H
#define _CONFIGS_H

//Here are these common definitions
//Once your system configration change,just modify the file

#define CONFIG_NR_DRAM_BANKS	1  /* SDRAM BANK Number: 1, 2*/
#define SDRAM_CASL		3	/* CAS latency: 2 or 3 */
// SDRAM Timings, unit: ns
#define SDRAM_TRAS		45	/* RAS# Active Time */
#define SDRAM_RCD		20	/* RAS# to CAS# Delay */
#define SDRAM_TPC		20	/* RAS# Precharge Time */
#define SDRAM_TRWL		7	/* Write Latency Time */
#define SDRAM_TREF	        15625	/* Refresh period: 4096 refresh cycles/64ms */

extern volatile u32 CPU_ID;
extern volatile u8 SDRAM_BW16;
extern volatile u8 SDRAM_BANK4;
extern volatile u8 SDRAM_ROW;
extern volatile u8 SDRAM_COL;
extern volatile u8 CONFIG_MOBILE_SDRAM;
extern volatile u32 CFG_CPU_SPEED;
extern volatile u8 PHM_DIV;
extern volatile u32 CFG_EXTAL;
extern volatile u32 CONFIG_BAUDRATE;
extern volatile u32 UART_BASE;
extern volatile u8 CONFIG_MOBILE_SDRAM;
extern volatile u8 IS_SHARE;

typedef struct {
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
}fw_args_t;

extern void gpio_init_4740(void);
extern void sdram_init_4740(void);
extern void serial_init_4740(void);
extern void pll_init_4740(void);

extern void gpio_init_4750(void);
extern void sdram_init_4750(void);
extern void serial_init_4750(void);
extern void pll_init_4750(void);

extern void serial_puts(const char *s);

#endif 
