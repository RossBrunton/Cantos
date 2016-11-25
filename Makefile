CROSS_PREFIX=i686-elf
CC=$(CROSS_PREFIX)-gcc
AS=$(CROSS_PREFIX)-as

CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -c
AFLAGS=
LDFLAGS=-T linker.ld -ffreestanding -O2 -nostdlib -lgcc

OBJECTS=obj/main/boot.o\
        obj/main/main.o

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.s
	$(AS) $(AFLAGS) -o $@ $^

clean:
	rm -r obj/*
	rm -r bin/*

dirs:
	test -e obj/main || mkdir obj/main

all: dirs all_objects

all_objects: $(OBJECTS)
	$(CC) $(LDFLAGS) -o bin/cantos.bin $^
