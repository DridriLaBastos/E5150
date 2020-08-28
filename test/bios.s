%macro FDC_COMMAND 1
	mov dx, 0x3F4
	in al, dx

	mov dx, 0x3F5
	mov ax, %1
	out dx, al
%endmacro

%macro FDC_RESULT 0
	mov dx, 0x3F4
	in al, dx

	mov dx, 0x3F5
	in al, dx
%endmacro

%macro SEEK 2
	FDC_COMMAND(0xF)
	FDC_COMMAND(%1)
	FDC_COMMAND(%2)
%endmacro

;1 HUT, 2 SRT, 3 HLT
%macro SPECIFY 3
	FDC_COMMAND(0b11)
	FDC_COMMAND((%2 << 4) | (%1 & 0xF))
	FDC_COMMAND(%3 << 1)
%endmacro

%macro SENSE_IT_STATUS 1
sis:
	FDC_COMMAND(0b1000)

	xor cx, cx
	.loopJmp:
		mov dx, 0x3F4
		in al, dx
		and al, 0b1000
		jnz .out
		inc cx
		jmp .loopJmp
	.out:
	mov dx, 0x3F5
	in al, dx

	FDC_RESULT
	FDC_RESULT
%endmacro

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

SPECIFY 0xF,0xA,1
SEEK 0,5

hlt