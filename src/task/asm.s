.globl task_asm_enter
task_asm_enter:
    mov %ecx, %esp;
    popal;
    ret;

.globl task_asm_entry_point
task_asm_entry_point:
    ret;

.globl task_asm_yield
task_asm_yield:
    pushal;
    mov %esp, %eax;
    mov %ecx, %esp;
    push %eax;
    call task_yield_done;
