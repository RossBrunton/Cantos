#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/stream.h"
#include "main/vga.h"
#include "main/multiboot.h"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "Wrong architecture!"
#endif

#define MSG "Hello world!"

void kernel_main(multiboot_info_t *mbi, unsigned int magic) {
    (void) magic;
    unsigned int i;
    mm_entry_t *entry;
    
    vga_init();
    
    uint8_t clr = vga_entry_colour(VGA_COLOUR_WHITE, VGA_COLOUR_BLACK);
    stream_writef(&vga_string_stream, 0, &clr, "Cantos\n", mbi->boot_loader_name);
    stream_writef(&vga_string_stream, 0, &clr, "Booted by %s [Flags: %x, Command: %s]\n",
        mbi->boot_loader_name, mbi->flags, mbi->cmdline);
    stream_writef(&vga_string_stream, 0, &clr, "Main function is located at %p\n", kernel_main);
    stream_writef(&vga_string_stream, 0, &clr, "MMap Entries:\n");
    
    entry = (void*)mbi->mmap_addr;
    for(i = 0; (uint32_t)((void *)entry - mbi->mmap_addr) < mbi->mmap_length; i ++) {
        stream_writef(&vga_string_stream, 0, &clr, "> [%p:%d] Entry %d: 0x%llx-0x%llx @ %d\n",
            entry, entry->size, i, entry->base, entry->base + entry->length, entry->type);
        entry = (mm_entry_t *)(((void *)entry) + entry->size + 4);
    }
}
