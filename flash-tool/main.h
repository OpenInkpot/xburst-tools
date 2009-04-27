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



#include <stdint.h>

#define VENDOR_ID	0x601a
#define PRODUCT_ID	0x4740

#define STAGE1_FILE_PATH "fw.bin"
#define STAGE2_FILE_PATH "usb_boot.bin"
#define CONFIG_FILE_PATH "usb_boot.cfg"


struct ingenic_dev {
	struct usb_device *usb_dev;
	struct usb_dev_handle *usb_handle;
	uint8_t interface;
	char cpu_info_buff[9];
	char *file_buff;
	int file_len;
};

unsigned int total_size;

