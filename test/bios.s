[CPU 8086]

%define STARTUP_JUMP 0x1000

jmp init_start

int_0:
	call default_int
	iret
.end:

int_1:
	call default_int
	iret
.end:

int_2:
	call default_int
	iret
.end:

default_int:
	mov ax, 0xCAFE
	mov bx, 0xBABE
	ret
.end:

init_start:
mov ax, 0b10011
out 0x20, al
out 0x21, al
out 0x21, al

mov ax, 0xFFFF
mov bx, 0xFFFF
add ax, bx
mov ax, 0
mov ds, ax
mov ax, int_0
mov word [0], ax
mov ax, cs
mov word [2], ax

mov ax, int_1
mov word [4], ax
mov ax, cs
mov word [6], ax

mov ax, int_2
mov word [8], ax
mov ax, cs
mov word [10], ax
sti
jmp 0x100:0