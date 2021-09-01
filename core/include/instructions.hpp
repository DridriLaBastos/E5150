#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "8086.hpp"

/* Data Transfer */

void MOV	(void); unsigned int getMOVCycles	(void);
void PUSH	(void); unsigned int getPUSHCycles	(void);
void POP	(void); unsigned int getPOPCycles	(void);
void XCHG	(void); unsigned int getXCHGCycles	(void);
void IN		(void); unsigned int getINCycles	(void);
void OUT	(void); unsigned int getOUTCycles	(void);
void XLAT	(void); unsigned int getXLATCycles	(void);
void LEA	(void); unsigned int getLEACycles	(void);
void LDS	(void); unsigned int getLDSCycles	(void);
void LES	(void); unsigned int getLESCycles	(void);
void LAHF	(void); unsigned int getLAHFCycles	(void);
void SAHF	(void); unsigned int getSAHFCycles	(void);
void PUSHF	(void); unsigned int getPUSHFCycles	(void);
void POPF	(void); unsigned int getPOPFCycles	(void);

/* Arithmetic */

void ADD	(void); unsigned int getADDCycles	(void);
//void ADC(void); unsigned int getADCCycles	(void);
void INC	(void); unsigned int getINCCycles	(void);
void AAA	(void); unsigned int getAAACycles	(void);
void DAA	(void); unsigned int getDAACycles	(void);
void SUB	(void); unsigned int getSUBCycles	(void);
//void SBB(void);unsigned int getSBBCycles	(void);
void DEC	(void); unsigned int getDECCycles	(void);
void NEG	(void); unsigned int getNEGCycles	(void);
void CMP	(void); unsigned int getCMPCycles	(void);
void AAS	(void); unsigned int getAASCycles	(void);
void DAS	(void); unsigned int getDASCycles	(void);
void MUL	(void); unsigned int getMULCycles	(void);
/*void IMUL	(void);*/unsigned int getIMULCycles	(void);
void DIV	(void); unsigned int getDIVCycles	(void);
/*void IDIV	(void);*/unsigned int getIDIVCycles	(void);
void AAD	(void); unsigned int getAADCycles	(void);
void CBW	(void); unsigned int getCBWCycles	(void);
void CWD	(void); unsigned int getCWDCycles	(void);

/* Logic */

void NOT	(void); unsigned int getNOTCycles	(void);
void SHIFT	(void); unsigned int getSHIFT_ROTATECycles	(void);
void ROTATE	(void); //Same number of cycles than SHIFT
// void SHL	(void); unsigned int getSHLCycles	(void);
// void SHR	(void); unsigned int getSHRCycles	(void);
// void SAR	(void); unsigned int getSARCycles	(void);
// void ROL	(void); unsigned int getROLCycles	(void);
// void ROR	(void); unsigned int getRORCycles	(void);
// void RCL	(void); unsigned int getRCLCycles	(void);
void RCR	(void); unsigned int getRCRCycles	(void);
void AND	(void); unsigned int getANDCycles	(void);
void TEST	(void); unsigned int getTESTCycles	(void);
void OR		(void); unsigned int getORCycles	(void);
void XOR	(void); unsigned int getXORCycles	(void);

/* String Manipulation */

void REP (void); unsigned int getREPCycles (void);
void MOVS (void); unsigned int getMOVSCycles (void);
void CMPS (void); unsigned int getCMPSCycles (void);
void SCAS (void); unsigned int getSCASCycles (void);
void LODS (void); unsigned int getLODSCycles (void);
void STOS (void); unsigned int getSTOSCycles (void);

/* Control Transfer */

void CALL_NEAR	(void); unsigned int getCALL_NEARCycles	(void);
void CALL_FAR	(void); unsigned int getCALL_FARCycles	(void);
void JMP_NEAR	(void); unsigned int getJMP_NEARCycles	(void);
void JMP_FAR	(void); unsigned int getJMP_FARCycles	(void);
void RET_NEAR	(void); unsigned int getRET_NEARCycles	(void);
void RET_FAR	(void); unsigned int getRET_FARCycles	(void);
void JZ			(void); unsigned int getJZCycles		(void);/* JE /JZ   */
void JL			(void); unsigned int getJLCycles		(void);/* JL /JNGE */
void JLE		(void); unsigned int getJLECycles		(void);/* JLE/JNG  */
void JB			(void); unsigned int getJBCycles		(void);/* JB /JNAE */
void JBE		(void); unsigned int getJBECycles		(void);/* JBE/JNA  */
void JP			(void); unsigned int getJPCycles		(void);/* JLE/JNG  */
void JO			(void); unsigned int getJOCycles		(void);/* JP /JPE  */
void JS			(void); unsigned int getJSCycles		(void);
void JNZ		(void); unsigned int getJNZCycles		(void);
void JNL		(void); unsigned int getJNLCycles		(void);
void JNLE		(void); unsigned int getJNLECycles		(void);
void JNB		(void); unsigned int getJNBCycles		(void);
void JNBE		(void); unsigned int getJNBECycles		(void);
void JNP		(void); unsigned int getJNPCycles		(void);
void JNS		(void); unsigned int getJNSCycles		(void);
void LOOP		(void); unsigned int getLOOPCycles		(void);
void LOOPZ		(void); unsigned int getLOOPZCycles		(void);
void LOOPNZ		(void); unsigned int getLOOPNZCycles	(void);
void JCXZ		(void); unsigned int getJCXZCycles		(void);
void INT		(void); unsigned int getINTCycles		(void);
void INTO		(void); unsigned int getINTOCycles		(void);
void IRET		(void); unsigned int getIRETCycles		(void);

/* Processor Control */

void CLC	(void); unsigned int getCLCCycles	(void);
void CMC	(void); unsigned int getCMCCycles	(void);
void STC	(void); unsigned int getSTCCycles	(void);
void CLD	(void); unsigned int getCLDCycles	(void);
void STD	(void); unsigned int getSTDCycles	(void);
void CLI	(void); unsigned int getCLICycles	(void);
void STI	(void); unsigned int getSTICycles	(void);
void HLT	(void); unsigned int getHLTCycles	(void);
void WAIT	(void); unsigned int getWAITCycles	(void);
void LOCK	(void); unsigned int getLOCKCycles	(void);

void NOP	(void); unsigned int getNOPCycles	(void);

#endif