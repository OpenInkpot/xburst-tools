AM_CFLAGS = -pedantic -Wall -W -O1 -g3 -std=gnu99 -lusb -lconfuse -DCFGDIR=\"$(cfgdir)\"

xburst-tools_version.h:
	echo -e '#ifndef XBURST_TOOLS_VERSION' \
	'\n#define XBURST_TOOLS_VERSION "20090630-1"' \
	'\n#endif' > xburst-tools_version.h
BUILT_SOURCES = xburst-tools_version.h

bin_PROGRAMS = usbboot
usbboot_SOURCES = cmd.c  command_line.c  ingenic_cfg.c  \
	ingenic_usb.c  main.c

pkgdatadir = $(datadir)/xburst-tools
pkgdata_DATA = ../xburst_stage1/xburst_stage1.bin \
	../xburst_stage2/xburst_stage2.bin

cfgdir = $(sysconfdir)/xburst-tools
cfg_DATA = ../doc/usbboot.cfg

EXTRA_DIST = $(pkgdatadir) $(cfgdir)

../xburst_stage1/xburst_stage1.bin:
	$(MAKE) CROSS_COMPILE=mipsel-openwrt-linux- -C ../xburst_stage1

../xburst_stage2/xburst_stage2.bin:
	$(MAKE) CROSS_COMPILE=mipsel-openwrt-linux- -C ../xburst_stage2
