.global stack_enter
.type stack_enter, @function

stack_enter:
    # rdi = stack_top
    # rsi = entry
    # rdx = arg
    mov %rdi, %rsp
    and $-16, %rsp

    mov %rdx, %rdi
    call *%rsi

1:
    cli
    hlt
    jmp 1b
