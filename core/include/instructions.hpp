#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "8086.hpp"

//TODO: continue to implement data transfer instructions
/* Data Transfer */
void MOV	(void); unsigned int getMOVCycles (void);
void PUSH	(void); unsigned int getPUSHCycles (void);
void POP	(void); unsigned int getPOPCycles (void);
void XCHG	(void); unsigned int getXCHGCycles (void);
void IN		(void); unsigned int getINCycles (void);
void OUT	(void); unsigned int getOUTCycles (void);
void XLAT	(void); unsigned int getXLATCycles (void);
void LEA	(void); unsigned int getLEACycles (void);
void LDS	(void); unsigned int getLDSCycles (void);
void LES	(void); unsigned int getLESCycles (void);
void LAHF	(void); unsigned int getLAHFCycles (void);
void SAHF	(void); unsigned int getSAHFCycles (void);
void PUSHF	(void); unsigned int getPUSHFCycles (void);
void POPF	(void); unsigned int getPOPFCycles (void);

//TODO: continue to implement arithmetic instructions
/* Arithmetic */
void ADD  (void); unsigned int getADDCycles (void);
void INC  (void); unsigned int getINCCycles (void);
void SUB  (void); unsigned int getSUBCycles (void);
void DEC  (void); unsigned int getDECCycles (void);
void NEG  (void); unsigned int getNEGCycles (void);
void CMP  (void); unsigned int getCMPCycles (void);
void MUL  (void); unsigned int getMULCycles (void);
void IMUL (void); unsigned int getIMULCycles (void);
void DIV  (void); unsigned int getDIVCycles (void);
void IDIV (void); unsigned int getIDIVCycles (void);

/* Control Transfer */
void CALL_NEAR	(void); unsigned int getCALL_NEARCycles (void);
void CALL_FAR	(void); unsigned int getCALL_FARCycles (void);
void JMP_NEAR	(void); unsigned int getJMP_NEARCycles (void);
void JMP_FAR	(void); unsigned int getJMP_FARCycles (void);
void RET_NEAR	(void); unsigned int getRET_NEARCycles (void);
void RET_FAR	(void); unsigned int getRET_FARCycles (void);
void JZ			(void);/*  JZ/JE   */ unsigned int getJZCycles (void);
void JL			(void);/*  JL/JNGE */ unsigned int getJLCycles (void);
void JLE		(void);/* JLE/JNG  */ unsigned int getJLECycles (void);
void JNZ		(void); unsigned int getJNZCycles (void);
void JNL		(void); unsigned int getJNLCycles (void);
void JNLE		(void); unsigned int getJNLECycles (void);
void LOOP		(void); unsigned int getLOOPCycles (void);
void JCXZ		(void); unsigned int getJCXZCycles (void);
void INT		(void); unsigned int getINTCycles (void);
void IRET		(void); unsigned int getIRETCycles (void);

void NOT (void); unsigned int getNOTCycles (void);

//TODO: continue to implements processor control instructions
/* Processor Control */
void CLC (void); unsigned int getCLCCycles (void);
void STC (void); unsigned int getSTCCycles (void);
void CLD (void); unsigned int getCLDCycles (void);
void STD (void); unsigned int getSTDCycles (void);
void CLI (void); unsigned int getCLICycles (void);
void STI (void); unsigned int getSTICycles (void);
void HLT (void); unsigned int getHLTCycles (void);
void NOP (void); unsigned int getNOPCycles (void);

#endif