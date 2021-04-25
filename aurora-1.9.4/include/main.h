/*
 * aurora - Communications with Magnetek Aurora Inverter
 *
 * Copyright (C) 2006-2019 Curtis J. Blank curt@curtronics.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program may be used and hosted free of charge by anyone for
 * personal purposes as long as this and all included copyright
 * notice(s) remain intact.
 *
 * Obtain permission before selling the code for this program or
 * hosting this software on a commercial website, excluding if it is
 * solely hosted for distribution, that is allowable . In all cases
 * copyright must remain intact.
 *
 * This work based on Magnetek's 'Aurora PV Inverter - Communications
 * Protocol -' document, Rel. 1.8 09/05/2005
 * Staring with v1.5-0 this work based on Power One's 'Aurora Inverter
 * Series - Communication Protocol -' document, Rel. 4.6 25/02/09
 *
 * Special thanks to Tron Melzl at Magnetek for all his help including, but
 * by no means limited to, supplying the Communications Protocol document
 *
 */

#define	VersionMH	"1.9.1"

#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
#define extern "C" {		/* respect c++ callers */
#endif

#define	ProgramName	"aurora"

typedef int BOOL;		/* yet another bool definition */
#define TRUE 1
#define FALSE 0
#define _ERROR_ 0xffffffff

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#define LOBYTE(w)	((BYTE)(w))
#define HIBYTE(w)	((BYTE)(((WORD)(w) >> (BYTE)8) & 0xff))

extern BOOL bVerbose;                           /* Verbose Mode */
extern BOOL bColumns;
extern BOOL bGetInvTime;
extern BOOL bGetLocTime;
extern int yGetDSP;
extern BOOL bGetDSPExtended;
extern BOOL bGetDSP3Phase;
extern BOOL bHideDSP;
extern BOOL bGetEnergy;
extern BOOL bCommCheck;
extern BOOL bColOutput;
extern BOOL bCalcGridPwr;
extern BOOL bRptRetries;
extern BOOL bRptReadPause;
extern BOOL bSwapEndian;
extern BOOL bUseMultiplierQ;
extern float yMultiplierK;
extern float yCost;
extern char *sCostType;
extern int yTimeout;
extern long unsigned int PID;
extern int yMaxAttempts;
extern int yReadPause;
extern long int yCommPause;
extern unsigned char yDelay;
extern time_t startTimeValue;
extern int yGetCount;
extern int yCentral;
extern int yMaxRunTime;
extern int yCheckSetTime;
extern struct timeval lastcommtv;

extern FILE *outfp;

extern char* getCurTime();
extern int ClrSerLock(long unsigned int PID);
extern int RestorePort(int fdser);

#define	ttyLCKloc	"/var/lock/LCK.."	/* location and prefix of serial port lock file */
#define uSecsLCKWaitMax    100000		/* 100 millseconds */
#define uSecsLCKWaitMin    10000		/*  10 Millseconds */

/* macros */
#define MAX(a,b) ({ typeof (a) _a = (a); typeof (b) _b = (b); _a > _b ? _a : _b; })

/* command structure */
#define	cAddress	0
#define	cCommand	1
#define cCommandEnd	2
#define	cParam1		2
#define cParam1End	3
#define	cParam2		3
#define cParam2End	4
#define	cParam3		4
#define	cParam4		5
#define cParam4End	6
#define	cParam5		6
#define	cParam6		7
#define cCRC_L		8
#define	cCRC_H		9
#define cEND		10
#define cSIZE		10

/* answer structure */
#define	aState		0
#define aMState		1
#define	aParam1		2
#define	aParam2		3
#define	aParam3		4
#define	aParam4		5
#define	aCRC_L		6
#define	aCRC_H		7
#define aEND		8
#define aSIZE		8

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MAIN_H__ */

