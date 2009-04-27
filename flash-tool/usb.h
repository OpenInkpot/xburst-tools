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



#define INGENIC_OUT_ENDPOINT	0x01

#define VR_GET_CPU_INFO		0x00
#define VR_SET_DATA_ADDRESS	0x01
#define VR_SET_DATA_LENGTH	0x02
#define VR_FLUSH_CACHES		0x03
#define VR_PROGRAM_START1	0x04
#define VR_PROGRAM_START2	0x05
#define VR_NOR_OPS		0x06
#define VR_NAND_OPS		0x07
#define VR_SDRAM_OPS		0x08
#define VR_CONFIGRATION		0x09
#define VR_GET_NUM		0x0a

#define STAGE1_ADDR_MSB (0x80002000 >> 16)
#define STAGE1_ADDR_LSB (0x80002000 & 0xffff)
#define STAGE2_ADDR_MSB 0x8000
#define STAGE2_ADDR_LSB 0x0000

#define USB_PACKET_SIZE 512
#define USB_TIMEOUT 5000


int usb_ingenic_init(struct ingenic_dev *ingenic_dev);
int usb_get_ingenic_cpu(struct ingenic_dev *ingenic_dev);
int usb_ingenic_upload(struct ingenic_dev *ingenic_dev, int stage);
void usb_ingenic_cleanup(struct ingenic_dev *ingenic_dev);
