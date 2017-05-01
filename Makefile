UNAME := $(shell uname)
CC=gcc
CFLAGS=-std=c99 -Wall -O3
ifeq ($(UNAME), Darwin)
	CFLAGS += -I osxinclude
endif

zpusim: opcode.h vm.h vm.c cmdline.h cmdline.c zpusim.c
	$(CC) -o $@ vm.c cmdline.c zpusim.c $(CFLAGS)

cmdline.c cmdline.h: args.ggo
	gengetopt -i args.ggo

.PHONY: clean
clean:
	rm -f *.o cmdline.c cmdline.h simulator zpusim
