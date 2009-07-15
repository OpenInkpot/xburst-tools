# "PanGu Makefile" - for setting up the PanGu development environment
#
# Copyright 2009 (C) Qi Hardware inc.,
# Author: Xiangfu Liu <xiangfu@qi-hardware.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 3 as published by the Free Software Foundation.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301, USA

# for the device stage
FLASH_TOOL_PATH = ./usbboot
STAGE1_PATH = $(FLASH_TOOL_PATH)/xburst_stage1
STAGE2_PATH = $(FLASH_TOOL_PATH)/xburst_stage2
CROSS_COMPILE ?= mipsel-openwrt-linux-

CFLAGS="-O2"

### u-boot
.PHONY: u-boot
u-boot:
	git clone git://github.com/xiangfu/qi-u-boot.git u-boot
	cd u-boot && \
	make qi_lb60_config && \
	make

### kernel
.PHONY: kernel
kernel:
	git clone git://github.com/xiangfu/qi-kernel.git kernel
	cd kernel && \
	make pi_defconfig && \
	make uImage

### flash-boot
.PHONY: usbboot
usbboot: stage1 stage2
	cd $(FLASH_TOOL_PATH)
	./autogen.sh && \
	./configure && \
	make

stage1:
	make CROSS_COMPILE=$(CROSS_COMPILE) -C $(STAGE1_PATH)

stage2:
	make CROSS_COMPILE=$(CROSS_COMPILE) -C $(STAGE2_PATH)

### clean up
distclean: clean clean-usbboot

clean:

clean-usbboot:
	make clean CROSS_COMPILE=$(CROSS_COMPILE) -C $(STAGE1_PATH)
	make clean CROSS_COMPILE=$(CROSS_COMPILE) -C $(STAGE2_PATH)
	make clean -C $(FLASH_TOOL_PATH)

help:
	@make --print-data-base --question |	\
	awk '/^[^.%][-A-Za-z0-9_]*:/		\
	{ print substr($$1, 1, length($$1)-1) }' | 	\
	sort |	\
	pr --omit-pagination --width=80 --columns=1
