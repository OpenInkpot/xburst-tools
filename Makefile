#
# "PI Makefile" - for setting up the PI development environment
#
# (C) Copyright 2009 PI.
# Author: xiangfu liu <xiangfu@openmoko.org>
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

BINUTILS_VER=binutils-2.17
GCC_VER=gcc-4.1.2
GLIBC_VER=glibc-2.6.1
GLIBC_PORTS_VER=glibc-ports-2.6.1
KERNEL_HEADERS_VER=linux-headers-2.6.24.3

TOOLCHAIN_PATH=toolchain
DL_PATH=$(TOOLCHAIN_PATH)/dl
INSTALL_PATH=install
PATCHES_PATH=$(TOOLCHAIN_PATH)/patches
GLIBC_PATCHES_PATH=$(PATCHES_PATH)/glibc

BINUTILS_PACKAGE=$(BINUTILS_VER).tar.bz2
BINUTILS_URL= \
  ftp://ftp.gnu.org/gnu/binutils/$(BINUTILS_PACKAGE)
GCC_PACKAGE=$(GCC_VER).tar.bz2
GCC_URL=ftp://ftp.gnu.org/gnu/gcc/$(GCC_VER)/$(GCC_PACKAGE)
GLIBC_PACKAGE=$(GLIBC_VER).tar.bz2
GLIBC_URL=ftp://ftp.gnu.org/gnu/glibc/$(GLIBC_PACKAGE)
GLIBC_PORTS_PACKAGE=$(GLIBC_PORTS_VER).tar.bz2
GLIBC_PORTS_URL=ftp://ftp.gnu.org/gnu/glibc/$(GLIBC_PORTS_PACKAGE)
KERNEL_HEADERS_PACKAGE=$(KERNEL_HEADERS_VER).tar.bz2
KERNEL_HEADERS_URL=

# for the device stage
FLASH_TOOL_PATH = ./flash-tool
FLASH_TOOL_BIN_PATH = $(FLASH_TOOL_PATH)/bin
STAGE1_PATH = $(FLASH_TOOL_PATH)/device_stage1
STAGE2_PATH = $(FLASH_TOOL_PATH)/device_stage2
CROSS_COMPILE ?= mipsel-linux-

CFLAGS="-O2"

export PATH:=$(PWD)/install/bin:$(PATH)

# this glibc version does not build with gcc 4.3 - hence this special compat parameter
GCC_VERSION=$(shell [ "`gcc --version | grep '4.3'`" ] && echo "43" )
ifeq ($(GCC_VERSION),43)
BUILD_CC="gcc -fgnu89-inline"
else
BUILD_CC="gcc"
endif

toolchain: binutils gcc glibc

### misc

$(DL_PATH):
	mkdir -p $(DL_PATH)

$(INSTALL_PATH):
	mkdir -p $(INSTALL_PATH)

%$(BINUTILS_PACKAGE): URL=$(BINUTILS_URL)
%$(GCC_PACKAGE): URL=$(GCC_URL)
%$(GLIBC_PACKAGE): URL=$(GLIBC_URL)
%$(GLIBC_PORTS_PACKAGE): URL=$(GLIBC_PORTS_URL)

.tar.bz2: $(DL_PATH) $(INSTALL_PATH)
	wget -c -O $@ $(URL)
	touch $@


### toolchain stuff
binutils: $(DL_PATH)/$(BINUTILS_PACKAGE)
	tar -xvjf $(DL_PATH)/$(BINUTILS_PACKAGE) -C $(TOOLCHAIN_PATH)
	mkdir -p $(TOOLCHAIN_PATH)/$(BINUTILS_VER)/build
	cd $(TOOLCHAIN_PATH)/$(BINUTILS_VER)/build && \
	../configure --target=mipsel-linux --prefix=$(PWD)/$(INSTALL_PATH) --disable-werror \
	&& make CFLAGS=$(CFLAGS) && make install
	touch $@

gcc: $(DL_PATH)/$(GCC_PACKAGE)
	tar -xvjf $(DL_PATH)/$(GCC_PACKAGE) -C $(TOOLCHAIN_PATH)
	cd $(TOOLCHAIN_PATH)/$(GCC_VER)/libiberty && \
	cat strsignal.c | sed -e 's/#ifndef HAVE_PSIGNAL/#if 0/g' >junk.c && \
	cp -f strsignal.c strsignal.c.fixed && mv -f junk.c strsignal.c
	mkdir -p $(TOOLCHAIN_PATH)/$(GCC_VER)/build
	cd $(TOOLCHAIN_PATH)/$(GCC_VER)/build && \
	../configure --target=mipsel-linux \
	--host=i686-pc-linux-gnu --prefix=$(PWD)/$(INSTALL_PATH) \
	--disable-shared --disable-threads --disable-multilib \
	--enable-languages=c && make CFLAGS=$(CFLAGS) all-gcc && make install-gcc
	touch $@

glibc: $(DL_PATH)/$(GLIBC_PACKAGE) $(DL_PATH)/$(GLIBC_PORTS_PACKAGE) $(DL_PATH)/$(KERNEL_HEADERS_PACKAGE)
	tar -xvjf $(DL_PATH)/$(KERNEL_HEADERS_PACKAGE) -C $(TOOLCHAIN_PATH)
	tar -xvjf $(DL_PATH)/$(GLIBC_PACKAGE) -C $(TOOLCHAIN_PATH)
	tar -xvjf $(DL_PATH)/$(GLIBC_PORTS_PACKAGE) -C $(TOOLCHAIN_PATH)/$(GLIBC_VER)
	mv $(TOOLCHAIN_PATH)/$(GLIBC_VER)/$(GLIBC_PORTS_VER) $(TOOLCHAIN_PATH)/$(GLIBC_VER)/ports
	cd $(TOOLCHAIN_PATH)/$(GLIBC_VER) && \
	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-cross_hacks-1.patch && \
	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-libgcc_eh-1.patch && \
	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-localedef_segfault-1.patch && \
	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-mawk_fix-1.patch
#	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-alpha_ioperm_fix-1.patch && \
#	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-RTLD_SINGLE_THREAD_P-1.patch && \
#	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-sysdep_cancel-1.patch && \
#	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-hppa_nptl-1.patch
	mkdir -p $(TOOLCHAIN_PATH)/$(GLIBC_VER)/build
	cd $(TOOLCHAIN_PATH)/$(GLIBC_VER)/build && \
	echo "libc_cv_forced_unwind=yes" > config.cache && \
	echo "libc_cv_c_cleanup=yes" >> config.cache && \
	echo "libc_cv_mips_tls=yes" >> config.cache && \
	BUILD_CC=$(BUILD_CC) CC="mipsel-linux-gcc" \
	AR="mipsel-linux-ar" RANLIB="mipsel-linux-ranlib" \
	../configure --prefix=$(PWD)/$(INSTALL_PATH) --libexecdir=$(PWD)/$(INSTALL_PATH)/libexec/glibc \
	--host=mipsel-linux --build=i686-pc-linux-gnu \
	--disable-profile --enable-add-ons --with-tls --enable-kernel=2.6.0 \
	--with-__thread --with-binutils=$(PWD)/$(INSTALL_PATH)/bin \
	--with-headers=$(PWD)/$(TOOLCHAIN_PATH)/$(KERNEL_HEADERS_VER) \
	--cache-file=config.cache && \
	make CFLAGS=$(CFLAGS) && make install
	cp $(GLIBC_PATCHES_PATH)/SUPPORTED $(TOOLCHAIN_PATH)/$(GLIBC_VER)/localedata/
	cd $(TOOLCHAIN_PATH)/$(GLIBC_VER)/build && \
	make localedata/install-locales
	touch $@

### u-boot
.PHONY: u-boot
u-boot:
	export PATH=/opt/mipseltools-gcc412-glibc261/bin:$PATH
	git clone git://github.com/xiangfu/pi-u-boot.git u-boot
	cd u-boot && \
	make pi_config && \
	make

### kernel
.PHONY: kernel
kernel:
	export PATH=/opt/mipseltools-gcc412-glibc261/bin:$PATH
	git clone git://github.com/xiangfu/pi-kernel.git kernel
	cd kernel && \
	make pi_defconfig && \
	make uImage

### flash-boot
.PHONY: flash-tool
flash-tool: stage1 stage2
	mkdir -p $(FLASH_TOOL_BIN_PATH)
	cp $(FLASH_TOOL_PATH)/src/usb_boot.cfg $(FLASH_TOOL_BIN_PATH)
	cd $(FLASH_TOOL_PATH) && \
	./autogen.sh && \
	./configure && \
	make
	cp $(FLASH_TOOL_PATH)/src/inflash $(FLASH_TOOL_BIN_PATH)

stage1:
	make CROSS_COMPILE=$(CROSS_COMPILE) -C $(STAGE1_PATH)

stage2:
	make CROSS_COMPILE=$(CROSS_COMPILE) -C $(STAGE2_PATH)

### clean up
distclean: clean clean-toolchain

clean:
	make clean -C flash-tool

clean-toolchain: clean-glibc
	rm -rf $(TOOLCHAIN_PATH)/$(BINUTILS_VER) binutils
	rm -rf $(TOOLCHAIN_PATH)/$(GCC_VER) gcc
	rm -rf $(INSTALL_PATH)

clean-glibc:
	rm -rf $(TOOLCHAIN_PATH)/$(GLIBC_VER) glibc

help:
	@make --print-data-base --question |	\
	awk '/^[^.%][-A-Za-z0-9_]*:/		\
	{ print substr($$1, 1, length($$1)-1) }' | 	\
	sort |	\
	pr --omit-pagination --width=80 --columns=1
