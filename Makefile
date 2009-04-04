#
# "PI Makefile" - for setting up the PI development environment
#
# (C) Copyright 2009 PI, Inc.
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

TOOLCHAIN_PATH=toolchain
DL_PATH=$(TOOLCHAIN_PATH)/dl
INSTALL_PATH=install
PATCHES_PATH=patches
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

CFLAGS="-O2"

export PATH:=$(PWD)/install/bin:$(PATH)


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

%.bz2: $(DL_PATH) $(INSTALL_PATH)
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

glibc: $(DL_PATH)/$(GLIBC_PACKAGE) $(DL_PATH)/$(GLIBC_PORTS_PACKAGE)
	[ -e $(TOOLCHAIN_PATH)/$(GLIBC_VER)/ports ] && rm -rf $(TOOLCHAIN_PATH)/$(GLIBC_VER)/ports
	tar -xvjf $(DL_PATH)/$(GLIBC_PACKAGE) -C $(TOOLCHAIN_PATH)
	tar -xvjf $(DL_PATH)/$(GLIBC_PORTS_PACKAGE) -C $(TOOLCHAIN_PATH)/$(GLIBC_VER)
	mv $(TOOLCHAIN_PATH)/$(GLIBC_VER)/$(GLIBC_PORTS_VER) $(TOOLCHAIN_PATH)/$(GLIBC_VER)/ports
	cd $(TOOLCHAIN_PATH)/$(GLIBC_VER) && \
	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-cross_hacks-1.patch && \
	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-libgcc_eh-1.patch && \
	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-localedef_segfault-1.patch && \
	patch -Np1 -i $(PWD)/$(GLIBC_PATCHES_PATH)/glibc-2.6.1-mawk_fix-1.patch
	mkdir -p $(TOOLCHAIN_PATH)/$(GLIBC_VER)/build
	cd $(TOOLCHAIN_PATH)/$(GLIBC_VER)/build && \
	echo "libc_cv_forced_unwind=yes" > config.cache && \
	echo "libc_cv_c_cleanup=yes" >> config.cache && \
	echo "libc_cv_mips_tls=yes" >> config.cache && \
	BUILD_CC="gcc" CC="mipsel-linux-gcc" \
	AR="mipsel-linux-ar" RANLIB="mipsel-linux-ranlib" \
	../configure --prefix=/usr --libexecdir=/usr/lib/glibc \
	--host=mipsel-linux --build=i686-pc-linux-gnu \
	--disable-profile --enable-add-ons --with-tls --enable-kernel=2.6.0 \
	--with-__thread --with-binutils=$(INSTALL_PATH)/bin \
	--cache-file=config.cache && make CFLAGS=$(CFLAGS) && make install
	touch $@


### clean up

distclean: clean clean-toolchain

clean:

clean-toolchain:
	rm -rf $(TOOLCHAIN_PATH)/$(BINUTILS_VER)
	rm -rf $(TOOLCHAIN_PATH)/$(GCC_VER)
	rm -rf $(TOOLCHAIN_PATH)/$(GLIBC_VER)
	rm -rf $(INSTALL_PATH) binutils gcc glibc

testhelp:
	make --print-data-base --question |	\
	awk '/^[^.%][-A-Za-z0-9_]*:/		\
	{ print substr($$1, 1, length($$1)-1) }' | 	\
	sort |	\
	pr --omit-pagination --width=80 --columns=1
