#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "8086.hpp"

//TODO: continue to implement data transfer instructions
/* Data Transfer */
void MOV	(CPU& cpu);
void PUSH	(CPU& cpu);
void POP	(CPU& cpu);
void XCHG	(CPU& cpu);
void IN		(CPU& cpu);
void OUT	(CPU& cpu);
void XLAT	(CPU& cpu);
void LEA	(CPU& cpu);
void LDS	(CPU& cpu);
void LES	(CPU& cpu);
void LAHF	(CPU& cpu);
void SAHF	(CPU& cpu);
void PUSHF	(CPU& cpu);
void POPF	(CPU& cpu);

//TODO: continue to implement arithmetic instructions
/* Arithmetic */
void ADD  (CPU& cpu);
void INC  (CPU& cpu);
void SUB  (CPU& cpu);
void DEC  (CPU& cpu);
void NEG  (CPU& cpu);
void CMP  (CPU& cpu);
void MUL  (CPU& cpu);
void IMUL (CPU& cpu);
void DIV  (CPU& cpu);
void IDIV (CPU& cpu);

/* Control Transfer */
void NEAR_CALL	(CPU& cpu);
void FAR_CALL	(CPU& cpu);
void NEAR_JMP	(CPU& cpu);
void FAR_JMP	(CPU& cpu);
void NEAR_RET	(CPU& cpu);
void FAR_RET	(CPU& cpu);
void JZ			(CPU& cpu);/*  JZ/JE   */
void JL			(CPU& cpu);/*  JL/JNGE */
void JLE		(CPU& cpu);/* JLE/JNG  */
void JNZ		(CPU& cpu);
void JNL		(CPU& cpu);
void JNLE		(CPU& cpu);
void LOOP		(CPU& cpu);
void JCXZ		(CPU& cpu);
void INT		(CPU& cpu);
void IRET		(CPU& cpu);

void NOT (CPU& cpu);

//TODO: continue to implements processor control instructions
/* Processor Control */
void CLC (CPU& cpu);
void STC (CPU& cpu);
void CLD (CPU& cpu);
void STD (CPU& cpu);
void CLI (CPU& cpu);
void STI (CPU& cpu);
void HLT (CPU& cpu);
void NOP (CPU& cpu);

#endif