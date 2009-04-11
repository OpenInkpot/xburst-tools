# 
# 
# Copyright (C) 2006 Ingenic Semiconductor Inc.
# By Lucifer
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
# 

# -----------------------------

CC	:= mipsel-linux-gcc
AR	:= mipsel-linux-ar rcsv
LD	:= mipsel-linux-ld
OBJCOPY	:= mipsel-linux-objcopy
NM	:= mipsel-linux-nm
OBJDUMP	:= mipsel-linux-objdump

CFLAGS	:= -mips32 -O2  -FPIC \
	   -fno-exceptions -ffunction-sections \
	   -fomit-frame-pointer -msoft-float -G 0

#CFLAGS	:= -mips32 -O2  -FPIC -mno-abicalls -fno-builtin \

LDFLAGS	:= -nostdlib -T target.ld $(CFLAGS)

LIBS	:= -lstdc++ -lc -lm -lgcc

USBBOOTDIR := .

LIBDIR	:= 

#SOURCES += $(wildcard $(USBBOOTDIR)/usb_boot/*.c) 

SOURCES := ./usb_boot/main.c ./usb_boot/udc.c ./usb_boot/cache.c ./usb_boot/serial.c ./usb_boot/boothandler.c
SOURCES += $(wildcard $(USBBOOTDIR)/nandflash/*.c) 
SOURCES	+= $(wildcard $(USBBOOTDIR)/usb_boot/*.S)

HEADS	:= $(SOCDIR)/head.S 

CFLAGS	+= -I$(SOCDIR)/include  -I$(USBBOOTDIR)/usb_boot  -I$(USBBOOTDIR)/nandflash   -I$(USBBOOTDIR)/include

OBJS	:= $(addsuffix .o , $(basename $(notdir $(SOURCES))))
HEADO	:= $(addsuffix .o , $(basename $(notdir $(HEADS))))

TARGET  := usb_boot
APP	:= $(TARGET).elf 

VPATH	:= $(ARCHDIR) $(SOCDIR) $(OSDIR) $(USBBOOTDIR)/usb_boot  $(USBBOOTDIR)/norflash $(USBBOOTDIR)/nandflash 

all:	$(APP)
	$(OBJCOPY) -O binary $(APP) $(TARGET).bin
	$(OBJDUMP) -D $(APP) > $(TARGET).dump
	$(NM) $(APP) | sort > $(TARGET).sym
	$(OBJDUMP) -h $(APP) > $(TARGET).map
	cp usb_boot.bin ../../../tools

$(APP):	$(HEADO) $(OBJS) $(EXTLIBS) 
	$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
.cpp.o:
	$(CC) $(CFLAGS) -fno-rtti -fvtable-gc -o $@ -c $<
.S.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -fr *.o $(APP) $(OBJS) core $(OTHER) *.sym *.map *.dump *.bin *.lib *.~ *.\#

