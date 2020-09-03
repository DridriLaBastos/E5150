%include "test/util.inc"

[CPU 8086]
mov dx, 0x20
mov ax, 0b10011
out dx, al

mov dx, 0x21
mov ax, 0xB0
out dx, al

mov ax, 1
out dx, al

sti

mov dx, 0x3F2
mov ax, 0b10000
out dx, al

SENSE_DRIVE_STATUS(0b100)
hlt