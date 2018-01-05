CROSS_PREFIX=i686-elf
CC=$(CROSS_PREFIX)-gcc
CPPC=$(CROSS_PREFIX)-gcc
AS=$(CROSS_PREFIX)-gcc

OPTFLAGS=-fno-omit-frame-pointer -Wno-format -Wno-unused-parameter
COMMON_FLAGS=-ffreestanding -O2 -pedantic -Wall -Wextra -c -Iinclude/ $(OPTFLAGS)
CFLAGS=-g -std=c99 $(COMMON_FLAGS)
CPPFLAGS=-g -std=c++14 $(COMMON_FLAGS) -fno-exceptions -fno-rtti
AFLAGS=-c -g -Iinclude/
LDFLAGS=-g -T linker.ld -ffreestanding -O2 -pedantic -nostdlib -lgcc -static-libgcc

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

OBJECTS=obj/debug/stack.o\
	obj/fs/filesystem.o\
	obj/hw/acpi.o\
	obj/hw/loacpi.o\
	obj/hw/pci.o\
	obj/hw/pit.o\
	obj/hw/ps2.o\
	obj/hw/ps2keyboard.o\
	obj/hw/serial.o\
	obj/hw/utils.o\
	obj/int/exceptions.o\
	obj/int/idt.o\
	obj/int/ioapic.o\
	obj/int/lapic.o\
	obj/int/pic.o\
	obj/int/wrapper.o\
	obj/main/asm_utils.o\
	obj/main/ap.o\
	obj/main/boot.o\
	obj/main/cpp.o\
	obj/main/cpu.o\
	obj/main/lomain.o\
	obj/main/main.o\
	obj/main/multiboot.o\
	obj/main/panic.o\
	obj/main/printk.o\
	obj/main/vga.o\
	obj/mem/gdt.o\
	obj/mem/kmem.o\
	obj/mem/object.o\
	obj/mem/page.o\
	obj/mem/vm.o\
	obj/structures/elf.o\
	obj/structures/list.o\
	obj/structures/mutex.o\
	obj/structures/static_list.o\
	obj/structures/shared_ptr.o\
	obj/structures/stream.o\
	obj/structures/unique_ptr.o\
	obj/structures/utf8.o\
	obj/task/asm.o\
	obj/task/task.o\
	obj/test/test.o

obj/%.o: src/%.cpp include/config.hpp
	$(CPPC) $(CPPFLAGS) -o $@ -imacros include/config.hpp $<

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.s
	$(AS) $(AFLAGS) -o $@ $^

obj/%.o: src/%.S
	$(AS) $(AFLAGS) -o $@ $^

clean:
	-rm -r obj/*
	-rm -r bin/*
	-rm -r isodir/*

dirs:
	test -e obj/main || mkdir obj/main
	test -e obj/mem || mkdir obj/mem
	test -e obj/int || mkdir obj/int
	test -e obj/hw || mkdir obj/hw
	test -e obj/task || mkdir obj/task
	test -e obj/structures || mkdir obj/structures
	test -e obj/debug || mkdir obj/debug
	test -e obj/fs || mkdir obj/fs
	test -e obj/test || mkdir obj/test

all: dirs obj/main/crti.o obj/main/crtn.o all_objects

all_objects: $(OBJECTS)
	$(CC) -o bin/cantos.bin obj/main/crti.o $(CRTBEGIN_OBJ) $^ $(CRTEND_OBJ) obj/main/crtn.o $(LDFLAGS)

grub: all
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub/
	cp bin/cantos.bin isodir/boot/cantos.bin
	grub-mkrescue -o cantos.iso isodir

docs:
	-rm -r doc/*
	doxygen include/Doxyfile
