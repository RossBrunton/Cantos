.globl task_asm_enter
task_asm_enter:
    mov %ecx, %esp;
    popal;
    ret;

.globl task_asm_entry_point
task_asm_entry_point:
    ret;
