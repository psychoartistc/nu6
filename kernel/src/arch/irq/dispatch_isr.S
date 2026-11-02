.altmacro
.section .text

.extern irq_dispatch

/* vectors that push their error code on the stack */
.macro isr_err_stub num
.global isr_stub_\num
isr_stub_\num:
	push $\num
	jmp isr_common_stub
.endm

.macro isr_noerr_stub num
.global isr_stub_\num
isr_stub_\num:
	push $0
	push $\num
	jmp isr_common_stub
.endm

/* cpu exceptions */
.set i, 0
.rept 32
	.if i == 8 || i == 10 || i == 11 || i == 12 || i == 13 || i == 14 || i == 17 || i == 21 || i == 29 || i == 30
		isr_err_stub %i
	.else
		isr_noerr_stub %i
	.endif
	.set i, i+1
.endr

/* 32 to 255 software irqs */
.set i, 32
.rept 224
	isr_noerr_stub %i
	.set i, i+1
.endr

isr_common_stub:
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    cld

    mov %rsp, %rdi
    sub $8, %rsp
    call irq_dispatch
    add $8, %rsp

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    add $16, %rsp
    iretq

.section .rodata
.global g_isrStubTable

.macro isr_addr num
	.quad isr_stub_\num
.endm

g_isrStubTable:
.set i, 0
.rept 256
	isr_addr %i
	.set i, i+1
.endr

