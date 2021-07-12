[CPU 8086]

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

	FDC_RESULT
	FDC_RESULT
%endmacro

%macro RECALIBRATE 1
	FDC_COMMAND(0b111)
	FDC_COMMAND(%1)
%endmacro

%macro SENSE_DRIVE_STATUS 1
	FDC_COMMAND(0b100)
	FDC_COMMAND(%1)

	FDC_RESULT
%endmacro