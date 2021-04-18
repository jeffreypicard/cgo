;
; aquire_blk.asm
;
; Assembly function to attempt to aquire a block.
; 
; Author: Jeffrey Picard
;

section .text
  global atomic_increment

atomic_increment:
  push  rbp                   ; Save old frame pointer
  push  rbx
  mov   rbp, rsp              ; Establish new frame pointer
;
  push  r12                   ; Save callee saved registers
  push  r13
  push  r14
;  push  r15
; 
  mov   r12, rdi              ; &size
  mov   r13, rsi              ; expected_size
  mov   r14, rdx	      ; new_size
;
  mov	rax, r13 ; owner should be NULL or it's owned and this fails
  lock  cmpxchg [r12], r14    ; owner = newowner
  jnz   fail
  mov   rax, 0
  mov   rdi, 0
  jmp   success
;
fail:
  mov   rax, 1
  mov   rdi, 1
success:
;
;  pop   r15                   ; Return callee saved registers
  pop   r14
  pop   r13
  pop   r12
;
  pop rbx
  pop rbp                     ; Re-establish old frame pointer
  ret                         ; Return
