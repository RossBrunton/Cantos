#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/vga.hpp"
#include "mem/page.hpp"
#include "structures/stream.hpp"

extern "C" {
#include "main/errno.h"
}

namespace vga {
uint8_t entry_colour(Colour fg, Colour bg) {
    return fg | bg << 4;
}

uint16_t add_colour(uint8_t uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

static size_t terminal_row;
static size_t terminal_column;
volatile uint16_t* terminal_buffer;

void init() {
    terminal_row = 0;
    terminal_column = 0;
    page::Page* page;

    page = page::create(0xB8000, page::FLAG_KERNEL, (128 * 1024) / PAGE_SIZE);
    terminal_buffer = (volatile uint16_t*)page::kinstall(page, page::PAGE_TABLE_CACHEDISABLE | page::PAGE_TABLE_RW);

    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            terminal_buffer[y * WIDTH + x] = ' ';
        }
    }
}

vector<unique_ptr<display::Display>> displays;
mutex::Mutex displayMutex;
int activeDisplay = -1;

display::Display& getDisplay(int id) {
    return *displays[id];
}

void switchDisplay(int id) {
    if (activeDisplay >= 0) {
        getDisplay(activeDisplay).disable();
    }
    getDisplay(id).winch(WIDTH, HEIGHT);
    getDisplay(id).enable();
}

static void put_at(uint16_t c, size_t x, size_t y) {
    terminal_buffer[y * WIDTH + x] = c;
}

static void _maybe_wrap(uint16_t colour) {
    colour = colour & 0xff00;
    if (terminal_row >= HEIGHT) {
        for (uint16_t i = WIDTH; i < HEIGHT * WIDTH; i++) {
            terminal_buffer[i - WIDTH] = terminal_buffer[i];
        }

        for (uint16_t i = (HEIGHT - 1) * WIDTH; i < HEIGHT * WIDTH; i++) {
            terminal_buffer[i] = colour;
        }

        terminal_row = HEIGHT - 1;
    }
}

static void _append(uint16_t c) {
    if (!terminal_buffer)
        return;

    if ((c & 0xff) == '\n') {
        terminal_row++;
        terminal_column = 0;
    } else {
        put_at(c, terminal_column, terminal_row);

        terminal_column++;
        if (terminal_column >= WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }

    _maybe_wrap(c);
}

error_t EntryStream::write(const void* buff, size_t len, uint32_t flags, void* data, uint32_t* written) {
    uint16_t* str = (uint16_t*)buff;
    for (size_t i = 0; i < len; i++) {
        _append(str[i]);
    }

    *written = len;
    return EOK;
}

error_t StringStream::write(const void* buff, size_t len, uint32_t flags, void* data, uint32_t* written) {
    uint8_t* colour = (uint8_t*)data;
    uint8_t* str = (uint8_t*)buff;
    for (size_t i = 0; i < len; i++) {
        _append(add_colour(str[i], *colour));
    }

    *written = len;
    return EOK;
}

StringStream string_stream;
EntryStream entry_stream;
}
