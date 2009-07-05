/*
 * Authors: Xiangfu Liu <xiangfu@qi-hardware.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#ifndef __INGENIC_CFG_H__
#define __INGENIC_CFG_H__

#include "usb_boot_defines.h"

#define CONFIG_FILE_PATH "/usr/share/xburst-tools/usbboot.cfg"

int hand_init_def(struct hand *hand);
int check_dump_cfg(struct hand *hand);
int parse_configure(struct hand *hand, char * file_path);

#endif	/*__INGENIC_CFG_H__ */
