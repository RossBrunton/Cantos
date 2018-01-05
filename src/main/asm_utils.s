.globl push_cli
push_cli:
    pushf;
    cli;
    pop %eax;
    ret;

.globl pop_flags
pop_flags:
    push %ecx;
    popf;
    ret;
