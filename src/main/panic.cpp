#include <stdarg.h>

#include "main/panic.hpp"
#include "main/vga.hpp"
#include "main/cpu.hpp"
#include "debug/stack.hpp"
#include "structures/elf.hpp"

extern "C" {
    static uint8_t clr = vga::COLOUR_WHITE | (vga::COLOUR_RED << 4);
    static uint8_t panicked = 0;

    void __attribute__((format(printf, 1, 2))) panic(const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);
        uint32_t ebp;
        cpu::Status *info;
        asm volatile("mov %%ebp, %0" : "=r" (ebp));
        char *name;

        stack::Unwinder unwinder(ebp);

        switch(panicked ++) {
            case 0:
                vga::string_stream.writef(0, &clr, "\nKERNEL PANIC: ");
                vga::string_stream.writef(0, &clr, fmt, va);
                va_end(va);

                info = cpu::info();
                if(info->thread) {
                    vga::string_stream.writef(0, &clr, " [%d/%d]", info->cpu_id, info->thread->task_id);
                }else{
                    vga::string_stream.writef(0, &clr, " [%d/-]", info->cpu_id);
                }

                // Unwind the stack
                if(ebp) {
                    do {
                        name = unwinder.methodName(elf::kernel_elf);
                        if(name) {
                            vga::string_stream.writef(0, &clr, "\nin %s", name, 0);
                        }else{
                            vga::string_stream.writef(0, &clr, "\nin [unknown frame]");
                        }
                    } while(unwinder.unwind());
                }
                break;

            case 1:
                vga::string_stream.writef(0, &clr, "\n! Panic Handling Panic");
                va_end(va);
                break;

            default:
                // Pass
                va_end(va);
                break;
        }

        __asm__ volatile ("cli");
        while(1) {
            __asm__ volatile ("hlt");
        }
    }
}
