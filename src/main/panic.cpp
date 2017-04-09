#include <stdarg.h>

#include "main/panic.hpp"
#include "main/vga.hpp"
#include "main/cpu.hpp"

extern "C" {
    static uint8_t clr = vga::COLOUR_WHITE | (vga::COLOUR_RED << 4);

    void __attribute__((format(printf, 1, 2))) panic(const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);

        vga::string_stream.writef(0, &clr, "\nKERNEL PANIC: ");
        vga::string_stream.writef(0, &clr, fmt, va);

        cpu::Status *info = cpu::info();
        if(info->thread) {
            vga::string_stream.writef(0, &clr, "\n> Processor %d, in task %d", info->cpu_id, info->thread->task_id);
        }else{
            vga::string_stream.writef(0, &clr, "\n> Processor %d, in no thread", info->cpu_id);
        }

        __asm__ volatile ("cli");
        while(1) {
            __asm__ volatile ("hlt");
        }

        va_end(va);
    }
}
