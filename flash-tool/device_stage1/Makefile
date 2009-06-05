#
# Makefile for the JZ4740 internal boot rom code.
#
# (C) Copyright 2009
# Author: Xiangfu Liu <xiangfu.z@gmail.com>
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

FLASH_TOOL_PATH = ..

CROSS_COMPILE ?= mipsel-linux-

CFLAGS	:= -O2 -mips32 -fno-pic -mno-abicalls -I..
LDFLAGS	:= -nostdlib -EL -T target.ld 

OBJS	= head.o main.o common.o board_4740.o board_4750.o debug.o

all: fw.bin

fw.bin: fw.elf
	$(CROSS_COMPILE)objcopy -O binary $< $@
	$(CROSS_COMPILE)objdump -D $< > fw.dump
	$(CROSS_COMPILE)objdump -h $< > fw.map
	$(CROSS_COMPILE)nm -n $< > System.map
	cp fw.bin $(FLASH_TOOL_PATH)

fw.elf: $(OBJS)
	$(CROSS_COMPILE)ld $(LDFLAGS) $(OBJS) -o $@
.c.o:
	$(CROSS_COMPILE)gcc $(CFLAGS) -c $< -o $@
.S.o:
	$(CROSS_COMPILE)gcc $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o *.elf *.bin *.dump *.map
	rm -f *~
