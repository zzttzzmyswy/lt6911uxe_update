#
# Copyright (c) 2015 - 2020 DisplayLink (UK) Ltd.
#

TARGET = lt6911uxe_update

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

GIT_COMMIT_HASH = $(shell git describe --tags)-$(shell git rev-parse HEAD)
GIT_COMMIT_DATE = $(shell git show -s --format=%ci HEAD)

INCLUDE = -I. -I./include
LIBS = -L. 

CFLAGS = -static -DGIT_COMMIT_HASH="\"$(GIT_COMMIT_HASH)\"" -DGIT_COMMIT_DATE="\"$(GIT_COMMIT_DATE)\"" $(INCLUDE) $(LIBS) -std=gnu99 

ifeq ($(ONLY_LT6911_I2C_TEST), 1)
	CFLAGS += -DONLY_LT6911_I2C_TEST=1
else
	CFLAGS += -DONLY_LT6911_I2C_TEST=0
endif

ifeq ($(DEBUG_MODE), 1)
	CFLAGS +=  -Wall -O0 -g
else
	CFLAGS +=  -Wall -O3
endif

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))
PREFIX = /usr/local/bin

all: clean $(TARGET)

ifeq ($(DEBUG_MODE), 1)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
else
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(STRIP) -v --strip-all $@*
	@if command -v upx >/dev/null 2>&1; then upx -9 --best $@*; fi
endif

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
