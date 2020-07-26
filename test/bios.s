[CPU 8086]

mov ax, 0b10000
mov dx, 0x3F2
out dx, al

mov ax, 0b100001
out dx, al

mov ax, 0b100010
out dx, al

hlt