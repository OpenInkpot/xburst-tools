AM_CFLAGS = -pedantic -Wall -W -O1 -g3 -std=gnu99 -lusb -lconfuse

BUILD_BRANCH := $(shell git branch | grep ^\* | cut -d' ' -f2)
BUILD_HEAD := $(shell git show --pretty=oneline | head -n1 | cut -d' ' -f1 | cut -b1-16)
BUILD_VERSION := ${BUILD_BRANCH}_${BUILD_HEAD}

inflash_version.h:
	echo -e '#ifndef INFLASH_VERSION' \
	'\n#define INFLASH_VERSION "20090628-1"' \
	'\n#endif' > inflash_version.h
BUILT_SOURCES = inflash_version.h

bin_PROGRAMS = inflash
inflash_SOURCES = cmd.c  command_line.c  ingenic_cfg.c  \
	ingenic_usb.c  main.c

prefix = /usr
datadir = /usr/share/inflash
data_DATA = ../xburst_stage1/xburst_stage1.bin \
	../xburst_stage2/xburst_stage2.bin \
	inflash.cfg
EXTRA_DIST = $(datadir)

../xburst_stage1/xburst_stage1.bin:
	$(MAKE) -C ../xburst_stage1

../xburst_stage2/xburst_stage2.bin:
	$(MAKE) -C ../xburst_stage2
