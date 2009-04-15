/*
 * Ingenic driver
 *
 * Copyright (C) 2009 Pi,  Xiangfu Liu (xiangfu.z@gmail.com)
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <linux/config.h>
#ifdef CONFIG_USB_DEBUG
	#define DEBUG	1
#endif
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>


#define DRIVER_AUTHOR "xiangfu, xiangfu.z@gmail.com"
#define DRIVER_DESC "Ingenic driver"

#define VENDOR_ID	0x601a
#define PRODUCT_ID	0x4740

/* table of devices that work with this driver */
static struct usb_device_id ingenic_table [] = {
	{ USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{ },
};
MODULE_DEVICE_TABLE (usb, ingenic_table);

struct usb_ingenic {
	struct usb_device *	udev;
};

static int ingenic_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct usb_ingenic *dev = NULL;
	int retval = -ENOMEM;

	dev = kmalloc(sizeof(struct usb_ingenic), GFP_KERNEL);
	if (dev == NULL) {
		dev_err(&interface->dev, "Out of memory\n");
		goto error;
	}
	memset (dev, 0x00, sizeof (*dev));

	dev->udev = usb_get_dev(udev);

	usb_set_intfdata (interface, dev);

	dev_info(&interface->dev, "Ingenic device now attached\n");
	return 0;

error:
	kfree(dev);
	return retval;
}

static void ingenic_disconnect(struct usb_interface *interface)
{
	struct usb_ingenic *dev;

	dev = usb_get_intfdata (interface);
	usb_set_intfdata (interface, NULL);

	usb_put_dev(dev->udev);

	kfree(dev);

	dev_info(&interface->dev, "Ingenic device now disconnected\n");
}

static struct usb_driver ingenic_driver = {
	.owner =	THIS_MODULE,
	.name =		"Ingenic",
	.probe =	ingenic_probe,
	.disconnect =	ingenic_disconnect,
	.id_table =	id_table,
};

static int __init usb_ingenic_init(void)
{
	int retval = 0;

	retval = usb_register(&ingenic_driver);
	if (retval)
		err("usb_register failed. Error number %d", retval);
	return retval;
}

static void __exit usb_ingenic_exit(void)
{
	usb_deregister(&ingenic_driver);
}

module_init (usb_ingenic_init);
module_exit (usb_ingenic_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
