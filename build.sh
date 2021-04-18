#!/bin/bash
nasm -g -f elf64 cgo.asm -o cgo_asm.o
#nasm -g -f elf64 queue_insert.asm -o q_insert.o
gcc -g -Wall -std=c99 -pthread -c main.c -o main.o
#gcc -g -pthread main.o cgo_asm.o q_insert.o -o cgo
gcc -g -pthread main.o cgo_asm.o -o cgo
