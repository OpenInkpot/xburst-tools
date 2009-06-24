AM_CFLAGS = -pedantic -Wall -W -O1 -g3 -std=gnu99 -lusb -lconfuse

inflash_version.h:
	echo -e '#ifndef INFLASH_VERSION' \
	'\n#define INFLASH_VERSION "'`svnversion`'"' \
	'\n#endif' > inflash_version.h
BUILT_SOURCES = inflash_version.h

bin_PROGRAMS = inflash
inflash_SOURCES = cmd.c  command_line.c  ingenic_cfg.c  \
	ingenic_usb.c  main.c

datadir = /usr/share/inflash
data_DATA = ../device_stage1/fw.bin ../device_stage2/usb_boot.bin
EXTRA_DIST = $(datadir)