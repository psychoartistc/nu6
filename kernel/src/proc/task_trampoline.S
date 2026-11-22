.pushsection .text
.globl task_trampoline
.type task_trampoline, @function

task_trampoline:
  call task_first_run
  movq %r13, %rdi
  call *%r12
  call task_exit
  ud2 // oh no!

.size task_trampoline, .-task_trampoline
.popsection
