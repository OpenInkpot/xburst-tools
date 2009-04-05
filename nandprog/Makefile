#
# Makefile
#

CROSS	= mipsel-linux-
CC	= $(CROSS)gcc
LD 	= $(CROSS)ld

ROOTFIR = .
JZ4730DIR = $(ROOTFIR)/jz4730
JZ4740DIR = $(ROOTFIR)/jz4740
INCDIR  = $(ROOTFIR)/include
COMDIR  = $(ROOTFIR)/common

SOURCES	+= $(wildcard $(JZ4730DIR)/*.c)
SOURCES	+= $(wildcard $(JZ4740DIR)/*.c)
SOURCES	+= $(wildcard $(COMDIR)/*.c)
HEADS   :=$(wildcard $(INCDIR)/*.h)
OBJS	:= $(addsuffix .o , $(basename $(notdir $(SOURCES))))

CFLAGS := -I$(INCDIR) -O2 -Wall 
VPATH	:= $(JZ4730DIR) $(COMDIR) $(JZ4740DIR)
TARGETS = nandprog

all: $(TARGETS)

$(TARGETS) : $(OBJS) $(HEADS)
	$(CC) $(CFLAGS) -o $(TARGETS) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

install:

clean:
	rm -f *.o $(TARGETS)

