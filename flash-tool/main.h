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

#define INGENIC_REQUEST_TYPE	0x40
#define VR_GET_CPU_INFO		0x00
#define VR_SET_DATA_ADDRESS	0x01
#define VR_SET_DATA_LENGTH	0x02
#define VR_FLUSH_CACHES		0x03
#define VR_PROGRAM_START1	0x04
#define VR_PROGRAM_START2	0x05

#define USB_TIMEOUT 5000


struct ingenic_dev {
	struct usb_device *usb_dev;
	struct usb_dev_handle *usb_handle;
	uint8_t interface;
};
