#ifndef __NAND_ECC_H__
#define __NAND_ECC_H__

#include "include.h"

// This head file define these ecc position and ecc types
struct nand_oobinfo oob_64[] = 
{
{
	.eccname = JZ4730CPU,
	.eccbytes = 24,
	.eccpos = 
	{
		4, 5, 6, 
		8, 9, 10, 
		12,13,14,
		16,17,18,
		20,21,22,
		24,25,26,
		28,29,30,
		32,33,34,
	},
},
{
	.eccname = LINUXHM,
	.eccbytes = 24,
	.eccpos = 
	{
		41, 40, 42, 
		44, 43, 45, 
		47, 46, 48, 
		50, 49, 51, 
		53, 52, 54, 
		56, 55, 57, 
		59, 58, 60, 
		62, 61, 63

/* old need change position
		40, 41, 42, 
		43, 44, 45, 
		46, 47, 48, 
		49, 50, 51, 
		52, 53, 54, 
		55, 56, 57, 
		58, 59, 60, 
		61, 62, 63

 */
	},
},
{
	.eccname = JZ4740CPU,
	.eccbytes = 36,
	.eccpos = 
	{
		6, 7, 8, 9, 10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,
		24,25,26,27,28,29,30,31,32,
		33,34,35,36,37,38,39,40,41
	},

},
{
	.eccname = LINUXRS,
	.eccbytes = 36,
	.eccpos = 
	{
		28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 
		40, 41, 42, 43, 44, 45, 46, 47, 
		48, 49, 50, 51, 52, 53, 54, 55, 
		56, 57, 58, 59, 60, 61, 62, 63
	},

},

{       //this one must update by config file
	.eccname = USERSPEC,
	.eccbytes = 64,
	.eccpos = 
	{
		0, 0, 0, 0,
	},
	
},

};

#endif
