[CPU 8086]

mov dx, 0x3F2
in al, dx

mov al, 0b1001010
mov dx, 0x3F5
out dx, al

mov dx, 0x3F4
in al, dx

hlt