#ifndef _H_TASK_TASK_
#define _H_TASK_TASK_

#include <stddef.h>

#include "mem/page.h"

#define TASK_STACK_TOP KERNEL_VM_BASE

typedef struct task_process_s task_process_t;
typedef struct task_thread_s task_thread_t;

struct task_process_s {
    task_thread_t *thread;
    uint32_t process_id;
    uint32_t owner;
    uint32_t group;
    uint32_t thread_counter;
    
    task_process_t *next;
};

struct task_thread_s {
    task_process_t *process;
    uint32_t thread_id;
    uint32_t task_id;
    
    page_vm_map_t *vm;
    
    addr_logical_t stack_top;
    addr_logical_t stack_bottom;
    page_t *stack_page;
    addr_logical_t stack_pointer;
    
    task_thread_t *next_in_process;
    task_thread_t *next_in_tasks;
};


extern task_process_t kernel_process;

void task_init();
task_process_t *task_proc_create(uint32_t owner, uint32_t group);
task_thread_t *task_thread_create(task_process_t *process, addr_logical_t entry);
void task_enter(task_thread_t *thread);
void task_yield();
void task_yield_done(uint32_t sp);

#endif
