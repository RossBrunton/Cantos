CROSS_PREFIX=i686-elf
CC=$(CROSS_PREFIX)-gcc
AS=$(CROSS_PREFIX)-as

DEBUGFLAGS=-DDEBUG_MEM -DDEBUG_SERIAL
CFLAGS=-g -std=c99 -ffreestanding -O2 -pedantic -Wall -Wextra -c -Iinclude/ $(DEBUGFLAGS)
AFLAGS=-g
LDFLAGS=-g -T linker.ld -ffreestanding -O2 -pedantic -nostdlib -lgcc -static-libgcc

OBJECTS=obj/main/boot.o\
		obj/main/main.o\
		obj/structures/stream.o\
		obj/main/vga.o\
		obj/main/printk.o\
		obj/mem/page.o\
		obj/mem/kmem.o\
		obj/main/lomain.o\
		obj/main/multiboot.o\
		obj/main/panic.o\
		obj/mem/gdt.o\
		obj/interrupts/idt.o\
		obj/interrupts/wrapper.o\
		obj/interrupts/exceptions.o\
		obj/io/utils.o\
		obj/io/pic.o\
		obj/io/serial.o\
		obj/task/task.o\
		obj/task/asm.o\
		obj/main/cpu.o\
		obj/mem/vm.o\
		obj/mem/object.o

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
	test -e obj/interrupts || mkdir obj/interrupts
	test -e obj/io || mkdir obj/io
	test -e obj/task || mkdir obj/task
	test -e obj/structures || mkdir obj/structures

all: dirs all_objects

all_objects: $(OBJECTS)
	$(CC) -o bin/cantos.bin $^ $(LDFLAGS)

grub: all
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub/
	cp bin/cantos.bin isodir/boot/cantos.bin
	grub-mkrescue -o cantos.iso isodir
