#ifndef _HPP_MAIN_VGA_
#define _HPP_MAIN_VGA_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "display/display.hpp"
#include "structures/stream.hpp"

namespace vga {
/* Hardware text mode color constants. */
enum Colour {
    COLOUR_BLACK = 0,
    COLOUR_BLUE = 1,
    COLOUR_GREEN = 2,
    COLOUR_CYAN = 3,
    COLOUR_RED = 4,
    COLOUR_MAGENTA = 5,
    COLOUR_BROWN = 6,
    COLOUR_LIGHT_GREY = 7,
    COLOUR_DARK_GREY = 8,
    COLOUR_LIGHT_BLUE = 9,
    COLOUR_LIGHT_GREEN = 10,
    COLOUR_LIGHT_CYAN = 11,
    COLOUR_LIGHT_RED = 12,
    COLOUR_LIGHT_MAGENTA = 13,
    COLOUR_LIGHT_BROWN = 14,
    COLOUR_WHITE = 15,
};

const int WIDTH = 80;
const int HEIGHT = 25;

uint8_t entry_colour(Colour fg, Colour bg);
uint16_t add_colour(uint8_t uc, uint8_t color);

/** @private */
extern vector<unique_ptr<display::Display>> displays;
/** @private */
extern mutex::Mutex displayMutex;
/** @private */
extern volatile uint16_t* terminal_buffer;
template <class T>
display::Display& addDisplay() {
    displayMutex.lock();
    displays.push_back(make_unique<T>(&terminal_buffer, displays.size()));

    display::Display& toRet = *displays.back();
    displayMutex.unlock();
    return toRet;
}
display::Display& getDisplay(int id);
void switchDisplay(int id);

void init();

class EntryStream : public stream::Stream {
public:
    error_t write(const void* buff, size_t len, uint32_t flags, void* data, uint32_t* written);
};

class StringStream : public stream::Stream {
public:
    error_t write(const void* buff, size_t len, uint32_t flags, void* data, uint32_t* written);
};

extern EntryStream entry_stream;
extern StringStream string_stream;
}

#endif
