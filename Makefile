#
# Copyright (c) 2015 - 2020 DisplayLink (UK) Ltd.
#

TARGET = lt6911_update

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

GIT_COMMIT_HASH = $(shell git describe --tags)-$(shell git rev-parse HEAD)
GIT_COMMIT_DATE = $(shell git show -s --format=%ci HEAD)

INCLUDE = -I. -I$(PWD)/include
LIBS = -L. 

CFLAGS = -DGIT_COMMIT_HASH="\"$(GIT_COMMIT_HASH)\"" -DGIT_COMMIT_DATE="\"$(GIT_COMMIT_DATE)\"" -std=gnu99 -Wall -O0 -g 

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
PREFIX = /usr/local/bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
# $(STRIP) -v --strip-debug --strip-unneeded $@*

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
