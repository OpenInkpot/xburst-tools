#!/bin/sh
# Generate debian/xburst_stage1.bin and debian/xburst_stage2.bin
#
# Requires a mipsel-openwrt-linux- toolchain on the $PATH.

set -e

debian/rules firmware
cp -f xburst_stage1/xburst_stage1.bin debian/
cp -f xburst_stage2/xburst_stage2.bin debian/
