#pragma once

#include "main/common.hpp"

namespace display {
class Display {
protected:
    int cols = 0;
    int rows = 0;
    volatile uint16_t** buffer = nullptr;

public:
    bool active = false;
    size_t id = 0;

public:
    Display(volatile uint16_t** buffer, size_t id);
    virtual void winch(int cols, int rows);
    virtual void enable();
    virtual void disable();
    virtual ~Display() {};
};

class TestDisplay : public Display {
public:
    TestDisplay(volatile uint16_t** buffer, size_t id);
    void enable() override;

private:
    void update();
};
}
