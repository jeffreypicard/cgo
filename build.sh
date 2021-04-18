#!/bin/bash
nasm -O0 -g -F dwarf -f elf64 cgo.asm -o cgo_asm.o
nasm -O0 -g -F dwarf -f elf64 queue_insert.asm -o q_insert.o
#nasm -g cgo.asm -o cgo_asm.o
#nasm -g queue_insert.asm -o q_insert.o
#/usr/local/bin/gcc-10  -pg -O0 -ggdb -Wall -std=c99 -pthread -c main.c -o main.o
#/usr/local/bin/gcc-10  -L/usr/local/lib -pg -O0 -ggdb -pthread main.o cgo_asm.o q_insert.o -o cgo
gcc  -O0 -g -Wall -std=c99 -pthread -c main.c -o main.o
gcc  -L/usr/local/lib -O0 -g -pthread main.o cgo_asm.o q_insert.o -o cgo
#gcc -g -pthread main.o cgo_asm.o -o cgo
