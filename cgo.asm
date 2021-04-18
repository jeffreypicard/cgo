;
; cgo.asm
;
; Meshes pthread to the callers function for cgo
; 
; Author: Jeffrey Picard
;

section .text
  global cgo_asm

cgo_asm:
  push    rbp
  mov     rbp,rsp
  sub     rsp,48           ;Allocate space for local variables (8 bytes each)
  mov     [rbp-24],rbx     ;We're responsible for restoring this value
  mov     [rbp-32],r12     ;We're responsible for restoring this value
  mov     [rbp-40],r13     ;We're responsible for restoring this value
  mov     r12,rdi          ;move the arg pointer becasuse rdi is needed
  mov     rbx,[r12]        ;How many params
  mov     r13,[r12+8]      ;Function addr
  xor     rax,rax

loop:
  cmp     rax,rbx
  je      go

  cmp     rax,0
  je      one

  cmp     rax,1
  je      two

  cmp     rax,2
  je      three

  cmp     rax,3
  je      four

  cmp     rax,4
  je      five

  cmp     rax,5
  je      six

one:
  mov     rdi,[r12+16]
  jmp     next
two:
  mov     rsi,[r12+24]
  jmp     next
three:
  mov     rdx,[r12+32]
  jmp     next
four:
  mov     rcx,[r12+40]
  jmp     next
five:
  mov     r8,[r12+48]
  jmp     next
six:
  mov     r9,[r12+56]
;more: FIXME: Implement more than 6 args

next:
  add     rax,1
  jmp     loop

go:

  call    r13
  mov     rbx,[rbp-24]
  mov     r12,[rbp-32]
  mov     r13,[rbp-42]
  add     rsp,48
  xor     rax,rax
  pop     rbp
  ret
