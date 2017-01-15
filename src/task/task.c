#include <stdint.h>
#include <stdbool.h>

#include "task/task.h"
#include "mem/kmem.h"
#include "interrupts/idt.h"
#include "mem/gdt.h"
#include "task/asm.h"
#include "main/cpu.h"
#include "mem/object.h"
#include "main/panic.h"

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


/** 
 * @todo Copy all the objects into the new memory map
 * @todo Get the stack object properly
 */
task_thread_t *task_thread_create(task_process_t *process, addr_logical_t entry) {
    task_thread_t *thread;
    bool kernel = process->process_id == 0;
    uint32_t *sp;
    idt_proc_state_t pstate = {0};
    void *stack_installed;
    
    thread = kmalloc(sizeof(task_thread_t), 0);
    thread->next_in_process = process->thread;
    process->thread = thread;
    thread->process = process;
    thread->thread_id = ++process->thread_counter;
    thread->task_id = ++task_counter;
    
    // Create the virtual memory map
    thread->vm = vm_map_alloc(process->process_id, thread->task_id, kernel);
    
    // Create the stack object
    thread->stack = object_alloc(object_gen_empty, object_del_free, 1, PAGE_TABLE_RW, OBJECT_FLAG_AUTOFREE);
    object_generate(thread->stack, 0, 1);
    object_add_to_vm(thread->stack, thread->vm, TASK_STACK_TOP - PAGE_SIZE);
    
    stack_installed = page_kinstall(thread->stack->pages->page, 0);
    
    // Initial stack format:
    // [pushad values]
    // entry eip
    sp = (uint32_t *)((addr_logical_t)stack_installed + PAGE_SIZE) - 1;
    *sp = entry;
    sp --;
    *sp = (addr_logical_t)task_asm_entry_point;
    sp -= (sizeof(pstate) / 4);
    _memcpy(sp - (sizeof(pstate) / 4), &pstate, sizeof(pstate));
    
    thread->stack_pointer = TASK_STACK_TOP - sizeof(void *) * 2 - sizeof(pstate);
    
    page_kuninstall(stack_installed, thread->stack->pages->page);
    
    thread->next_in_tasks = tasks;
    tasks = thread;
    
    return thread;
}


void task_thread_destroy(task_thread_t *thread) {
    task_thread_t *now;
    task_thread_t *prev = NULL;
    
    // Remove it from the process thread list
    for(now = thread->process->thread; now != thread; (prev = now), (now = now->next_in_process));
    if(prev) {
        prev->next_in_process = thread->next_in_process;
    }else{
        thread->process->thread = thread->next_in_process;
    }
    
    if(!now) {
        panic("Thread to destroy not found in it's process' threads.");
    }
    
    // And the main thread list
    prev = NULL;
    for(now = tasks; now != thread; (prev = now), (now = now->next_in_tasks));
    if(prev) {
        prev->next_in_tasks = thread->next_in_tasks;
    }else{
        tasks = thread->next_in_tasks;
    }
    
    if(!now) {
        panic("Thread to destroy not found in task lists.");
    }
    
    // And delete the memory map
    vm_map_free(thread->vm);
    
    kfree(thread);
}


void __attribute__((noreturn)) task_enter(task_thread_t *thread) {
    cpu_status_t *info;
    
    info = cpu_info();
    info->thread = thread;
    
    // Load the task's memory map
    vm_table_switch(thread->vm->physical_dir->mem_base);
    
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
    vm_table_clear();
    
    next = _next_task(current);
    
    task_enter(next);
}
