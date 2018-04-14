#include "display/display.hpp"

#include "main/vga.hpp"

namespace display {
Display::Display(volatile uint16_t** buffer, size_t id) : buffer(buffer), id(id) {}

void Display::winch(int cols, int rows) {
    this->cols = cols;
    this->rows = rows;
}

void Display::enable() { active = true; }
void Display::disable() { active = false; }

TestDisplay::TestDisplay(volatile uint16_t** buffer, size_t id) : Display(buffer, id) {}
void TestDisplay::enable() {
    Display::enable();
    update();
}

void TestDisplay::update() {
    if (!active)
        return;
    for (int y = 0; y < rows; y++) {
        int white = y % vga::COLOUR_WHITE;
        for (int x = 0; x < cols; x++) {
            (*buffer)[y * cols + x] = vga::add_colour(
                '\0', vga::entry_colour(vga::COLOUR_WHITE, vga::Colour(white % vga::COLOUR_WHITE + 1)));
            white ++;
        }
    }
}
}
