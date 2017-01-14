#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "structures/stream.h"
#include "main/vga.h"
#include "mem/page.h"

uint8_t vga_entry_colour(vga_colour_t fg, vga_colour_t bg) {
    return fg | bg << 4;
}

uint16_t vga_add_colour(uint8_t uc, uint8_t color) {
    return (uint16_t) uc | ((uint16_t) color << 8);
}

static size_t terminal_row;
static size_t terminal_column;
static volatile uint16_t *terminal_buffer;

void vga_init() {
    terminal_row = 0;
    terminal_column = 0;
    page_t *page;
    
    page = page_create(0xB8000, PAGE_FLAG_KERNEL, (128 * 1024) / PAGE_SIZE);
    terminal_buffer = page_kinstall(page, PAGE_TABLE_CACHEDISABLE | PAGE_TABLE_RW);
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = ' ';
        }
    }
}

void vga_put_at(uint16_t c, size_t x, size_t y) {
    terminal_buffer[y * VGA_WIDTH + x] = c;
}

static void _maybe_wrap() {
    if(terminal_row >= VGA_HEIGHT) {
        for(uint16_t i = VGA_WIDTH; i < VGA_HEIGHT*VGA_WIDTH; i ++) {
            terminal_buffer[i - VGA_WIDTH] = terminal_buffer[i]; 
        }
        
        for(uint16_t i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i ++) {
            terminal_buffer[i] = 0x0000; 
        }
        
        terminal_row = VGA_HEIGHT - 1;
    }
}

static void _vga_append(uint16_t c) {
    if(!terminal_buffer) return;
    
    if((c & 0xff) == '\n') {
        terminal_row ++;
        terminal_column = 0;
    }else{
        vga_put_at(c, terminal_column, terminal_row);
        
        terminal_column ++;
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            terminal_row ++;
        }
    }
    
    _maybe_wrap();
}

static int _vga_stream_write(void *buff, size_t len, uint32_t flags, void *data, stream_t *stream) {
    (void)data;
    (void)flags;
    (void)stream;
    uint16_t *str = buff;
    for(size_t i = 0; i < len; i ++) {
        _vga_append(str[i]);
    }

    return len;
}

static int _vga_string_stream_write(void *buff, size_t len, uint32_t flags, void *data, stream_t *stream) {
    (void)flags;
    (void)stream;
    uint8_t *colour = data;
    uint8_t *str = buff;
    for(size_t i = 0; i < len; i ++) {
        _vga_append(vga_add_colour(str[i], *colour));
    }

    return len;
}

stream_t vga_stream = {
    .write = _vga_stream_write
};

stream_t vga_string_stream = {
    .write = _vga_string_stream_write
};
