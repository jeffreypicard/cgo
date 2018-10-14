#!/bin/bash
nasm -g -f elf64 cgo.asm -o cgo_asm.o
gcc -g -Wall -std=c99 -pthread -c main.c -o main.o
gcc -g -pthread main.o cgo_asm.o -o cgo
