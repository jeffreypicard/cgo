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
  sub     rsp,32           ;Allocate space for local variables (8 bytes each)
  mov     [rbp-24],rbx     ;We're responsible for restoring this value
  mov     r10,rdi          ;move the arg pointer becasuse rdi is needed
  mov     rbx,[r10]        ;How many params
  mov     r11,[r10+8]      ;Function addr
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
  mov     rdi,[r10+16]
  jmp     next
two:
  mov     rsi,[r10+24]
  jmp     next
three:
  mov     rdx,[r10+32]
  jmp     next
four:
  mov     rcx,[r10+40]
  jmp     next
five:
  mov     r8,[r10+48]
  jmp     next
six:
  mov     r9,[r10+56]
;more: FIXME: Implement more than 6 args

next:
  add     rax,1
  jmp     loop

go:

  call    r11
  mov     rbx,[rbp-24]
  add     rsp,32
  pop     rbp
  ret
