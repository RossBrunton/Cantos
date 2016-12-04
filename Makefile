CROSS_PREFIX=i686-elf
CC=$(CROSS_PREFIX)-gcc
AS=$(CROSS_PREFIX)-as

CFLAGS=-std=c99 -ffreestanding -O2 -Wall -Wextra -c -Iinclude/
AFLAGS=
LDFLAGS=-T linker.ld -ffreestanding -O2 -nostdlib -lgcc -static-libgcc

OBJECTS=obj/main/boot.o\
		obj/main/main.o\
		obj/main/stream.o\
		obj/main/vga.o\
		obj/main/printk.o\
		obj/mem/page.o\
		obj/mem/kmem.o

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.s
	$(AS) $(AFLAGS) -o $@ $^

clean:
	-rm -r obj/*
	-rm -r bin/*
	-rm -r isodir/*

dirs:
	test -e obj/main || mkdir obj/main
	test -e obj/mem || mkdir obj/mem

all: dirs all_objects

all_objects: $(OBJECTS)
	$(CC) -o bin/cantos.bin $^ $(LDFLAGS)

grub: all
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub/
	cp bin/cantos.bin isodir/boot/cantos.bin
	grub-mkrescue -o cantos.iso isodir
