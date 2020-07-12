[CPU 8086]

mov dx, 0x3F4
in al, dx

mov ax, 0b1001010
mov dx, 0x3F5
out dx, al

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
mov ax, 0
out dx, al

times 10 nop

mov dx, 0x3F5
in al, dx

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
in al, dx
in al, dx
out dx, al

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
out dx, al
in al, dx

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
out dx, al

mov dx, 0x3F4
in al, dx

;à partir de là c'est bon
mov dx, 0x3F5
in al, dx

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
in al, dx

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
in al, dx

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
in al, dx

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
in al, dx

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
in al, dx

mov dx, 0x3F4
in al, dx

mov dx, 0x3F5
in al, dx

hlt