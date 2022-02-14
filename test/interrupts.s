%include "test/util.inc"

dd 0
dd int1
times 0xAE dd intDef

times 6 dd intDef
dd int6

intDef:
	iret

int1:
	xchg ax, ax
	iret

int6:
	SENSE_IT_STATUS 0
	mov ax, 0x20
	mov dx, 0x20
	out dx, al
	
	iret
