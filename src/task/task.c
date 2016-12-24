#include <stdint.h>
#include <stdbool.h>

#include "task/task.h"
#include "mem/kmem.h"
#include "interrupts/idt.h"
#include "mem/gdt.h"
#include "task/asm.h"

task_process_t kernel_process;

static uint32_t thread_counter;
static uint32_t process_counter;
static uint32_t task_counter;


static void *_memcpy(void *destination, const void *source, size_t num) {
    size_t i;
    for(i = 0; i < num; i ++) {
        ((char *)destination)[i] = ((char *)source)[i];
    }
    return destination;
}


void task_init() {
    kernel_process.process_id = 0;
    // All other fields 0 by default
}


task_process_t *task_process_create(uint32_t owner, uint32_t group);


task_thread_t *task_thread_create(task_process_t *process, addr_logical_t entry) {
    task_thread_t *thread;
    bool kernel = process->process_id == 0;
    uint32_t *sp;
    idt_proc_state_t pstate = {0};
    
    thread = kmalloc(sizeof(task_thread_t));
    thread->next = process->thread;
    process->thread = thread;
    thread->process = process;
    thread->thread_id = ++process->thread_counter;
    thread->task_id = ++task_counter;
    
    // Create the virtual memory map
    thread->vm = page_alloc_vm_map(process->process_id, thread->task_id, kernel);
    
    // Temporarily use the vm to set up values
    page_table_switch(thread->vm->physical_dir->mem_base);
    
    // Create the stack
    thread->stack_top = TASK_STACK_TOP;
    thread->stack_bottom = TASK_STACK_TOP - PAGE_SIZE;
    thread->stack_page = page_alloc(process->process_id, PAGE_FLAG_AUTOKMALLOC, 1);
    
    page_vm_map_new_table(TASK_STACK_TOP - PAGE_SIZE, thread->vm, NULL, NULL, PAGE_TABLE_RW);
    page_vm_map_insert(TASK_STACK_TOP - PAGE_SIZE, thread->vm, thread->stack_page, PAGE_TABLE_RW);
    
    // Initial stack format:
    // [pushad values]
    // entry eip
    sp = (uint32_t *)(TASK_STACK_TOP - sizeof(void *));
    *sp = entry;
    sp --;
    *sp = task_asm_entry_point;
    sp -= (sizeof(pstate) / 4);
    _memcpy(sp - (sizeof(pstate) / 4), &pstate, sizeof(pstate));
    
    thread->stack_pointer = sp;
    
    page_table_clear();
    
    return thread;
}

void task_enter(task_thread_t *thread) {
    // Load the task's memory map
    page_table_switch(thread->vm->physical_dir->mem_base);
    
    // And then hop into it
    task_asm_enter(thread->stack_pointer);
}
