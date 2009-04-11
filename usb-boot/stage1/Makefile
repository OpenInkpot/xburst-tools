#
# Copyright (C) 2006 Ingenic Semiconductor Inc.
#
# Makefile for the JZ4740 internal bootrom code.
#

CROSS	:= mipsel-linux-

CFLAGS	:= -O2 -mips32 -fno-pic -mno-abicalls
LDFLAGS	:= -nostdlib -EL -T target.ld 

OBJS	= head.o main.o common.o board_4740.o board_4750.o debug.o

all: fw.bin

fw.bin: fw.elf
	$(CROSS)objcopy -O binary $< $@
	$(CROSS)objdump -D $< > fw.dump
	$(CROSS)objdump -h $< > fw.map
	$(CROSS)nm -n $< > System.map
	cp fw.bin ../../../../tools

fw.elf: $(OBJS)
	$(CROSS)ld $(LDFLAGS) $(OBJS) -o $@
.c.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
.S.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
clean:
	rm -fr *.o *.elf *.bin *.dump *.map
