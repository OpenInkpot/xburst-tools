/*
 * put all the configure operate to this file
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

#ifndef __INGENIC_CFG_H__
#define __INGENIC_CFG_H__

#include "usb_boot_defines.h"

#define CONFIG_FILE_PATH "usb_boot.cfg"

int hand_init_def(struct hand_t *hand);
int check_dump_cfg(struct hand_t *hand);
int parse_configure(struct hand_t *hand, char * file_path);

#endif	/*__INGENIC_CFG_H__ */
