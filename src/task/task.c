#include <stdint.h>
#include <stdbool.h>

#include "task/task.h"
#include "mem/kmem.h"
#include "interrupts/idt.h"
#include "mem/gdt.h"
#include "task/asm.h"
#include "main/cpu.h"

task_process_t kernel_process;

static uint32_t thread_counter;
static uint32_t process_counter;
static uint32_t task_counter;

static task_thread_t *tasks;

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

static task_thread_t *_next_task(task_thread_t *task) {
    if(task->next_in_tasks) {
        return task->next_in_tasks;
    }else{
        return tasks;
    }
}


task_process_t *task_process_create(uint32_t owner, uint32_t group);


task_thread_t *task_thread_create(task_process_t *process, addr_logical_t entry) {
    task_thread_t *thread;
    bool kernel = process->process_id == 0;
    uint32_t *sp;
    idt_proc_state_t pstate = {0};
    
    thread = kmalloc(sizeof(task_thread_t));
    thread->next_in_process = process->thread;
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
    thread->stack_page = page_alloc(process->process_id, 0, 1);
    
    page_vm_map_new_table(TASK_STACK_TOP - PAGE_SIZE, thread->vm, NULL, NULL, PAGE_TABLE_RW);
    page_vm_map_insert(TASK_STACK_TOP - PAGE_SIZE, thread->vm, thread->stack_page, PAGE_TABLE_RW);
    
    // Initial stack format:
    // [pushad values]
    // entry eip
    sp = (uint32_t *)(TASK_STACK_TOP - sizeof(void *));
    *sp = entry;
    sp --;
    *sp = (addr_logical_t)task_asm_entry_point;
    sp -= (sizeof(pstate) / 4);
    _memcpy(sp - (sizeof(pstate) / 4), &pstate, sizeof(pstate));
    
    thread->stack_pointer = (addr_logical_t)sp;
    
    page_table_clear();
    
    thread->next_in_tasks = tasks;
    tasks = thread;
    
    return thread;
}

void task_enter(task_thread_t *thread) {
    cpu_status_t *info;
    
    info = cpu_info();
    info->thread = thread;
    
    // Load the task's memory map
    page_table_switch(thread->vm->physical_dir->mem_base);
    
    // And then hop into it
    task_asm_enter(thread->stack_pointer);
}

void task_yield() {
    cpu_status_t *info;
    info = cpu_info();
    
    // Call the exit function to move over the stack, will call task_yield_done
    task_asm_yield((uint32_t)info->stack + PAGE_SIZE - 4);
}

void task_yield_done(uint32_t sp) {
    cpu_status_t *info;
    task_thread_t *current;
    task_thread_t *next;
    info = cpu_info();
    
    info->thread->stack_pointer = sp;
    current = info->thread;
    info->thread = NULL;
    
    // And then use the "normal" memory map
    page_table_clear();
    
    next = _next_task(current);
    
    task_enter(next);
}
