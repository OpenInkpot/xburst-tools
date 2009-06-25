#
# Authors: Lucifer at Ingenic Semiconductor Inc.
#          Xiangfu Liu <xiangfu.z@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
#

FLASH_TOOL_SRC_PATH = ../src

ifeq ($(CROSS_COMPILE),)
$(error CROSS_COMPILE variable not set, should point to .../mipsel-openwrt-linux-)
endif

CC	:= $(CROSS_COMPILE)gcc
AR	:= $(CROSS_COMPILE)ar rcsv
LD	:= $(CROSS_COMPILE)ld
OBJCOPY	:= $(CROSS_COMPILE)objcopy
NM	:= $(CROSS_COMPILE)nm
OBJDUMP	:= $(CROSS_COMPILE)objdump

CFLAGS	:= -mips32 -O2 -fno-exceptions -ffunction-sections \
	-fomit-frame-pointer -msoft-float -G 0
LDFLAGS	:= -nostdlib -T target.ld $(CFLAGS)
LIBS	:= -lstdc++ -lc -lm -lgcc

USBBOOTDIR	:= .

CFLAGS += -I$(USBBOOTDIR)/usb_boot \
	-I$(USBBOOTDIR)/nandflash -I$(USBBOOTDIR)/include \
	-I$(FLASH_TOOL_SRC_PATH)

OBJS	:= main.o udc.o cache.o serial.o boothandler.o nandflash_4740.o nandflash_4750.o

VPATH	:= $(USBBOOTDIR)/usb_boot $(USBBOOTDIR)/nandflash

all: xburst_stage2.elf
	$(OBJCOPY) -O binary xburst_stage2.elf xburst_stage2.bin
	$(OBJDUMP) -D xburst_stage2.elf > xburst_stage2.dump
	$(NM) xburst_stage2.elf | sort > xburst_stage2.sym
	$(OBJDUMP) -h xburst_stage2.elf > xburst_stage2.map

xburst_stage2.elf: head.o $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
.cpp.o:
	$(CC) $(CFLAGS) -fno-rtti -fvtable-gc -o $@ -c $<
.S.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(addprefix xburst_stage2., bin dump elf map sym)
	rm -f head.o $(OBJS)
