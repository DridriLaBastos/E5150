#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "8086.hpp"

//TODO: continue to implement data transfer instructions
/* Data Transfer */
void MOV	(CPU&);
void PUSH	(CPU&);
void POP	(CPU&);
void XCHG	(CPU&);
void IN		(CPU&);
void OUT	(CPU&);
void XLAT	(CPU&);
void LEA	(CPU&);
void LDS	(CPU&);
void LES	(CPU&);
void LAHF	(CPU&);
void SAHF	(CPU&);
void PUSHF	(CPU&);
void POPF	(CPU&);

//TODO: continue to implement arithmetic instructions
/* Arithmetic */
void ADD  (CPU&);
void INC  (CPU&);
void SUB  (CPU&);
void DEC  (CPU&);
void NEG  (CPU&);
void CMP  (CPU&);
void MUL  (CPU&);
void IMUL (CPU&);
void DIV  (CPU&);
void IDIV (CPU&);

/* Control Transfer */
void NEAR_CALL	(CPU&);
void FAR_CALL	(CPU&);
void NEAR_JMP	(CPU&);
void FAR_JMP	(CPU&);
void NEAR_RET	(CPU&);
void FAR_RET	(CPU&);
void JZ			(CPU&);/*  JZ/JE   */
void JL			(CPU&);/*  JL/JNGE */
void JLE		(CPU&);/* JLE/JNG  */
void JNZ		(CPU&);
void JNL		(CPU&);
void JNLE		(CPU&);
void LOOP		(CPU&);
void JCXZ		(CPU&);
void INT		(CPU&);
void IRET		(CPU&);

void NOT (CPU&);

//TODO: continue to implements processor control instructions
/* Processor Control */
void CLC (CPU&);
void STC (CPU&);
void CLD (CPU&);
void STD (CPU&);
void CLI (CPU&);
void STI (CPU&);
void HLT (CPU&);
void NOP (CPU&);

#endif