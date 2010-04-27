AM_CFLAGS = -pedantic -Wall -W -O1 -g3 -std=gnu99 -lusb

xbboot_version.h:
	echo -e '#ifndef XBBOOT_VERSION' \
	'\n#define XBBOOT_VERSION "20100427-1"' \
	'\n#endif' > xbboot_version.h
BUILT_SOURCES = xbboot_version.h

bin_PROGRAMS = xbboot
xbboot_SOURCES = host_main.c

prefix = /usr
datadir = /usr/share/xburst-tools
data_DATA = ../target-stage1/stage1.bin

EXTRA_DIST = $(datadir)

../target-stage1/stage.bin:
	$(MAKE) -C ../target-stage1
