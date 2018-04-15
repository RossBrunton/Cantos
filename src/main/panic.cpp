#include <stdarg.h>
#include <stdint.h>

#include "main/panic.hpp"
#include "main/vga.hpp"
#include "main/cpu.hpp"
#include "debug/stack.hpp"
#include "structures/elf.hpp"
#include "int/numbers.h"

extern "C" {
    static uint8_t clr = vga::COLOUR_WHITE | (vga::COLOUR_RED << 4);
    static uint8_t panicked = 0;

    void __attribute__((format(printf, 1, 2))) panic(const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);

        uint32_t ebp;
        asm volatile("mov %%ebp, %0" : "=r" (ebp));

        vpanic_at(ebp, 0, fmt, va);
        va_end(va);
    }

    void __attribute__((format(printf, 3, 4))) panic_at(uint32_t ebp, uint32_t eip, const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);

        vpanic_at(ebp, eip, fmt, va);
        va_end(va);
    }

    void vpanic_at(uint32_t ebp, uint32_t eip, const char *fmt, va_list ap) {
        char *name;
        __asm__ volatile ("cli");

        stack::Unwinder unwinder(ebp);

        switch(panicked ++) {
            case 0: {
                vga::string_stream.writef(0, &clr, "\nKERNEL PANIC: ");
                vga::string_stream.writef(0, &clr, fmt, ap);
                va_end(ap);

                cpu::Status& info = cpu::info();
                if(info.thread) {
                    vga::string_stream.writef(0, &clr, " [%d/%d]", info.cpu_id, info.thread->task_id);
                }else{
                    vga::string_stream.writef(0, &clr, " [%d/-]", info.cpu_id);
                }

                // Unwind the stack
                if(elf::kernel_elf) {
                    if(eip) {
                        char * symname = elf::kernel_elf->
                            runtimeFindSymbolName(eip, elf::st_info(elf::STB_GLOBAL, elf::STT_FUNC));
                        if(symname) {
                            vga::string_stream.writef(0, &clr, "\nin %s", symname, 0);
                        }else{
                            vga::string_stream.writef(0, &clr, "\nin [unknown frame]");
                        }
                    }

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
                }else{
                    vga::string_stream.writef(0, &clr, "\n(no elf headers found, no debug info available)");
                }
                break;
            }

            case 1:
                vga::string_stream.writef(0, &clr, "\n! Panic Handling Panic");
                va_end(ap);
                break;

            default:
                // Pass
                va_end(ap);
                break;
        }

        // Tell other processors to stop
        uint32_t id = cpu::id();
        for(uint32_t i = 0; i < acpi::proc_count; i ++) {
            if(i != id) {
                lapic::ipi(INT_LAPIC_BASE + INT_LAPIC_PANIC, i);
            }
        }

        while(1) {
            __asm__ volatile ("hlt");
        }
    }
}
