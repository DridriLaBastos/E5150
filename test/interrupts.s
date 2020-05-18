section interruptVectors
;int0
dw int0
dw 0

;int1
dw int1
dw 0

;int2
dw int2
dw 0

times 0x500 - ($ - $$) db 0

section interruptImpl follows=interruptVectors
int0:
	iret

int1:
	iret

int2:
	iret