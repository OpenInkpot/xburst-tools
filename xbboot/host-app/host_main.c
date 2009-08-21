//
// Authors: Wolfgang Spraul <wolfgang@qi-hardware.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 3 of the License, or (at your option) any later version.
//

#include <stdio.h>
#include <string.h>
#include <usb.h>
#include "xbboot_version.h"

#define HIWORD(dw)	(((dw) >> 16) & 0xFFFF)
#define LOWORD(dw)	((dw) & 0xFFFF)

#define INGENIC_VENDOR_ID	0x601A
#define INGENIC_XBURST_USBBOOT	0x4740

// REQ_ values are negative so they can be mixed together with the VR_ types in a signed integer.
#define REQ_BULK_READ		-1
#define REQ_BULK_WRITE		-2

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

#define USB_TIMEOUT		3000
#define VR_GET_CPU_INFO_LEN	8
#define INGENIC_IN_ENDPOINT	0x81
#define INGENIC_OUT_ENDPOINT	0x01

int main(int argc, char** argv)
{
	struct usb_device* xburst_dev = 0;
	uint8_t xburst_interface = 0;
	struct usb_dev_handle* xburst_h = 0;
	int request_type, usb_status;

	if (argc < 2
	    || !strcmp(argv[1], "-h")
	    || !strcmp(argv[1], "--help")) {
		printf("\n"
		       "xbboot version %s - Ingenic XBurst USB Boot Vendor Requests\n"
		       "(c) 2009 Wolfgang Spraul\n"
		       "Report bugs to <wolfgang@qi-hardware.com>.\n"
		       "\n"
		       "xbboot [vendor_request] ... (must run as root)\n"
		       "  -h --help                                 print this help message\n"
		       "  -v --version                              print the version number\n"
		       "\n"
		       "  bulk_read <len>                           read len bulk bytes from USB, write to stdout\n"
		       "  bulk_write <path>                         write file at <path> to USB\n"
		       "  [get_info | VR_GET_CPU_INFO]              read 8-byte CPU info and write to stdout\n"
		       "  [set_addr | VR_SET_DATA_ADDRESS] <addr>   send memory address\n"
		       "  [set_len | VR_SET_DATA_LENGTH] <len>      send data length\n"
		       "  [flush_cache | VR_FLUSH_CACHES]           flush I-Cache and D-Cache\n"
		       "  [start1 | VR_PROGRAM_START1] <addr>       transfer data from D-Cache to I-Cache and branch to I-Cache\n"
		       "  [start2 | VR_PROGRAM_START2] <addr>       branch to <addr> directly\n"
		       "\n"
		       "- all numbers can be prefixed 0x for hex otherwise decimal\n"
		       "\n", XBBOOT_VERSION);
// stage1: 0x80002000
// stage2: 0x81C00000
// u-boot: 0x80600000
// uImage: 0x80010000
		return EXIT_SUCCESS;
	}
	if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
		printf("xbboot version %s\n", XBBOOT_VERSION);
		return EXIT_SUCCESS;
	}
	if (!strcmp(argv[1], "bulk_read"))
		request_type = REQ_BULK_READ;
	else if (!strcmp(argv[1], "bulk_write"))
		request_type = REQ_BULK_WRITE;
	else if (!strcmp(argv[1], "VR_GET_CPU_INFO") || !strcmp(argv[1], "get_info"))
		request_type = VR_GET_CPU_INFO;
	else if (!strcmp(argv[1], "VR_SET_DATA_ADDRESS") || !strcmp(argv[1], "set_addr"))
		request_type = VR_SET_DATA_ADDRESS;
	else if (!strcmp(argv[1], "VR_SET_DATA_LENGTH") || !strcmp(argv[1], "set_len"))
		request_type = VR_SET_DATA_LENGTH;
	else if (!strcmp(argv[1], "VR_FLUSH_CACHES") || !strcmp(argv[1], "flush_cache"))
		request_type = VR_FLUSH_CACHES;
	else if (!strcmp(argv[1], "VR_PROGRAM_START1") || !strcmp(argv[1], "start1"))
		request_type = VR_PROGRAM_START1;
	else if (!strcmp(argv[1], "VR_PROGRAM_START2") || !strcmp(argv[1], "start2"))
		request_type = VR_PROGRAM_START2;
	else {
		fprintf(stderr, "Error - unknown vendor request %s - run with --help to see all requests\n", argv[1]);
		return EXIT_FAILURE;
	}
	if ((getuid()) || (getgid())) {
		fprintf(stderr, "Error - you must be root to run '%s'\n", argv[0]);
		return EXIT_FAILURE;
	}

	usb_init();
	usb_find_busses();
	usb_find_devices();

	// look for Ingenic XBurst USB boot device & interface
	{
		{
			struct usb_bus* usb_bus;
			struct usb_device* usb_dev;

			for (usb_bus = usb_get_busses(); usb_bus != 0; usb_bus = usb_bus->next) {
				for (usb_dev = usb_bus->devices; usb_dev != 0; usb_dev = usb_dev->next) {
					if (usb_dev->descriptor.idVendor == INGENIC_VENDOR_ID
					    && usb_dev->descriptor.idProduct == INGENIC_XBURST_USBBOOT) {
						if (xburst_dev) {
							fprintf(stderr, "Error - more than one XBurst boot device found.\n");
							goto xout;
						}
						xburst_dev = usb_dev;
						// keep searching to make sure there is only 1 XBurst device
					}
				}
			}
			if (!xburst_dev) {
				fprintf(stderr, "Error - no XBurst boot device found.\n");
				goto xout;
			}
		}
		{
			struct usb_config_descriptor* usb_config_desc;
			struct usb_interface_descriptor* usb_if_desc;
			struct usb_interface* usb_if;
			int cfg_index, if_index, alt_index;

			for (cfg_index = 0; cfg_index < xburst_dev->descriptor.bNumConfigurations; cfg_index++) {
				usb_config_desc = &xburst_dev->config[cfg_index];
				if (!usb_config_desc) {
					fprintf(stderr, "Error - usb_config_desc NULL\n");
					goto xout;
				}
				for (if_index = 0; if_index < usb_config_desc->bNumInterfaces; if_index++) {
					usb_if = &usb_config_desc->interface[if_index];
					if (!usb_if) {
						fprintf(stderr, "Error - usb_if NULL\n");
						goto xout;
					}
					for (alt_index = 0; alt_index < usb_if->num_altsetting; alt_index++) {
						usb_if_desc = &usb_if->altsetting[alt_index];
						if (!usb_if_desc) {
							fprintf(stderr, "Error - usb_if_desc NULL\n");
							goto xout;
						}
						if (usb_if_desc->bInterfaceClass == 0xFF
						    && usb_if_desc->bInterfaceSubClass == 0) {
							xburst_interface = usb_if_desc->bInterfaceNumber;
							goto interface_found;
						}
					}
				}
			}
		interface_found: ;
		}
	}
	xburst_h = usb_open(xburst_dev);
	if (!xburst_h) {
		fprintf(stderr, "Error - can't open XBurst device: %s\n", usb_strerror());
		goto xout;
	}
	if (usb_claim_interface(xburst_h, xburst_interface) < 0) {
		fprintf(stderr, "Error - can't claim XBurst interface: %s\n", usb_strerror());
		goto xout_xburst_h;
	}
	switch (request_type) {
		case VR_GET_CPU_INFO: {
			char cpu_info_buf[VR_GET_CPU_INFO_LEN+1] = {0};
			usb_status = usb_control_msg(xburst_h,
				/* requesttype */ USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				/* request     */ VR_GET_CPU_INFO,
				/* value       */ 0,
				/* index       */ 0,
				/* bytes       */ cpu_info_buf,
				/* size        */ VR_GET_CPU_INFO_LEN,
				/* timeout     */ USB_TIMEOUT);
			if (usb_status != VR_GET_CPU_INFO_LEN) {
				fprintf(stderr, "Error - %s() returned %i\n", argv[1], usb_status);
				goto xout_xburst_interface;
			}
			printf("VR_GET_CPU_INFO %s\n", cpu_info_buf);
			break;
		}
		case VR_SET_DATA_ADDRESS:
		case VR_SET_DATA_LENGTH:
		case VR_PROGRAM_START1:
		case VR_PROGRAM_START2: {
			uint32_t u32_param;
			if (argc != 3) {
				fprintf(stderr, "Error - number of %s parameters %i\n", argv[1], argc);
				goto xout_xburst_interface;
			}
			if (argv[2][0] == '0' && argv[2][1] == 'x')
				u32_param = strtoul(&argv[2][2], 0 /* endptr */, 16 /* base */);
			else
				u32_param = strtoul(argv[2], 0 /* endptr */, 10 /* base */);

			usb_status = usb_control_msg(xburst_h,
				/* requesttype */ USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				/* request     */ request_type,
				/* value       */ HIWORD(u32_param),
				/* index       */ LOWORD(u32_param),
				/* bytes       */ 0,
				/* size        */ 0,
				/* timeout     */ USB_TIMEOUT);
			if (usb_status) {
				fprintf(stderr, "Error - %s() returned %i\n", argv[1], usb_status);
				goto xout_xburst_interface;
			}
			printf("%s %lxh\n", argv[1], (unsigned long) u32_param);
			break;
		}
		case VR_FLUSH_CACHES: {
			usb_status = usb_control_msg(xburst_h,
				/* requesttype */ USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				/* request     */ VR_FLUSH_CACHES,
				/* value       */ 0,
				/* index       */ 0,
				/* bytes       */ 0,
				/* size        */ 0,
				/* timeout     */ USB_TIMEOUT);
			if (usb_status) {
				fprintf(stderr, "Error - %s() returned %i\n", argv[1], usb_status);
				goto xout_xburst_interface;
			}
			printf("VR_FLUSH_CACHES\n");
			break;
		}
		case REQ_BULK_READ: {
			int read_len;
			char* read_buf;

			if (argc != 3) {
				fprintf(stderr, "Error - number of %s parameters %i\n", argv[1], argc);
				goto xout_xburst_interface;
			}
			if (argv[2][0] == '0' && argv[2][1] == 'x')
				read_len = strtol(&argv[2][2], 0 /* endptr */, 16 /* base */);
			else
				read_len = strtol(argv[2], 0 /* endptr */, 10 /* base */);
			read_buf = (char*) malloc(read_len);
			if (!read_buf) {
				fprintf(stderr, "Error - cannot allocate %i bytes read buffer.\n", read_len);
				goto xout_xburst_interface;
			}
			usb_status = usb_bulk_read(xburst_h,
				/* endpoint */	INGENIC_IN_ENDPOINT,
				/* bytes */	read_buf,
				/* size */	read_len,
				/* timeout */	USB_TIMEOUT);
			if (usb_status > 0)
				fwrite(read_buf, 1 /* size */, usb_status, stdout);
			free(read_buf);
			if (usb_status < read_len) {
				fprintf(stderr, "Error reading %d bytes (result %i).\n", read_len, usb_status);
				goto xout_xburst_interface;
			}
			break;
		}
		case REQ_BULK_WRITE: {
			char* file_data;
			int file_len;
			FILE* file_h;
			size_t num_read;

			if (argc != 3) {
				fprintf(stderr, "Error - number of parameters %i\n", argc);
				goto xout_xburst_interface;
			}
			file_h = fopen(argv[2], "rb");
			if (!file_h) {
				fprintf(stderr, "Error opening %s.\n", argv[2]);
				goto xout_xburst_interface;
			}
			if (fseek(file_h, 0, SEEK_END)) {
				fprintf(stderr, "Error seeking to end of %s.\n", argv[2]);
				fclose(file_h);
				goto xout_xburst_interface;
			}
			file_len = ftell(file_h);
			if (fseek(file_h, 0, SEEK_SET)) {
				fprintf(stderr, "Error seeking to beginning of %s.\n", argv[2]);
				fclose(file_h);
				goto xout_xburst_interface;
			}
			file_data = (char*) malloc(file_len);
			if (!file_data) {
				fprintf(stderr, "Error allocating %d bytes data.\n", file_len);
				fclose(file_h);
				goto xout_xburst_interface;
			}
			num_read = fread(file_data, 1, file_len, file_h);
			fclose(file_h);
			if (num_read != (size_t) file_len) {
				fprintf(stderr, "Error reading %d bytes (got %d).\n", file_len, num_read);
				free(file_data);
				goto xout_xburst_interface;
			}
			usb_status = usb_bulk_write(xburst_h,
				/* endpoint */	INGENIC_OUT_ENDPOINT,
				/* bytes */	file_data,
				/* size */	file_len,
				/* timeout */	USB_TIMEOUT);
			free(file_data);
			if (usb_status < file_len) {
				fprintf(stderr, "Error writing %d bytes (result %i).\n", file_len, usb_status);
				goto xout_xburst_interface;
			}
			printf("bulk_write successfully wrote %i bytes.\n", usb_status);
			break;
		}
	}
	usb_release_interface(xburst_h, xburst_interface);
	usb_close(xburst_h);
	return EXIT_SUCCESS;

xout_xburst_interface:
	usb_release_interface(xburst_h, xburst_interface);
xout_xburst_h:
	usb_close(xburst_h);
xout:
	return EXIT_FAILURE;
}
