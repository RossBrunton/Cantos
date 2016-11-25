#ifndef _H_MAIN_VGA_
#define _H_MAIN_VGA_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/stream.h"

/* Hardware text mode color constants. */
typedef enum vga_colour_e {
    VGA_COLOUR_BLACK = 0,
    VGA_COLOUR_BLUE = 1,
    VGA_COLOUR_GREEN = 2,
    VGA_COLOUR_CYAN = 3,
    VGA_COLOUR_RED = 4,
    VGA_COLOUR_MAGENTA = 5,
    VGA_COLOUR_BROWN = 6,
    VGA_COLOUR_LIGHT_GREY = 7,
    VGA_COLOUR_DARK_GREY = 8,
    VGA_COLOUR_LIGHT_BLUE = 9,
    VGA_COLOUR_LIGHT_GREEN = 10,
    VGA_COLOUR_LIGHT_CYAN = 11,
    VGA_COLOUR_LIGHT_RED = 12,
    VGA_COLOUR_LIGHT_MAGENTA = 13,
    VGA_COLOUR_LIGHT_BROWN = 14,
    VGA_COLOUR_WHITE = 15,
} vga_colour_t;
 
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

uint8_t vga_entry_colour(vga_colour_t fg, vga_colour_t bg);
uint16_t vga_add_colour(uint8_t uc, uint8_t color);

void vga_init();
void vga_put_at(uint16_t c, size_t x, size_t y);

stream_t vga_stream;
stream_t vga_string_stream;

#endif
