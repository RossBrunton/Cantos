.globl task_asm_enter
task_asm_enter:
    mov %edx, %cr3;
    mov %ecx, %esp;
    popal;
    popf;
    ret;

.globl task_asm_entry_point
task_asm_entry_point:
    sti;
    ret;

.globl task_asm_yield
task_asm_yield:
    pushf;
    pushal;
    mov %esp, %eax;
    mov %ecx, %esp;
    push %eax;
    call task_yield_done;

.globl task_asm_set_stack
task_asm_set_stack:
    mov %ecx, %esp;
    push %edx;
    ret;
