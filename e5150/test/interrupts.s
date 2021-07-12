%include "test/util.inc"

section interruptVectors
times 0xB0*4 db 0

times 6 dd intDef
dd int6

section interruptImpl follows=interruptVectors
intDef:
	iret

int6:
	SENSE_IT_STATUS 0
	mov ax, 0x20
	mov dx, 0x20
	out dx, al
	
	iret