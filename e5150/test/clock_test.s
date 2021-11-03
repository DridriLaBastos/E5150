[BITS  16]
[CPU 8086]

mov ax, ax
mov ax, bx
mov ax, cx
mov ax, dx

mov bx, ax
mov bx, bx
mov bx, cx
mov bx, dx

mov cx, ax
mov cx, bx
mov cx, cx
mov cx, dx

mov cx, ax
mov cx, bx
mov cx, cx
mov cx, dx

;;------------

mov ax, [0]
mov ax, [bx]
mov ax, [di]
mov ax, [si]

mov bx, [0]
mov bx, [bx]
mov bx, [di]
mov bx, [si]

mov cx, [0]
mov cx, [bx]
mov cx, [di]
mov cx, [si]

mov cx, [0]
mov cx, [bx]
mov cx, [di]
mov cx, [si]

cli
hlt
