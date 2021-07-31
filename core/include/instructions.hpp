#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "8086.hpp"

//TODO: continue to implement data transfer instructions
/* Data Transfer */
void execMOV	(CPU&); unsigned int getMOVCycles (void);
void execPUSH	(CPU&); unsigned int getPUSHCycles (void);
void execPOP	(CPU&); unsigned int getPOPCycles (void);
void execXCHG	(CPU&); unsigned int getXCHGCycles (void);
void execIN		(CPU&); unsigned int getINCycles (void);
void execOUT	(CPU&); unsigned int getOUTCycles (void);
void execXLAT	(CPU&); unsigned int getXLATCycles (void);
void execLEA	(CPU&); unsigned int getLEACycles (void);
void execLDS	(CPU&); unsigned int getLDSCycles (void);
void execLES	(CPU&); unsigned int getLESCycles (void);
void execLAHF	(CPU&); unsigned int getLAHFCycles (void);
void execSAHF	(CPU&); unsigned int getSAHFCycles (void);
void execPUSHF	(CPU&); unsigned int getPUSHFCycles (void);
void execPOPF	(CPU&); unsigned int getPOPFCycles (void);

//TODO: continue to implement arithmetic instructions
/* Arithmetic */
void execADD  (CPU&); unsigned int getADDCycles (void);
void execINC  (CPU&); unsigned int getINCCycles (void);
void execSUB  (CPU&); unsigned int getSUBCycles (void);
void execDEC  (CPU&); unsigned int getDECCycles (void);
void execNEG  (CPU&); unsigned int getNEGCycles (void);
void execCMP  (CPU&); unsigned int getCMPCycles (void);
void execMUL  (CPU&); unsigned int getMULCycles (void);
void execIMUL (CPU&); unsigned int getIMULCycles (void);
void execDIV  (CPU&); unsigned int getDIVCycles (void);
void execIDIV (CPU&); unsigned int getIDIVCycles (void);

/* Control Transfer */
void execNEAR_CALL	(CPU&); unsigned int getNEAR_CALLCycles (void);
void execFAR_CALL	(CPU&); unsigned int getFAR_CALLCycles (void);
void execNEAR_JMP	(CPU&); unsigned int getNEAR_JMPCycles (void);
void execFAR_JMP	(CPU&); unsigned int getFAR_JMPCycles (void);
void execNEAR_RET	(CPU&); unsigned int getNEAR_RETCycles (void);
void execFAR_RET	(CPU&); unsigned int getFAR_RETCycles (void);
void execJZ			(CPU&);/*  JZ/JE   */ unsigned int getJZCycles (void);
void execJL			(CPU&);/*  JL/JNGE */ unsigned int getJLCycles (void);
void execJLE		(CPU&);/* JLE/JNG  */ unsigned int getJLECycles (void);
void execJNZ		(CPU&); unsigned int getJNZCycles (void);
void execJNL		(CPU&); unsigned int getJNLCycles (void);
void execJNLE		(CPU&); unsigned int getJNLECycles (void);
void execLOOP		(CPU&); unsigned int getLOOPCycles (void);
void execJCXZ		(CPU&); unsigned int getJCXZCycles (void);
void execINT		(CPU&); unsigned int getINTCycles (void);
void execIRET		(CPU&); unsigned int getIRETCycles (void);

void execNOT (CPU&); unsigned int getNOTCycles (void);

//TODO: continue to implements processor control instructions
/* Processor Control */
void execCLC (CPU&); unsigned int getCLCCycles (void);
void execSTC (CPU&); unsigned int getSTCCycles (void);
void execCLD (CPU&); unsigned int getCLDCycles (void);
void execSTD (CPU&); unsigned int getSTDCycles (void);
void execCLI (CPU&); unsigned int getCLICycles (void);
void execSTI (CPU&); unsigned int getSTICycles (void);
void execHLT (CPU&); unsigned int getHLTCycles (void);
void execNOP (CPU&); unsigned int getNOPCycles (void);

#endif