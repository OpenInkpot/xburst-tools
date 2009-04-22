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



#include "main.h"

#include <usb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


static int get_ingenic_device(struct ingenic_dev *ingenic_dev)
{
	struct usb_bus *usb_busses, *usb_bus;
	struct usb_device *usb_dev;
	int count = 0;

	usb_busses = usb_get_busses();

	for (usb_bus = usb_busses; usb_bus != NULL; usb_bus = usb_bus->next) {
		for (usb_dev = usb_bus->devices; usb_dev != NULL; usb_dev = usb_dev->next) {

			if ((usb_dev->descriptor.idVendor == VENDOR_ID) &&
				(usb_dev->descriptor.idProduct == PRODUCT_ID)) {
				ingenic_dev->usb_dev = usb_dev;
				count++;
			}

		}
	}

	return count;
}

static int get_ingenic_interface(struct ingenic_dev *ingenic_dev)
{
	struct usb_config_descriptor *usb_config_desc;
	struct usb_interface_descriptor *usb_if_desc;
	struct usb_interface *usb_if;
	int config_index, if_index, alt_index;

	for (config_index = 0; config_index < ingenic_dev->usb_dev->descriptor.bNumConfigurations; config_index++) {
		usb_config_desc = &ingenic_dev->usb_dev->config[config_index];

		if (!usb_config_desc)
			return 0;

		for (if_index = 0; if_index < usb_config_desc->bNumInterfaces; if_index++) {
			usb_if = &usb_config_desc->interface[if_index];

			if (!usb_if)
				return 0;

			for (alt_index = 0; alt_index < usb_if->num_altsetting; alt_index++) {
				usb_if_desc = &usb_if->altsetting[alt_index];

				if (!usb_if_desc)
					return 0;

				if ((usb_if_desc->bInterfaceClass == 0xff) &&
					(usb_if_desc->bInterfaceSubClass == 0)) {
					ingenic_dev->interface = usb_if_desc->bInterfaceNumber;
					return 1;
				}
			}
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int num_ingenic, status;
	int res = EXIT_FAILURE;
	char cpu_buff[8];
	struct ingenic_dev ingenic_dev;

	if ((getuid()) || (getgid())) {
		fprintf(stderr, "Error - you must be root to run '%s'\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&ingenic_dev, 0, sizeof(struct ingenic_dev));
	memset(cpu_buff, 0, sizeof(cpu_buff));

	usb_init();
// 	usb_set_debug(255);
	usb_find_busses();
	usb_find_devices();

	num_ingenic = get_ingenic_device(&ingenic_dev);

	if (num_ingenic == 0) {
		fprintf(stderr, "Error - no Ingenic device found\n");
		goto exit;
	}

	if (num_ingenic > 1) {
		fprintf(stderr, "Error - too many Ingenic devices found: %i\n", num_ingenic);
		goto exit;
	}

	ingenic_dev.usb_handle = usb_open(ingenic_dev.usb_dev);
	if (!ingenic_dev.usb_handle) {
		fprintf(stderr, "Error - can't open Ingenic device: %s\n", usb_strerror());
		goto exit;
	}

	if (get_ingenic_interface(&ingenic_dev) < 1) {
		fprintf(stderr, "Error - can't find Ingenic interface\n");
		goto close_handle;
	}

	if (usb_claim_interface(ingenic_dev.usb_handle, ingenic_dev.interface) < 0) {
		fprintf(stderr, "Error - can't claim Ingenic interface: %s\n", usb_strerror());
		goto close_handle;
	}

	status = usb_control_msg(ingenic_dev.usb_handle,
          /* bmRequestType */ INGENIC_REQUEST_TYPE,
          /* bRequest      */ VR_GET_CPU_INFO,
          /* wValue        */ 0,
          /* wIndex        */ 0,
          /* Data          */ cpu_buff,
          /* wLength       */ 8,
                              USB_TIMEOUT);

	if (status != sizeof(cpu_buff)) {
		fprintf(stderr, "Error - can't retrieve Ingenic CPU type: %i\n", status);
		goto release_if;
	}

	printf("Found cpu: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n",
		cpu_buff[0], cpu_buff[1], cpu_buff[2], cpu_buff[3],
		cpu_buff[4], cpu_buff[5], cpu_buff[6], cpu_buff[7]);

release_if:
	usb_release_interface(ingenic_dev.usb_handle, ingenic_dev.interface);
close_handle:
	usb_close(ingenic_dev.usb_handle);
exit:
	exit(res);
}

