#
# "Ingenic flash tool" - flash the Ingenic CPU via USB
#
# (C) Copyright 2009
# Author: Marek Lindner <lindner_marek@yahoo.de>
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



ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	Q_CC = @echo '   ' CC $@;
	Q_LD = @echo '   ' LD $@;
	export Q_CC
	export Q_LD
endif
endif

CC = 		gcc
CFLAGS +=	-pedantic -Wall -W -O1 -g3 -std=gnu99
LDFLAGS +=	-lusb -lconfuse
BINARY_NAME =	inflash

SRC_C= main.c usb.c cmd_boot.c
SRC_H= usb.h usb_boot_defines.h
SRC_O= $(SRC_C:.c=.o)

$(BINARY_NAME):	$(SRC_O) $(SRC_H) Makefile
	$(Q_LD)$(CC) -o $@ $(SRC_O) $(LDFLAGS)

.c.o:
	$(Q_CC)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MD -c $< -o $@
-include $(SRC_C:.c=.d)

clean:
	rm -f $(BINARY_NAME) *.o *~
	rm -f `find . -name '*.d' -print`
