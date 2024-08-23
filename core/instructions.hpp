#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "core/8086.hpp"

/* Data Transfer */

void MOV	(E5150::Intel8088* cpu); unsigned int getMOVCycles	(void);
void PUSH	(E5150::Intel8088* cpu); unsigned int getPUSHCycles	(void);
void POP	(E5150::Intel8088* cpu); unsigned int getPOPCycles	(void);
void XCHG	(E5150::Intel8088* cpu); unsigned int getXCHGCycles	(void);
//For an unknonw reason, msvc doesn't want to let me use the names OUT ans IN
void _IN	(E5150::Intel8088* cpu); unsigned int getINCycles	(void);
void _OUT	(E5150::Intel8088* cpu); unsigned int getOUTCycles	(void);
void XLAT	(E5150::Intel8088* cpu); unsigned int getXLATCycles	(void);
void LEA	(E5150::Intel8088* cpu); unsigned int getLEACycles	(void);
void LDS	(E5150::Intel8088* cpu); unsigned int getLDSCycles	(void);
void LES	(E5150::Intel8088* cpu); unsigned int getLESCycles	(void);
void LAHF	(E5150::Intel8088* cpu); unsigned int getLAHFCycles	(void);
void SAHF	(E5150::Intel8088* cpu); unsigned int getSAHFCycles	(void);
void PUSHF	(E5150::Intel8088* cpu); unsigned int getPUSHFCycles	(void);
void POPF	(E5150::Intel8088* cpu); unsigned int getPOPFCycles	(void);

/* Arithmetic */

void ADD	(E5150::Intel8088* cpu); unsigned int getADDCycles	(void);
//void ADC(void); unsigned int getADCCycles	(void);
void INC	(E5150::Intel8088* cpu); unsigned int getINCCycles	(void);
void AAA	(E5150::Intel8088* cpu); unsigned int getAAACycles	(void);
void DAA	(E5150::Intel8088* cpu); unsigned int getDAACycles	(void);
void SUB	(E5150::Intel8088* cpu); unsigned int getSUBCycles	(void);
//void SBB(void);unsigned int getSBBCycles	(void);
void DEC	(E5150::Intel8088* cpu); unsigned int getDECCycles	(void);
void NEG	(E5150::Intel8088* cpu); unsigned int getNEGCycles	(void);
void CMP	(E5150::Intel8088* cpu); unsigned int getCMPCycles	(void);
void AAS	(E5150::Intel8088* cpu); unsigned int getAASCycles	(void);
void DAS	(E5150::Intel8088* cpu); unsigned int getDASCycles	(void);
void MUL	(E5150::Intel8088* cpu); unsigned int getMULCycles	(void);
/*void IMUL	(void);*/unsigned int getIMULCycles	(void);
void DIV	(E5150::Intel8088* cpu); unsigned int getDIVCycles	(void);
/*void IDIV	(void);*/unsigned int getIDIVCycles	(void);
void AAD	(E5150::Intel8088* cpu); unsigned int getAADCycles	(void);
void CBW	(E5150::Intel8088* cpu); unsigned int getCBWCycles	(void);
void CWD	(E5150::Intel8088* cpu); unsigned int getCWDCycles	(void);

/* Logic */

void NOT	(E5150::Intel8088* cpu); unsigned int getNOTCycles	(void);
void SHIFT	(E5150::Intel8088* cpu); unsigned int getSHIFT_ROTATECycles	(const unsigned int nPrefix);
void ROTATE	(E5150::Intel8088* cpu); //Same number of cycles than SHIFT
// void SHL	(void); unsigned int getSHLCycles	(void);
// void SHR	(void); unsigned int getSHRCycles	(void);
// void SAR	(void); unsigned int getSARCycles	(void);
// void ROL	(void); unsigned int getROLCycles	(void);
// void ROR	(void); unsigned int getRORCycles	(void);
// void RCL	(void); unsigned int getRCLCycles	(void);
void RCR	(E5150::Intel8088* cpu); unsigned int getRCRCycles	(void);
void AND	(E5150::Intel8088* cpu); unsigned int getANDCycles	(void);
void TEST	(E5150::Intel8088* cpu); unsigned int getTESTCycles	(void);
void OR		(E5150::Intel8088* cpu); unsigned int getORCycles	(void);
void XOR	(E5150::Intel8088* cpu); unsigned int getXORCycles	(void);

/* String Manipulation */

void MOVS (E5150::Intel8088* cpu); unsigned int getMOVSCycles (void);
void REP_MOVS (E5150::Intel8088* cpu); unsigned int getREP_MOVSCycles (const unsigned int repeatCount);
void CMPS (E5150::Intel8088* cpu); unsigned int getCMPSCycles (void);
void REP_CMPS (E5150::Intel8088* cpu); unsigned int getREP_CMPSCycles (const unsigned int repeatCount);
void SCAS (E5150::Intel8088* cpu); unsigned int getSCASCycles (void);
void REP_SCAS (E5150::Intel8088* cpu); unsigned int getREP_SCASCycles (const unsigned int repeatCount);
void LODS (E5150::Intel8088* cpu); unsigned int getLODSCycles (void);
void REP_LODS (E5150::Intel8088* cpu); unsigned int getREP_LODSCycles (const unsigned int repeatCount);
void STOS (E5150::Intel8088* cpu); unsigned int getSTOSCycles (void);
void REP_STOS (E5150::Intel8088* cpu); unsigned int getREP_STOSCycles (const unsigned int repeatCount);

/* Control Transfer */

void CALL_NEAR	(E5150::Intel8088* cpu); unsigned int getCALLCycles (void);
void CALL_FAR	(E5150::Intel8088* cpu); unsigned int getCALL_FARCycles	(void);
void JMP_NEAR	(E5150::Intel8088* cpu); unsigned int getJMPCycles		(void);
void JMP_FAR	(E5150::Intel8088* cpu); unsigned int getJMP_FARCycles	(void);
void RET_NEAR	(E5150::Intel8088* cpu); unsigned int getRETCycles	(void);
void RET_FAR	(E5150::Intel8088* cpu); unsigned int getRET_FARCycles	(void);
/* JE/JZ   */void JZ			(E5150::Intel8088* cpu); unsigned int getJXXCycles		(const bool conditionValue);
/* JL/JNGE */void JL			(E5150::Intel8088* cpu); unsigned int getJLCycles		(void);
/* JLE/JNG  */void JLE		(E5150::Intel8088* cpu); // unsigned int getJLECycles		(void);
/* JB/JNAE */void JB			(E5150::Intel8088* cpu); // unsigned int getJBCycles		(void);
/* JBE/JNA  */void JBE		(E5150::Intel8088* cpu); // unsigned int getJBECycles		(void);
/* JLE/JNG  */void JP			(E5150::Intel8088* cpu); // unsigned int getJPCycles		(void);
/* JP/JPE  */void JO			(E5150::Intel8088* cpu); // unsigned int getJOCycles		(void);
void JS			(E5150::Intel8088* cpu); // unsigned int getJSCycles		(void);
void JNZ		(E5150::Intel8088* cpu); // unsigned int getJNZCycles		(void);
void JNL		(E5150::Intel8088* cpu); // unsigned int getJNLCycles		(void);
void JNLE		(E5150::Intel8088* cpu); // unsigned int getJNLECycles		(void);
void JNB		(E5150::Intel8088* cpu); // unsigned int getJNBCycles		(void);
void JNBE		(E5150::Intel8088* cpu); // unsigned int getJNBECycles		(void);
void JNP		(E5150::Intel8088* cpu); // unsigned int getJNPCycles		(void);
void JNS		(E5150::Intel8088* cpu); // unsigned int getJNSCycles		(void);
void LOOP		(E5150::Intel8088* cpu); unsigned int getLOOPCycles		(void);
void LOOPZ		(E5150::Intel8088* cpu); unsigned int getLOOPZCycles		(void);
void LOOPNZ		(E5150::Intel8088* cpu); unsigned int getLOOPNZCycles	(void);
void JCXZ		(E5150::Intel8088* cpu); unsigned int getJCXZCycles		(void);
void IRET		(E5150::Intel8088* cpu); unsigned int getIRETCycles		(void);

/* Processor Control */

void CLC	(E5150::Intel8088* cpu); unsigned int getCLCCycles	(void);
void CMC	(E5150::Intel8088* cpu); unsigned int getCMCCycles	(void);
void STC	(E5150::Intel8088* cpu); unsigned int getSTCCycles	(void);
void CLD	(E5150::Intel8088* cpu); unsigned int getCLDCycles	(void);
void STD	(E5150::Intel8088* cpu); unsigned int getSTDCycles	(void);
//CLI is already used as namespace name inside CLI11
void _CLI	(E5150::Intel8088* cpu); unsigned int getCLICycles	(void);
void STI	(E5150::Intel8088* cpu); unsigned int getSTICycles	(void);
void HLT	(E5150::Intel8088* cpu); unsigned int getHLTCycles	(void);
void WAIT	(E5150::Intel8088* cpu); unsigned int getWAITCycles	(void);
void LOCK	(E5150::Intel8088* cpu); unsigned int getLOCKCycles	(void);

void NOP	(E5150::Intel8088* cpu); unsigned int getNOPCycles	(void);

#endif
