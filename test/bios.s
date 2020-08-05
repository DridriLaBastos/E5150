%macro FDC_COMMAND 1
	mov dx, 0x3F4
	in al, dx

	mov dx, 0x3F5
	mov ax, %1
	out dx, al
%endmacro

%macro SEEK 2
	FDC_COMMAND(0xF)
	FDC_COMMAND(%1)
	FDC_COMMAND(%2)
%endmacro

[CPU 8086]

mov dx, 0x3F2
mov ax, 0b10000
out dx, ax

SEEK 0, 5

hlt