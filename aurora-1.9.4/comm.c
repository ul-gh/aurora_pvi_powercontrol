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
 * And another thanks to Jonathan Duddington on the other side of the pond
 * for his contribution of items he gathered while reverse engineering the 
 * inverters communication some of which were incorporated beginning with 
 * verison 1.7.8
 *
 * modified 17-jul-2006 cjb	1. Last 7 Days production value has been dropped in the v2.3 Communications Protocol doc
 * modified 13-oct-2006 cjb	1. correct possible divide by zero when calculating inverter efficiency
 * modified 25-apr-2007 cjb	1. update set time warning
 *				2. take into account Daylight Savings Time when setting the Aurora's time
 * modified 29-dec-2008 cjb     1. correct an error in strftime that only may show up in the first or last
 *                                 week of the year (%G vs %Y)
 * modified 19-aug-2009 cjb	1. szSerBuffer needs to be [11] if ending with "\0"
 * modified 18-sep-2009 cjb	1. add cCommandEnd = '\0'
 * modified 12-oct-2009 cjb	1. add -o option to output data to a file
 * modified 30-oct-2009 cjb     1. added errno for read problems
 * modified 07-mar-2010 cjb	1. fix sizeof pointer passing in memset
 *				2. use -Y option in Communicate function
 *				3. in ReadNextChar restore serial port settings and clear lock if exiting
 * modified 13-mar-2010 cjb	1. if yReadPause is set use it
 * modified 28-mar-2010 cjb	1. working on adding more DSP information
 * modified 27-jul-2010 cjb	1. added -P option to throttle commands sent to the inverter
 * modified 21-sep-2010 cjb	1. fix using wrong param when displaying TransState message
 *				2. pass TransState the command description so if it gets a non-zero status it can
 *				   report for what command it got it
 *				3. added reporting for "Last four alarms"
 * modified 30-jul-2011 cjb	1. fixed joule conversion error
 * modified 06-aug-2011 cjb	1. fixed a integer conversion issue for -k due to some architectures
 *                              2. fixed bizarreness with -bt
 *                              3. adjust TimeBase to true UTC-0000 and take into account timezone in GetTime (-t)
 *                                 SetTime (-S) functions
 * modified 22-aug-2011 cjb	1. in SetTime check scanf return value rc
 * modified 05-nov-2011 cjb     1. added function for -q --energy-sent option
 * modified 09-nov-2011 cjb     1. added function for -L --reconcile-time option
 * modified 13-nov-2011 cjb	1. added handling for -W option swap endian
 * modified 22-nov-2011 cjb	1. fixed -k, -n, & -p strings output that were overflowing on occasion
 * modified 21-dec-2011 cjb     1. fixed some bad trailing characters showing up in date strings on MIPS platforms
 * modified 31-dec-2011 cjb	1. --energy-sent values are W not kW Doh!
 *                              2. Grid Current & Grid Power can now return negative values
 * modified 02-jan-2012 cjb     1. add bad return code tracking
 * modified 03-jan-2012 cjb	1. fix W not kW in all spots for --energy-sent. My bad.
 * modified 07-jan-2012 cjb     1. timing issues again setting the inverter time waiting for inverter response
 * modified 15-jan-2012 cjb	1. --daily-kwh date started tomorrow instead of today, adjusted timebase
 *                              2. dynamically adjust timebase for --daily-kwh if it looks to be off
 *                              3. watch for out of order or duplicate dates in --daily-kwh
 * modified 30-jan-2012 cjb	1. be more aggressive and at the same time patient trying to read characters from 
 *				   slow to respond inverters (READ! the README for the impacts this has)
 *                              2. added -M option maximum amount of time in seconds that aurora
 *                                 will run
 * modified 16-feb-2012 cjb     1. use multiplier for --energy-sent if known for inverter model
 *                              2. revamp --daily-kwh after discovering data may not always be contiguous
 * modified 24-feb-2012 cjb     1. modify -i, --get-count option to use a bitmask indictating what to do
 * modified 28-feb-2012 cjb	1. initialize some variables so as to avoid compiler warnings
 *                              2. check validity of value returned by GetInvTime to be sure inverter responded
 * modified 29-feb-2012 cjb	1. allow display of out of order dates for -k (--daily-kwh) if they appear to be
 *                                 valid this may then require the output be sorted
 * modified 05-mar.2012 cjb	1. handle out of order dates for -k (--daily-kwh) by sorting them and suppressing
 *                                 duplicate entries for a date
 * modified 18-mar-2013 cjb	1. add -Q option and only use qMultiplier for -q, --energy-sent if -Q is used
 *                              2. add -K option and use value supplied for -k, --daily-kwh value multiplier
 * modified 16-nov-2013 cjb	1. handle missing tm_gmtoff in tm structure
 * modified 05-dec-2013 cjb	1. temperatures can be below zero i.e. negative (inverters installed outdoors)
 * modified 11-apr-2014 cjb	1. correct reporting wrong model when -Q is not used 
 * modified 11-jul-2016 cjb	1. if returning _ERROR_ and veborse print out debug info
 *				2. change how _ERROR_ is determined in float routines
 * modified 12-mar-2019 cjb	1. correct a typo in GetPNC CMD 105 error reporting
 *
 */

char     VersionC[7] = "1.9.4";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <error.h>
#include "include/main.h"
#include "include/comm.h"
#include "include/names.h"
#include "include/states.h"

char VersionSHc[6] = VersionSH;
int invOp;
int invFunc;

static char szSerBuffer[_szSerBufferLen];			/* serial read/write buffer */
static WORD crcValue;
static int ModelID;
static float qMultiplier = -1.0;
static char ModelName[_ModelNameLen];
static int star[5] = { '|','/','-','\\','*' };
static BOOL CmdNotImp = FALSE;

/* local functions */
static long int getGMToffset(time_t seconds);
static int TransState(int TransCode, char Desription[], BOOL FailIf);
static int PrintModel(void);
static unsigned long GetCEdata(int fdser, int yAddress, int opcode, int param, char description[]);
static float GetCECdata(int fdser, int yAddress, int opcode, int param, char description[], int yGetEnergyCen);
static float GetDSPdata(int fdser, int yAddress, int opcode, int param, char description[], BOOL TScheck);
static time_t GetInvTime(int fdser, int yAddress);
static int GetCountersData(int fdser, int yAddress, int param, char *uptime);
static BOOL exceededRT();
static int Communicate(int fdser, int yAddress);
static int ReadNextChar(int nfd, char *pChar, int timeout);
static int ReadToBuffer(int nfd, char *pszBuffer, int nBufSize);
static WORD crc16(char *data_p, unsigned short length);
static char* FindString(WORD wRule, char *ptr);
static float szCvrtFloat(char *Buffer);
static unsigned short szCvrtShort(char *Buffer);
static unsigned long szCvrtLong(char *Buffer);
static unsigned long cvrtLong(char *Buffer);


/*--------------------------------------------------------------------------
    TransState
----------------------------------------------------------------------------*/
int TransState(int TransCode, char Desription[], BOOL FailIf)
{
    if (bVerbose) fprintf(stderr, "Transmission State Check: %i\n",TransCode);
    CmdNotImp = FALSE;
    if (TransCode == 0) return(1);
    if (TransCode == 51 || TransCode == 52) {
        CmdNotImp = TRUE;
        if (! FailIf) return(1);
    }
    fprintf(outfp, "\nTransmission State: %2i Command: \"%s\" %s\n\n",TransCode,Desription,FindString(TransCode, szTransStates));
    return(0);
}


/*--------------------------------------------------------------------------
    CommCheck
----------------------------------------------------------------------------*/
int CommCheck(int fdser, int yAddress)
{
    int nCnt;
    int pos = 0;

    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = opGetVer;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetVer,TRUE)) {
        ModelID = szSerBuffer[aPar1];
        while (ModelID != model_names[pos].ID && model_names[pos].ID >= 0) pos++;
        if (bUseMultiplierQ) qMultiplier = model_names[pos].multipler;
        strcpy(ModelName,model_names[pos].name);
        if (bVerbose) {
            fprintf(stderr, "Model ID \"%c\" qMultiplier %.7f ",ModelID,qMultiplier);
            PrintModel();
        }
        return(0);
    }
    invOp = 0;

    invFunc = -1;
    return(-1);
}

/*--------------------------------------------------------------------------
    getGMToffset
----------------------------------------------------------------------------*/
long int getGMToffset(time_t seconds)
{
    long int gmtoff = 0;
    struct tm local;
#ifdef NO_TM_GMTOFF
    struct tm gmt;
    int day = 0;
    int hour = 0;
    int min = 0;
#endif

    local = *localtime(&seconds);
#ifndef NO_TM_GMTOFF
    gmtoff = local.tm_gmtoff;
#else
    gmt = *gmtime(&seconds);
    day = local.tm_yday - gmt.tm_yday;
    hour = ((day < -1 ? 24 : 1 < day ? -24 : day * 24) + local.tm_hour - gmt.tm_hour);
    min = hour * 60 + local.tm_min - gmt.tm_min;
    gmtoff = min * 60;
#endif
    if (bVerbose) fprintf(stderr, "gmtoff %li\n", gmtoff);
    return(gmtoff);
}


/*--------------------------------------------------------------------------
    GetState
----------------------------------------------------------------------------*/
int GetState(int fdser, int yAddress)
{
    int nCnt;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetState;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetState,TRUE)) {
        fprintf(outfp, "\nGlobal State:          %s\n",FindString((int)szSerBuffer[aMState], szGlobalStates));
        fprintf(outfp, "Inverter State:        %s\n",FindString((int)szSerBuffer[aParam1], szInverterState));
        fprintf(outfp, "Channel 1 Dc/Dc State: %s\n",FindString((int)szSerBuffer[aParam2], szDcDcStatus));
        fprintf(outfp, "Channel 2 Dc/Dc State: %s\n",FindString((int)szSerBuffer[aParam3], szDcDcStatus));
        fprintf(outfp, "Alarm State:           %s\n",FindString((int)szSerBuffer[aParam4], szAlarmState));
        return(0);
    }
    return(-1);
}


/*--------------------------------------------------------------------------
    GetLastAlarms
----------------------------------------------------------------------------*/
int GetLastAlarms(int fdser, int yAddress)
{
    int nCnt;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));	/*clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;			/*set inverter address */
    szSerBuffer[cCommand] = invOp = opGetLastAlarms;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetLastAlarms,TRUE)) {
        fprintf(outfp, "\nAlarm 1:               %s\n",FindString((int)szSerBuffer[aParam1], szAlarmState));
        fprintf(outfp, "Alarm 2:               %s\n",FindString((int)szSerBuffer[aParam2], szAlarmState));
        fprintf(outfp, "Alarm 3:               %s\n",FindString((int)szSerBuffer[aParam3], szAlarmState));
        fprintf(outfp, "Alarm 4:               %s\n",FindString((int)szSerBuffer[aParam4], szAlarmState));
        return(0);
    }
    return(-1);
}


/*--------------------------------------------------------------------------
    GetPN CMD 52
----------------------------------------------------------------------------*/
int GetPN(int fdser, int yAddress)
{
    int nCnt;
    char PartNumber[7];

    PartNumber[0] = '\0';
    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetPN;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0) {
       strncpy(PartNumber, szSerBuffer, 6);
       PartNumber[6] = '\0';
       fprintf(outfp, "\nPart Number: %s\n",PartNumber);
       return(0);
   }
    return(-1);
}


/*--------------------------------------------------------------------------
    GetPNC CMD 105
----------------------------------------------------------------------------*/
int GetPNC(int fdser, int yAddress)
{
    int nCnt;
    char PartNumber[7];

    if (ModelID != 'C' && ModelID != 'M' && ModelID != 'L' && ModelID != 'B' && ModelID != 'A') {
        fprintf(outfp, "\nCannot query for Part Number, does not appear to be a Central inverter (%c)\n\n", ModelID);
        return(-1);
    }
    PartNumber[0] = '\0';
    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));          /* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;           /* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetPNC;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0) {
       strncpy(PartNumber, szSerBuffer, 6);
       PartNumber[6] = '\0';
       fprintf(outfp, "\nPart Number (Central): %s\n",PartNumber);
       return(0);
   }
    return(-1);
}


/*--------------------------------------------------------------------------
    PrintModel
----------------------------------------------------------------------------*/
int PrintModel(void)
{
    if (outfp != stderr) fprintf(outfp, "\nInverter Version: ");
    fprintf(outfp, "-- %s --\n",ModelName);
    return(0);
}


/*--------------------------------------------------------------------------
    GetVer
----------------------------------------------------------------------------*/
int GetVer(int fdser, int yAddress)
{
    int nCnt;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetVer;
    szSerBuffer[cCommandEnd] = '\0';
    szSerBuffer[cParam1] = '.';
    nCnt = Communicate(fdser, yAddress);
    crcValue = crc16(szSerBuffer, 8);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetVer,TRUE)) {
        if (bVerbose)
            fprintf(stderr, "aPar2  %c aPar3  %c aPar4  %c\naPar2 %2x aPar3 %2x aPar4 %2x\n", szSerBuffer[aPar2], szSerBuffer[aPar3], szSerBuffer[aPar4], szSerBuffer[aPar2], szSerBuffer[aPar3], szSerBuffer[aPar4]);
        PrintModel();
        switch (szSerBuffer[aPar2]) {
            case 'A':       fprintf(outfp, "%s -- ",aPar2A); break;
            case 'E':       fprintf(outfp, "%s -- ",aPar2E); break;
            case 'S':       fprintf(outfp, "%s -- ",aPar2S); break;
            case 'I':       fprintf(outfp, "%s -- ",aPar2I); break;
            case 'U':       fprintf(outfp, "%s -- ",aPar2U); break;
            case 'K':       fprintf(outfp, "%s -- ",aPar2K); break;
            case 'F':       fprintf(outfp, "%s -- ",aPar2F); break;
            case 'R':       fprintf(outfp, "%s -- ",aPar2R); break;
            case 'B':       fprintf(outfp, "%s -- ",aPar2B); break;
            case 'O':       fprintf(outfp, "%s -- ",aPar2O); break;
            case 'G':       fprintf(outfp, "%s -- ",aPar2G); break;
            case 'T':       fprintf(outfp, "%s -- ",aPar2T); break;
            case 'C':       fprintf(outfp, "%s -- ",aPar2C); break;
            case 'Q':       fprintf(outfp, "%s -- ",aPar2Q); break;
            case 'a':       fprintf(outfp, "%s -- ",aPar2a); break;
            case 'b':       fprintf(outfp, "%s -- ",aPar2b); break;
            case 'c':       fprintf(outfp, "%s -- ",aPar2c); break;
            case 'X':       fprintf(outfp, "%s -- ",aPar2X); break;
            case 'x':       fprintf(outfp, "%s -- ",aPar2x); break;
            case 'u':       fprintf(outfp, "%s -- ",aPar2u); break;
            case 'k':       fprintf(outfp, "%s -- ",aPar2k); break;
            case 'W':       fprintf(outfp, "%s -- ",aPar2W); break;
            case 'H':       fprintf(outfp, "%s -- ",aPar2H); break;
            case 'o':       fprintf(outfp, "%s -- ",aPar2o); break;
            case 'P':       fprintf(outfp, "%s -- ",aPar2P); break;
            case 'e':       fprintf(outfp, "%s -- ",aPar2e); break;
        }

        switch (szSerBuffer[aPar3]) {
            case 'T':       fprintf(outfp, "%s -- ",aPar3T); break;
            case 'N':       fprintf(outfp, "%s -- ",aPar3N); break;
            case 't':       fprintf(outfp, "%s -- ",aPar3t); break;
            case 'X':       fprintf(outfp, "%s -- ",aPar3X); break;
            default:        fprintf(outfp, "%s -- ","unknown"); break;
        }
        switch (szSerBuffer[aPar4]) {
            case 'W':       fprintf(outfp, "%s -- ",aPar4W); break;
            case 'N':       fprintf(outfp, "%s -- ",aPar4N); break;
            case 'X':       fprintf(outfp, "%s -- ",aPar4X); break;
            default:        fprintf(outfp, "%s -- ","unknown"); break;
        }
        fprintf(outfp, "\n");
        return(0);
    }
    return(-1);
}


/*--------------------------------------------------------------------------
    GetConf
----------------------------------------------------------------------------*/
int GetConf(int fdser, int yAddress)
{
    int nCnt;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetConfig;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    crcValue = crc16(szSerBuffer, 8);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetConfig,TRUE)) {
        switch (szSerBuffer[aConfCode]) {
            case ConfCode0:	fprintf(outfp, "\n%s\n",_ConfCode0); break;
            case ConfCode1:	fprintf(outfp, "\n%s\n",_ConfCode1); break;
            case ConfCode2:	fprintf(outfp, "\n%s\n",_ConfCode2); break;
            default:        break;
        }
        return(0);
    }
    return(-1);
}



/*--------------------------------------------------------------------------
    GetSN
----------------------------------------------------------------------------*/
int GetSN(int fdser, int yAddress)
{
    int nCnt;
    char SerialNumber[7];

    SerialNumber[0] = '\0';
    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetSN;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0) {
        strncpy(SerialNumber, szSerBuffer, 6);
        SerialNumber[6] = '\0';
        fprintf(outfp, "\nSerial Number: %s\n",SerialNumber);
        return(0);
   }
    return(-1);
}


/*--------------------------------------------------------------------------
    GetVerFW
----------------------------------------------------------------------------*/
int GetVerFW(int fdser, int yAddress)
{
    int nCnt;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetVerFW;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetVerFW,TRUE)) {
        fprintf(outfp, "\nFirmware: %c.%c.%c.%c\n",szSerBuffer[aRel3],szSerBuffer[aRel2],szSerBuffer[aRel1],szSerBuffer[aRel0]);
        return(0);
   }
    return(-1);
}


/*--------------------------------------------------------------------------
    GetMfgDate
----------------------------------------------------------------------------*/
int GetMfgDate(int fdser, int yAddress)
{
    int nCnt;
    char MfgWeek[3] = "  \0";
    char MfgYear[3] = "  \0";

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetMfg;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetMfg,TRUE)) {
        MfgWeek[0] = szSerBuffer[aWeekH];
        MfgWeek[1] = szSerBuffer[aWeekL];
        MfgYear[0] = szSerBuffer[aYearH];
        MfgYear[1] = szSerBuffer[aYearL];

        fprintf(outfp, "\nManufacturing Date: Year %s Week %s\n",MfgYear,MfgWeek);
        return(0);
   }
    return(-1);
}

/*--------------------------------------------------------------------------
    GetCESent ** Experimental **
----------------------------------------------------------------------------*/
int GetCESent(int fdser, int yAddress, int yGetEnergySent)
{
    int nCnt = 0;
    time_t timeValLong = 0;
    time_t timeValLongLast = 0;
    long int gmtoff = 0;
    long int gmtoffTS = 0;
    long int gmtoffTSnext = 0;
    struct tm TS, TSnext;
    char DT[18] = " ";
    int addC = 0;
    int addS = 0;
    BOOL loop = TRUE;
    BOOL beg = FALSE;
    BOOL tsync = FALSE;
    BOOL cr = FALSE;
    BOOL odd = FALSE;
    int value1 = 0;
    int value2 = 0;
    static char tBuf[4];
    int ffff = 0;
    int cnt = 0;
    int i = 0;

    if (bVerbose)fprintf(stderr, "TimeBase %lu timeValLong %lu\n",(long)TimeBase,timeValLong);
    if ((timeValLong = GetInvTime(fdser, yAddress)) < 0) return(-1);
    TS = *(localtime(&timeValLong));
    gmtoff = getGMToffset(timeValLong);
    if (bVerbose) {
        strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
        DT[sizeof(DT)-1] = '\0';
        fprintf(stderr, "Inverter Time: %17s\t0.0 timeValLong %lu gmtoff %li TimeBase %lu\n",DT,timeValLong,gmtoff,(long)TimeBase);
    }

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));	/*clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;			/*set inverter address */
    szSerBuffer[cCommand] = invOp = HIBYTE(opGetCESent);
    szSerBuffer[cParam1] = invFunc = LOBYTE(opGetCESent);
    szSerBuffer[cParam1End] = '\0';
    nCnt = Communicate(fdser, yAddress);

    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetCESent,TRUE)) {
        addS = szCvrtShort(szSerBuffer);
        if (bVerbose) {
            fprintf(stderr, "\nszSerBuffer addS: ");
            for (i = 0; i < cSIZE; i++) {
                fprintf(stderr, "%02x ",(unsigned char)szSerBuffer[i]);
            }
            fprintf(stderr, "\naddS 0x%04x %i\n",addS,addS);
        }
        addC = addS;
        if (addC == aCESMemAdd) beg = TRUE;
        invFunc = -1;
        while (loop && nCnt > 0) {
            memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));
            szSerBuffer[cAddress] = yAddress;			/*set inverter address */
            szSerBuffer[cCommand] = invOp = opGetCEValue;
            szSerBuffer[cCEDailyAddH] = HIBYTE(addC);
            szSerBuffer[cCEDailyAddL] = LOBYTE(addC);
            szSerBuffer[cCEDailyEnd] = '\0';
            nCnt = Communicate(fdser, yAddress);
            cnt++;
            if (bVerbose) {
                fprintf(stderr, "%i 0x%02x%02x szSerBuffer addS: ",cnt,HIBYTE(addC),LOBYTE(addC));
                for (i = 0; i < cSIZE; i++) {
                    fprintf(stderr, "%02x ",(unsigned char)szSerBuffer[i]);
                }
                fprintf(stderr, "\n");
            }
            if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetCESent,TRUE)) {
                if (bVerbose) { fprintf(stderr, "addr 0x%02x%02x ",HIBYTE(addC),LOBYTE(addC)); }
                value1 = szCvrtShort(szSerBuffer);
                if (bVerbose) { fprintf(stderr, "addr 0x%02x%02x ",HIBYTE(addC+2),LOBYTE(addC+2)); }
                value2 = szCvrtShort(szSerBuffer+2);
                if (addC <= ((aCESMemAdd+(aCESMaxCnt*4))-4)) {
                    if (value1 == 0xffff) {
                        ffff = value1;
                        tBuf[0] = HIBYTE(value2);
                        tBuf[1] = LOBYTE(value2);
                        value2 = 0xffff;
                    } else if (value2 == 0xffff) {
                        ffff = value2;
                    } else if (ffff != 0) {
                        if (cvrtLong(tBuf) == 0) {
                            tBuf[0] = HIBYTE(value1);
                            tBuf[1] = LOBYTE(value1);
                            tBuf[2] = HIBYTE(value2);
                            tBuf[3] = LOBYTE(value2);
                            value1 = value2 = 0xffff;
                        } else {
                            tBuf[2] = HIBYTE(value1);
                            tBuf[3] = LOBYTE(value1);
                            value1 = 0xffff;
                        }
                        timeValLong = cvrtLong(tBuf) + TimeBase;
                        TS = *(localtime(&timeValLong));
                        gmtoff = getGMToffset(timeValLong);
                        timeValLong -= gmtoff;
                        tBuf[0] = tBuf[1] = tBuf[2] = tBuf[3] = '\0';
                        ffff = 0;
                        if (bVerbose) {
                            fprintf(stderr, "time change old: timeValLong %lu ",timeValLongLast);
                            fprintf(stderr, "new: timeValLong %lu gmtoff %li\n",timeValLong,gmtoff);
                            TS = *(localtime(&timeValLongLast));
                            strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
                            DT[sizeof(DT)-1] = '\0';
                            fprintf(stderr, "old: %17s",DT);
                            TS = *(localtime(&timeValLong));
                            strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
                            DT[sizeof(DT)-1] = '\0';
                            fprintf(stderr, " new: %17s\n",DT);
                        }
                        if (! tsync) {
                            TSnext = *(localtime(&timeValLong));
                            gmtoffTSnext = getGMToffset(timeValLong);
                            timeValLongLast = timeValLong;
                            TS = *(localtime(&timeValLongLast));
                            TS.tm_sec = TS.tm_min = TS.tm_hour = 0;
                            timeValLongLast = mktime(&TS);
                            TS = *(localtime(&timeValLongLast));
                            gmtoffTS = getGMToffset(timeValLongLast);
                            timeValLongLast += (gmtoffTS - gmtoffTSnext);
                            TS = *(localtime(&timeValLongLast));
                            if (TSnext.tm_year == TS.tm_year && TSnext.tm_mon == TS.tm_mon && TSnext.tm_mday == TS.tm_mday) tsync =TRUE;
                        }
                        if (tsync) {
                            while (timeValLongLast < timeValLong) {
                                TS = *(localtime(&timeValLongLast));
                                strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
                                DT[sizeof(DT)-1] = '\0';
                                fprintf(outfp, "\n%17s  %9.1f W",DT,0.0);
                                timeValLongLast += 10;
                            }
                            if (bVerbose && ! cr) { fprintf(outfp, "\n"); }
                        } else
                            tsync = TRUE;
                    }
                }
                if (bVerbose && addC == aCESMemAdd) {
                    fprintf(stderr, "addC %04x cnt = %i szSerBuffer addC: ",addC,cnt);
                    for (i = 0; i < cSIZE; i++) {
                        fprintf(stderr, "%02x ",(unsigned char)szSerBuffer[i]);
                    }
                    fprintf(stderr, "\n");
                }
                if (addC != aCESMemAdd &&  addC <= ((aCESMemAdd+(aCESMaxCnt*4))-4) && value1 != 0xffff) {
                    if (cr) fprintf(outfp, "\n");
                    TS = *(localtime(&timeValLong));
                    gmtoff = getGMToffset(timeValLong);
                    if (tsync) {
                        strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
                        DT[sizeof(DT)-1] = '\0';
                        if (qMultiplier <= 0)
                            fprintf(outfp, "%17s  %9.1f W",DT,(float)value1/10);
                        else
                            fprintf(outfp, "%17s  %9.1f W",DT,(float)value1*qMultiplier);
                    } else {
                        if (qMultiplier <= 0)
                            fprintf(outfp, "YYYYMMDD-%04i  %9.1f W",((cnt-1)*2)+1,(float)value1/10);
                        else
                            fprintf(outfp, "YYYYMMDD-%04i  %9.1f W",((cnt-1)*2)+1,(float)value1*qMultiplier);
                    }
                    if (bVerbose) fprintf(outfp, "\t0x%04x",value1);
                    if (tsync) {
                        timeValLong += 10;
                        timeValLongLast = timeValLong;
                    }
                    cr = TRUE;
                }
                if ((addC+2) <= ((aCESMemAdd+(aCESMaxCnt*4))-2) && value2 != 0xffff && ! (cnt == aCESMaxCnt && odd)) {
                    if (cr) fprintf(outfp, "\n");
                    TS = *(localtime(&timeValLong));
                    gmtoff = getGMToffset(timeValLong);
                    if (tsync) {
                        strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
                        DT[sizeof(DT)-1] = '\0';
                        if (qMultiplier <= 0)
                            fprintf(outfp, "%17s  %9.1f W",DT,(float)value2/10);
                        else
                            fprintf(outfp, "%17s  %9.1f W",DT,(float)value2*qMultiplier);
                    } else {
                        if (qMultiplier <= 0)
                            fprintf(outfp, "YYYYMMDD-%04i  %9.1f W",((cnt-1)*2)+2,(float)value2/10);
                        else
                            fprintf(outfp, "YYYYMMDD-%04i  %9.1f W",((cnt-1)*2)+2,(float)value2*qMultiplier);
                    }
                    if (bVerbose) fprintf(outfp, "\t0x%04x",value2);
                    if (tsync) {
                        timeValLong += 10;
                        timeValLongLast = timeValLong;
                    }
                    cr = TRUE;
                }
                addC += 4;
                if (cnt >= aCESMaxCnt || cnt >= yGetEnergySent || (beg && addC >= addS))
                    loop = FALSE;
                else {
                    if (addC > ((aCESMemAdd+(aCESMaxCnt*4))-2)) {	/* -2 in case start align was +1 */
                        if (beg)
                            loop = FALSE;
                        else {
                            if (bVerbose) { fprintf(stderr, "%i wrapping addC cur 0x%02x%02x ",cnt,HIBYTE(addC),LOBYTE(addC)); }
                            if (addC > ((aCESMaxCnt*4)+aCESMemAdd)) odd = TRUE;
                            addC = aCESMemAdd;
                            if (bVerbose) { fprintf(stderr, "new 0x%02x%02x %s\n",HIBYTE(addC),LOBYTE(addC),odd ? "odd" : "even"); }
                            beg = TRUE;
                        }
                    }
                }
            } else {
                return(-1);
            }
        }
    } else 
        return(-1);
    return(0);
}

/*--------------------------------------------------------------------------
    GetCEDaily ** Experimental **
----------------------------------------------------------------------------*/
int GetCEDaily(int fdser, int yAddress, int yGetEnergyDaily)
{
    int nCnt;
    int addC = 0;
    int loop = 1;
    int pCnt = 0;
    int i = 0, j = 0;
    time_t timeValLongBase = 0, timeValLongToday = 0, tVal = 0;
    long int gmtoff = 0;
    struct tm TS;
    char DT[18] = "";
    char YMD[9] = "";
    char DCEL[95] = "";
    unsigned long days = 0, lastDays = 0, begDay = 0, kwh = 0;
    BOOL looped = FALSE;
    int data[366][2];

    DT[0] = '\0';
    if (bVerbose)fprintf(stderr, "TimeBase %lu timeValLong %lu\n",(long)TimeBase,timeValLongBase);
    if ((timeValLongBase = GetInvTime(fdser, yAddress)) < 0) return(-1);
    TS = *(localtime(&timeValLongBase));
    gmtoff = getGMToffset(timeValLongBase);
    if (bVerbose) {
        strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
        DT[sizeof(DT)-1] = '\0';
        fprintf(stderr, "\nInverter Time: %17s      timeValLong %12lu gmtoff %7li TimeBase %12lu\n",DT,timeValLongBase,gmtoff,(long)TimeBase);
    }
    TS.tm_sec = TS.tm_min = TS.tm_hour = TS.tm_isdst = 0;
    timeValLongToday = mktime(&TS);

    timeValLongBase = time(NULL);
    TS = *(localtime(&timeValLongBase));
    TS.tm_sec = TS.tm_min = TS.tm_hour = TS.tm_mon = TS.tm_isdst = 0;
    TS.tm_sec = TS.tm_min = TS.tm_hour = TS.tm_isdst = 0;
    TS.tm_mday = 1;
    TS.tm_year = 100;
    timeValLongBase = mktime(&TS);
    timeValLongBase -= 86400;
    TS = *(localtime(&timeValLongBase));
    gmtoff = getGMToffset(timeValLongBase);
    DT[sizeof(DT)-1] = '\0';
    if (bVerbose) {
        strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
        DT[sizeof(DT)-1] = '\0';
        fprintf(stderr, "Timebase:      %17s  0.0 timeValLong %12lu gmtoff %7li TimeBase %12lu\n",DT,timeValLongBase,gmtoff,(long)TimeBase);
    }
    if (bVerbose) {
        TS = *(localtime(&timeValLongToday));
        strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
        DT[sizeof(DT)-1] = '\0';
        fprintf(stderr, "Todays Base:   %17s      timeValLong %12lu gmtoff %7li TimeBase %12lu\n",DT,timeValLongToday,gmtoff,(long)TimeBase);
    }
    if (bVerbose) {
        fprintf(stderr, "Multiplier:    %f\n", yMultiplierK);
    }

    YMD[0] = '\0';
    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));             /* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;			/*set inverter address */
    szSerBuffer[cCommand] = invOp = HIBYTE(opGetCEAdd); 
    szSerBuffer[cParam1] = invFunc = LOBYTE(opGetCEAdd);
    szSerBuffer[cParam1End] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetCEAdd,TRUE)) {
        addC = szCvrtShort(szSerBuffer);
        if (bVerbose)
            fprintf(stderr, "\nDCE Epoch:    Address: 0x%04x\n",(WORD)addC);
        else
            fprintf(outfp, "\n");
        invFunc = -1;
        while (loop && nCnt > 0) {
            if (!bVerbose) fprintf(stderr, "%c%c", star[(loop-1)%5], 0x0d);
            memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));
            szSerBuffer[cAddress] = yAddress;			/*set inverter address */
            szSerBuffer[cCommand] = invOp = opGetCEValue;
            szSerBuffer[cCEDailyAddH] = HIBYTE(addC);
            szSerBuffer[cCEDailyAddL] = LOBYTE(addC);
            szSerBuffer[cCEDailyEnd] = '\0';
            nCnt = Communicate(fdser, yAddress);
            if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetCEAdd,TRUE)) {
                days = szCvrtShort(szSerBuffer);
                kwh = szCvrtShort(szSerBuffer+2);
                if (bVerbose) sprintf(DCEL, "DCE Loop: %3d Address: 0x%04x Day: %3d Days: 0x%02x%02x Value: 0x%02x%02x  %5li = ",loop,addC,pCnt+1,(BYTE)szSerBuffer[aCEDailyDaysH],(BYTE)szSerBuffer[aCEDailyDaysL],(BYTE)szSerBuffer[aCEDailyValH],(BYTE)szSerBuffer[aCEDailyValL],days);
                if (kwh != 0xffff) {
                    tVal = timeValLongBase + ((days-1)*86400);
                    if (loop == 1) {
                        begDay = days;
                        if (tVal != timeValLongToday) {
                            if (bVerbose) {
                                TS = *(localtime(&timeValLongBase));
                                strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
                                DT[sizeof(DT)-1] = '\0';
                                fprintf(stderr, "\nAdjusting timebase by %li\nwas %12li  %s\n",timeValLongToday-tVal,timeValLongBase,DT);
                            }
                            timeValLongBase = timeValLongToday - ((days-1)*86400);
                            if (bVerbose) {
                                TS = *(localtime(&timeValLongBase));
                                strftime(DT,sizeof(DT),"%Y%m%d-%H:%M:%S",&TS);
                                DT[sizeof(DT)-1] = '\0';
                                fprintf(stderr, "is  %12li  %s\n\n",timeValLongBase,DT);
                            }
                            tVal = timeValLongBase + ((days-1)*86400);
                        }
                    }
                    if (bVerbose) {
                        TS = *(localtime(&tVal));
                        strftime(YMD,sizeof(YMD),"%Y%m%d",&TS);
                        YMD[sizeof(YMD)-1] = '\0';
                        fprintf(stderr, "%s%s %6.3f kWh",DCEL,YMD,(float)kwh/(1000.0/yMultiplierK));
                    }
                    if (days != lastDays && (days < begDay || lastDays == 0)) {
                        data[pCnt][0] = days;
                        data[pCnt][1] = kwh;
                        lastDays = days;
                        pCnt++;
                    } else 
                        if (bVerbose) fprintf(stderr, " out of order or duplicate\n");
                } else
                    if (bVerbose) fprintf(stderr, "%s???????? %6.3f kWh invalid",DCEL,(float)kwh/(1000.0/yMultiplierK));
                if (! looped && addC <= 0x4388) {
                    addC += (366*4);
                    looped = TRUE;
                    if (bVerbose) fprintf(stderr, " Move to end 0x%04x",addC);
                } else 
                    addC -= 4;
                if (bVerbose) fprintf(stderr, "\n");
                if (pCnt >= yGetEnergyDaily) 
                    loop = 0;
                else
                    loop++;
            } else 
                return(-1);
            if (loop > 367) loop = 0;
        }
        for (i = 0; i < pCnt; i++) {
            for (j = i; j < pCnt; j++) {
                if (data[i][0] < data[j][0] || (data[i][0] == data[j][0] && data[i][1] < data[j][1])) {
                    data[i][0] += data[j][0];
                    data[j][0] = data[i][0] - data[j][0];
                    data[i][0] -= data[j][0];
                    data[i][1] += data[j][1];
                    data[j][1] = data[i][1] - data[j][1];
                    data[i][1] -= data[j][1];
                }
            }
        }
        lastDays = -1;
        for (i = 0; i < pCnt; i++) {
            if (data[i][0] != lastDays) {
                tVal = timeValLongBase + ((data[i][0]-1)*86400);
                TS = *(localtime(&tVal));
                strftime(YMD,sizeof(YMD),"%Y%m%d",&TS);
                YMD[sizeof(YMD)-1] = '\0';
                fprintf(outfp, "%s %9.3f kWh\n",YMD,(float)data[i][1]/(1000.0/yMultiplierK));
                lastDays = data[i][0];
            }
        }
        return(0);
    } else if (bVerbose)
        fprintf(stderr, "\nDCE Address retreival problem\n");

    return(-1);
}

/*--------------------------------------------------------------------------
    GetCEC CMD 68
----------------------------------------------------------------------------*/
int GetCEC(int fdser, int yAddress, int yGetEnergyCen)
{
    float DAILY = 0.0, WEEKLY = 0.0, MONTHLY = 0.0, YEARLY = 0.0, NDAYS = 0.0, TOTAL = 0.0, PARTIAL = 0.0;
    int yTimeoutOrg;
    char Days[16];

    if ((DAILY = GetCECdata(fdser,yAddress,opGetCEC,CECpar1,_opGetCEC,-1)) == _ERROR_) return(-1);
    if ((WEEKLY = GetCECdata(fdser,yAddress,opGetCEC,CECpar2,_opGetCEC,-1)) == _ERROR_) return(-1);
    if ((MONTHLY = GetCECdata(fdser,yAddress,opGetCEC,CECpar3,_opGetCEC,-1)) == _ERROR_) return(-1);
    if ((YEARLY = GetCECdata(fdser,yAddress,opGetCEC,CECpar4,_opGetCEC,-1)) == _ERROR_) return(-1);
    yTimeoutOrg = yTimeout;
    yTimeout = 2000;
    if ((NDAYS = GetCECdata(fdser,yAddress,opGetCEC,CECpar5,_opGetCEC,yGetEnergyCen)) == _ERROR_) return(-1);
    yTimeout = yTimeoutOrg;
    if ((TOTAL = GetCECdata(fdser,yAddress,opGetCEC,CECpar6,_opGetCEC,-1)) == _ERROR_) return(-1);
    if ((PARTIAL = GetCECdata(fdser,yAddress,opGetCEC,CECpar7,_opGetCEC,-1)) == _ERROR_) return(-1);

    sprintf(Days,"%d %s",yGetEnergyCen,_CECpar5);

    if (bColumns) {
        fprintf(outfp, "%12.3f  %12.3f  %12.3f  %12.3f  %12.3f  %12.3f  %12.3f  ",DAILY/1000.0,WEEKLY/1000.0,MONTHLY/1000.0,YEARLY/1000.0,NDAYS/1000.0,TOTAL/1000.0,PARTIAL/1000.0);
        bColOutput = TRUE;
    }
    else
    {
        fprintf(outfp, "\n%-26s = %12.3f kWh",_CECpar1,DAILY/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %11.3f)",sCostType,(DAILY/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %12.3f kWh",_CECpar2,WEEKLY/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %11.3f)",sCostType,(WEEKLY/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %12.3f kWh",_CECpar3,MONTHLY/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %11.3f)",sCostType,(MONTHLY/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %12.3f kWh",_CECpar4,YEARLY/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %11.3f)",sCostType,(YEARLY/1000.0)*yCost);

//        fprintf(outfp, "\n%3d %-22s = %12.3f kWh",yGetEnergyCen,_CECpar5,NDAYS/1000.0);
        fprintf(outfp, "\n%-26s = %12.3f kWh",Days,NDAYS/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %11.3f)",sCostType,(NDAYS/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %12.3f kWh",_CECpar6,TOTAL/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %11.3f)",sCostType,(TOTAL/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %12.3f kWh",_CECpar7,PARTIAL/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %11.3f)",sCostType,(PARTIAL/1000.0)*yCost);

        fprintf(outfp, "\n");
    }

    return(0);

}


/*--------------------------------------------------------------------------
    GetCECdata
----------------------------------------------------------------------------*/
float GetCECdata(int fdser, int yAddress, int opcode, int param, char description[], int yGetEnergyCen)
{
    int nCnt;
    float paramValFloat;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));	/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;			/* set inverter address */
    szSerBuffer[cCommand] = invOp = opcode;		/* set Measure request to the Energy opcode */
    szSerBuffer[cParam1] = invFunc = param;
    if (yGetEnergyCen > 0) {
        szSerBuffer[cParam2] = HIBYTE((WORD)yGetEnergyCen);
        szSerBuffer[cParam3] = LOBYTE((WORD)yGetEnergyCen);
    } else {
        szSerBuffer[cParam2] = szSerBuffer[cParam3] = 0;
    }
    szSerBuffer[cParam4] = 0;
    szSerBuffer[cParam4End] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],description,TRUE)) {
        paramValFloat = szCvrtFloat(szSerBuffer);
        if (bVerbose) fprintf(stderr, "value     %12.6f\n",paramValFloat);
        return(paramValFloat);
    }

    if (bVerbose) fprintf(stderr, "_ERROR_ (%i %i)\n",invOp,invFunc);
    return(_ERROR_);

}


/*--------------------------------------------------------------------------
    GetCE CMD 78
----------------------------------------------------------------------------*/
int GetCE(int fdser, int yAddress)
{
    unsigned long DAILY, WEEKLY, LAST7DAYS, MONTHLY, YEARLY, TOTAL, PARTIAL;

    if (bVerbose) fprintf(stderr, "\nAttempting to get Partial Energy value ");
    if ((PARTIAL = GetCEdata(fdser,yAddress,opGetCE,CEpar6,_opGetCE)) == _ERROR_) return(-1);
    if ((DAILY = GetCEdata(fdser,yAddress,opGetCE,CEpar0,_opGetCE)) == _ERROR_) return(-1);
    if ((WEEKLY = GetCEdata(fdser,yAddress,opGetCE,CEpar1,_opGetCE)) == _ERROR_) return(-1);
    LAST7DAYS = 0.0;    /* do this for now since this has been dropped in the v2.3 Communications Protocol doc  */
                        /* placeholder for the -c option (colums)for now                                                */
    if ((MONTHLY = GetCEdata(fdser,yAddress,opGetCE,CEpar3,_opGetCE)) == _ERROR_) return(-1);
    if ((YEARLY = GetCEdata(fdser,yAddress,opGetCE,CEpar4,_opGetCE)) == _ERROR_) return(-1);
    if ((TOTAL = GetCEdata(fdser,yAddress,opGetCE,CEpar5,_opGetCE)) == _ERROR_) return(-1);

    if (bColumns) {
        /* must continue to output LAST7DAYS as a placeholder even though the inverter(s) may no longer report it */
        fprintf(outfp, "%11.3f  %11.3f  %11.3f  %11.3f  %11.3f  %11.3f  %11.3f  ",DAILY/1000.0,WEEKLY/1000.0,LAST7DAYS/1000.0,MONTHLY/1000.0,YEARLY/1000.0,TOTAL/1000.0,PARTIAL/1000.0);
        bColOutput = TRUE;
    }
    else
    {
        fprintf(outfp, "\n%-26s = %11.3f kWh",_CEpar0,DAILY/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %10.3f)",sCostType,(DAILY/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %11.3f kWh",_CEpar1,WEEKLY/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %10.3f)",sCostType,(WEEKLY/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %11.3f kWh",_CEpar3,MONTHLY/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %10.3f)",sCostType,(MONTHLY/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %11.3f kWh",_CEpar4,YEARLY/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %10.3f)",sCostType,(YEARLY/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %11.3f kWh",_CEpar5,TOTAL/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %10.3f)",sCostType,(TOTAL/1000.0)*yCost);

        fprintf(outfp, "\n%-26s = %11.3f kWh",_CEpar6,PARTIAL/1000.0);
        if (yCost > 0) fprintf(outfp, "\t(%s %10.3f)",sCostType,(PARTIAL/1000.0)*yCost);

        fprintf(outfp, "\n");
    }

    return(0);
}


/*--------------------------------------------------------------------------
    GetCEdata
----------------------------------------------------------------------------*/
unsigned long GetCEdata(int fdser, int yAddress, int opcode, int param, char description[])
{
    int nCnt;
    unsigned long paramValLong;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opcode;		/* set Measure request to the Energy opcode */
    szSerBuffer[cParam1] = invFunc = param;
    if (yCentral < 0) 
        szSerBuffer[cParam1End] = '\0';
    else {
        szSerBuffer[cParam1] = yCentral;
        szSerBuffer[cParam2End] = '\0';
    }
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],description,TRUE)) {
        paramValLong = szCvrtLong(szSerBuffer);
        if (bVerbose) fprintf(stderr, "value        %12lu\n",(unsigned long)paramValLong);
        return((int)paramValLong);
    }

    if (bVerbose) fprintf(stderr, "_ERROR_ (%i %i)\n",invOp,invFunc);
    return(_ERROR_);
}


/*--------------------------------------------------------------------------
    GetDSP CMD 59
----------------------------------------------------------------------------*/
int GetDSP(int fdser, int yAddress)
{
    float GVR=0.0, GCR=0.0, GPR=0.0, GPRC=0.0, FRQ=0.0, INVeff=0.0, INVeffC=0.0, INVtemp=0.0, ENVtemp=0.0, PVpwr = 0.0;
    float STR1V=0.0, STR1C=0.0, STR2V=0.0, STR2C=0.0;
 
    STR1V = STR1C = STR2V = STR2C = -1.0;

    if ((FRQ = GetDSPdata(fdser,yAddress,opGetDSP,ToM4,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    if ((GVR = GetDSPdata(fdser,yAddress,opGetDSP,ToM1,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    if ((GCR = GetDSPdata(fdser,yAddress,opGetDSP,ToM2,_opGetDSP,TRUE)) >= _ERROR_) return(-1);	// < -1000000.0
    if ((GPR = GetDSPdata(fdser,yAddress,opGetDSP,ToM3,_opGetDSP,TRUE)) >= _ERROR_) return(-1);	// < -1000000.0

    if (yGetDSP == 0 || yGetDSP == 1) {
        if ((STR1V = GetDSPdata(fdser,yAddress,opGetDSP,ToM23,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((STR1C = GetDSPdata(fdser,yAddress,opGetDSP,ToM25,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    }
    if (yCentral < 0 && (yGetDSP == 0 || yGetDSP == 2)) {
        if ((STR2V = GetDSPdata(fdser,yAddress,opGetDSP,ToM26,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((STR2C = GetDSPdata(fdser,yAddress,opGetDSP,ToM27,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    }

    if (yCentral < 0) {
        if ((INVtemp = GetDSPdata(fdser,yAddress,opGetDSP,ToM21,_opGetDSP,TRUE)) < -100) return(-1);
        if ((ENVtemp = GetDSPdata(fdser,yAddress,opGetDSP,ToM22,_opGetDSP,TRUE)) < -100) return(-1);
    }

    if (bCalcGridPwr) GPRC = GVR * GCR;

    if (STR1V >= 0.0 && STR1C >= 0.0) PVpwr = STR1V*STR1C;
    if (STR2V >= 0.0 && STR2C >= 0.0) PVpwr += STR2V*STR2C;
    if (PVpwr > 0) {
        INVeff = (GPR/PVpwr)*100;
        INVeffC = (GPRC/PVpwr)*100;
    }

    if (bColumns) {
        if (yGetDSP == 0 || yGetDSP == 1)
            fprintf(outfp, "%13.6f  %13.6f  %13.6f  ",STR1V,STR1C,STR1V*STR1C);
        else
            fprintf(outfp, "%11s  %11s  %11s  ","n/a","n/a","n/a");
        if (yGetDSP == 0 || yGetDSP == 2)
            fprintf(outfp, "%13.6f  %13.6f  %13.6f  ",STR2V,STR2C,STR2V*STR2C);
        else
            fprintf(outfp, "%11s  %11s  %11s  ","n/a","n/a","n/a");
        fprintf(outfp, "%13.6f  %13.6f  %13.6f  %13.6f  ",GVR,GCR,GPR,FRQ);
        if (yGetDSP == 0 || INVeff < 101.0)
            if (INVeff >= 0.0)
                fprintf(outfp, "%13.6f  ",INVeff);
            else
                fprintf(outfp, "%13.6f  ",0.0);
        else
            fprintf(outfp, "%11s  ","OverRange");
        fprintf(outfp, "%13.6f  %13.6f  ",INVtemp,ENVtemp);
        bColOutput = TRUE;
    } else {
        if (yGetDSP == 0 || yGetDSP == 1) {
            fprintf(outfp, "\n%-27s = %13.6f V\n",_ToM23,STR1V);
            fprintf(outfp, "%-27s = %13.6f A\n",_ToM25,STR1C);
            fprintf(outfp, "%-27s = %13.6f W\n",_Str1P,STR1V*STR1C);
        } else {
            fprintf(outfp, "\n%-27s = %11s V\n",_ToM23,"n/a");
            fprintf(outfp, "%-27s = %11s A\n",_ToM25,"n/a");
            fprintf(outfp, "%-27s = %11s W\n",_Str1P,"n/a");
        }

	if (yCentral < 0) {
            if (yGetDSP == 0 || yGetDSP == 2) {
                fprintf(outfp, "\n%-27s = %13.6f V\n",_ToM26,STR2V);
                fprintf(outfp, "%-27s = %13.6f A\n",_ToM27,STR2C);
                fprintf(outfp, "%-27s = %13.6f W\n",_Str2P,STR2V*STR2C);
            } else {
                fprintf(outfp, "\n%-27s = %11s V\n",_ToM26,"n/a");
                fprintf(outfp, "%-27s = %11s A\n",_ToM27,"n/a");
                fprintf(outfp, "%-27s = %11s W\n",_Str2P,"n/a");
            }
        }

        fprintf(outfp, "\n%-27s = %13.6f V\n",_ToM1,GVR);
        fprintf(outfp, "%-27s = %13.6f A\n",_ToM2,GCR);
        fprintf(outfp, "%-27s = %13.6f W\n",_ToM3,GPR);
        if (bCalcGridPwr) fprintf(outfp, "%-27s = %13.6f W\n",_ToM3C,GPRC);
        fprintf(outfp, "%-27s = %13.6f Hz.\n",_ToM4,FRQ);

        if (yGetDSP == 0 || INVeff < 101.0)
            if (INVeff >= 0.0)
                fprintf(outfp, "\n%-27s = %13.1f %s",_DcAcEff,INVeff,"%");
            else
                fprintf(outfp, "\n%-27s = %13.1f %s",_DcAcEff,0.0,"%");
        else
            fprintf(outfp, "\n%-27s = %13s  ",_DcAcEff,"over range");
        if (bCalcGridPwr) {
            if (yGetDSP == 0 || INVeffC < 101.0)
                fprintf(outfp, " (Using Grid Power Reading)\n%-27s = %13.1f %s (Using Grid Power Calculated)",_DcAcEff,INVeffC,"%");
            else
                fprintf(outfp, " (Using Grid Power Reading)\n%-27s = %13s   (Using Grid Power Calculated)",_DcAcEff,"over range");
        }
        fprintf(outfp, "\n%-27s = %13.6f C\n",_ToM21,INVtemp);
        fprintf(outfp, "%-27s = %13.6f C\n",_ToM22,ENVtemp);
    }

    return(0);
}


void PrintBuffer()
{
    int i;
     for (i = 0; i < cSIZE; i++) {
        fprintf(stderr, "%02x ",(unsigned char)szSerBuffer[i]);
    }
    fprintf(stderr, "\n");
}

/*--------------------------------------------------------------------------
    GetDSPExtended
----------------------------------------------------------------------------*/
int GetDSPExtended(int fdser, int yAddress)
{
    float VB,ILD,ILI,P1,P2,GV,GF,IR,VBD,AGV,VBM,PP,PPT,GVn,WGF,VBp,VBm,ST,AT,HT,T1,T2,T3,F1,F2,F3,F4,F5,PSL,RRB,VPm;
    char note[3];

    VB = ILD = ILI = P1 = P2 = GV = GF = IR = VBD = AGV = VBM = PP = PPT = GVn = WGF = 0.0;
    VBp = VBm = ST = AT = HT = T1 = T2 = T3 = F1 = F2 = F3 = F4 = F5 = PSL = RRB = VPm = 0.0;

    if ((VB = GetDSPdata(fdser,yAddress,opGetDSP,ToM5,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    if ((IR = GetDSPdata(fdser,yAddress,opGetDSP,ToM30,_opGetDSP,FALSE)) >= _ERROR_) return(-1);
    if ((P1 = GetDSPdata(fdser,yAddress,opGetDSP,ToM8,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    if ((PPT = GetDSPdata(fdser,yAddress,opGetDSP,ToM35,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    if (yCentral < 0) {
        if ((ILD = GetDSPdata(fdser,yAddress,opGetDSP,ToM6,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((ILI = GetDSPdata(fdser,yAddress,opGetDSP,ToM7,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((P2 = GetDSPdata(fdser,yAddress,opGetDSP,ToM9,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((GV = GetDSPdata(fdser,yAddress,opGetDSP,ToM28,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((GF = GetDSPdata(fdser,yAddress,opGetDSP,ToM29,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((VBD = GetDSPdata(fdser,yAddress,opGetDSP,ToM31,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((AGV = GetDSPdata(fdser,yAddress,opGetDSP,ToM32,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((VBM = GetDSPdata(fdser,yAddress,opGetDSP,ToM33,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((PP = GetDSPdata(fdser,yAddress,opGetDSP,ToM34,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((GVn = GetDSPdata(fdser,yAddress,opGetDSP,ToM36,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((WGF = GetDSPdata(fdser,yAddress,opGetDSP,ToM37,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    } else {
        if ((VBp = GetDSPdata(fdser,yAddress,opGetDSP,ToM45,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((VBm = GetDSPdata(fdser,yAddress,opGetDSP,ToM46,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((ST = GetDSPdata(fdser,yAddress,opGetDSP,ToM47,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((AT = GetDSPdata(fdser,yAddress,opGetDSP,ToM48,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((HT = GetDSPdata(fdser,yAddress,opGetDSP,ToM49,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((T1 = GetDSPdata(fdser,yAddress,opGetDSP,ToM50,_opGetDSP,FALSE)) >= _ERROR_) return(-1);
        if ((T2 = GetDSPdata(fdser,yAddress,opGetDSP,ToM51,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((T3 = GetDSPdata(fdser,yAddress,opGetDSP,ToM52,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((F1 = GetDSPdata(fdser,yAddress,opGetDSP,ToM53,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((F2 = GetDSPdata(fdser,yAddress,opGetDSP,ToM54,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((F3 = GetDSPdata(fdser,yAddress,opGetDSP,ToM55,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((F4 = GetDSPdata(fdser,yAddress,opGetDSP,ToM56,_opGetDSP,FALSE)) >= _ERROR_) return(-1);
        if ((F5 = GetDSPdata(fdser,yAddress,opGetDSP,ToM57,_opGetDSP,FALSE)) >= _ERROR_) return(-1);
        if ((PSL = GetDSPdata(fdser,yAddress,opGetDSP,ToM58,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((RRB = GetDSPdata(fdser,yAddress,opGetDSP,ToM59,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
        if ((VPm = GetDSPdata(fdser,yAddress,opGetDSP,ToM60,_opGetDSP,TRUE)) >= _ERROR_) return(-1);
    }

    note[0] = '\0';
    if (bColumns) {
//        if (AGV == VBD) strcpy(note,"*\0");
        fprintf(outfp, "%11.6f  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f  %11.6f  %11.6f  %11.6f  %11.6f  %11.6f%s  %11.6f%s  %11.6f  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f  %11.6f  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  ",VB,VBM,note,VBp,note,VBm,note,VBD,ILD,ILI,IR,GV,AGV,note,GVn,note,GF,PP,note,PPT,note,ST,note,AT,note,HT,note,T1,note,T2,note,T3,note,F1,note,F2,note,F3,note,F4,note,F5,note,P1,P2,PSL,note,RRB,note,VPm,note,WGF,note);
        bColOutput = TRUE;
    }
    else
    {
        fprintf(outfp, "\nExtended DSP Reporting\n");
        fprintf(outfp, "%-31s = %11.6f V\n",_ToM5,VB);
        fprintf(outfp, "%-31s = %11.6f Mohm\n",_ToM30,IR);
        fprintf(outfp, "%-31s = %11.6f W  %s\n",_ToM35,PPT,note);
        fprintf(outfp, "%-31s = %11.6f W\n",_ToM8,P1);
        if (yCentral < 0) {
            if (AGV == VBD) strcpy(note,"*\0");
            fprintf(outfp, "%-31s = %11.6f W\n",_ToM9,P2);
            fprintf(outfp, "\n%-31s = %11.6f V  %s\n",_ToM33,VBM,note);
            fprintf(outfp, "%-31s = %11.6f V\n",_ToM31,VBD);
            fprintf(outfp, "\n%-31s = %11.6f A\n",_ToM6,ILD);
            fprintf(outfp, "%-31s = %11.6f A\n",_ToM7,ILI);
            fprintf(outfp, "\n%-31s = %11.6f V\n",_ToM28,GV);
            fprintf(outfp, "%-31s = %11.6f V  %s\n",_ToM32,AGV,note);
            fprintf(outfp, "%-31s = %11.6f V  %s\n",_ToM36,GVn,note);
            fprintf(outfp, "%-31s = %11.6f Hz\n",_ToM29,GF);
            fprintf(outfp, "\n%-31s = %11.6f W  %s\n",_ToM34,PP,note);
            fprintf(outfp, "%-31s = %11.6f Hz %s\n",_ToM37,WGF,note);
            if (AGV == VBD) fprintf(outfp, "(Note: * = May not be in this Inverter's firmware)\n");
        } else {
            fprintf(outfp, "\n%-31s = %11.6f V  %s\n",_ToM45,VBp,note);
            fprintf(outfp, "%-31s = %11.6f V  %s\n",_ToM46,VBm,note);
            fprintf(outfp, "\n%-31s = %11.6f C  %s\n",_ToM47,ST,note);
            fprintf(outfp, "%-31s = %11.6f C  %s\n",_ToM48,AT,note);
            fprintf(outfp, "%-31s = %11.6f C  %s\n",_ToM49,HT,note);
            fprintf(outfp, "%-31s = %11.6f C  %s\n",_ToM50,T1,note);
            fprintf(outfp, "%-31s = %11.6f C  %s\n",_ToM51,T2,note);
            fprintf(outfp, "%-31s = %11.6f C  %s\n",_ToM52,T3,note);
            fprintf(outfp, "\n%-31s = %11.6f RPM%s\n",_ToM53,F1,note);
            fprintf(outfp, "%-31s = %11.6f RPM%s\n",_ToM54,F2,note);
            fprintf(outfp, "%-31s = %11.6f RPM%s\n",_ToM55,F3,note);
            fprintf(outfp, "%-31s = %11.6f RPM%s\n",_ToM56,F4,note);
            fprintf(outfp, "%-31s = %11.6f RPM%s\n",_ToM57,F5,note);
            fprintf(outfp, "\n%-31s = %11.6f W  %s\n",_ToM58,PSL,note);
            fprintf(outfp, "%-31s = %11.6f V  %s\n",_ToM59,RRB,note);
            fprintf(outfp, "%-31s = %11.6f V  %s\n",_ToM60,VPm,note);
        }
    }

    return(0);
}

/*--------------------------------------------------------------------------
    GetDSP3Phase
----------------------------------------------------------------------------*/
int GetDSP3Phase(int fdser, int yAddress)
{
    float VBD,GVPn,GVPr,GVPs,GVPt,GCPr,GCPs,GCPt,FRQPr,FRQPs,FRQPt;
    BOOL GVPnImp = TRUE;
    char note[3];

    VBD = GVPn = GVPr = GVPs = GVPt = GCPr = GCPs = GCPt = FRQPr = FRQPs = FRQPt = 0.0;

    if (yCentral < 0) {
        if ((VBD = GetDSPdata(fdser,yAddress,opGetDSP,ToM31,_opGetDSP,TRUE)) < 0) return(-1);
    }
    if ((GVPn = GetDSPdata(fdser,yAddress,opGetDSP,ToM38,_opGetDSP,FALSE)) < 0) return(-1);
    GVPnImp = ! CmdNotImp;
    if ((GVPr = GetDSPdata(fdser,yAddress,opGetDSP,ToM61,_opGetDSP,TRUE)) < 0) return(-1);
    if ((GVPs = GetDSPdata(fdser,yAddress,opGetDSP,ToM62,_opGetDSP,TRUE)) < 0) return(-1);
    if ((GVPt = GetDSPdata(fdser,yAddress,opGetDSP,ToM63,_opGetDSP,TRUE)) < 0) return(-1);
    if ((GCPr = GetDSPdata(fdser,yAddress,opGetDSP,ToM39,_opGetDSP,TRUE)) < 0) return(-1);
    if ((GCPs = GetDSPdata(fdser,yAddress,opGetDSP,ToM40,_opGetDSP,TRUE)) < 0) return(-1);
    if ((GCPt = GetDSPdata(fdser,yAddress,opGetDSP,ToM41,_opGetDSP,TRUE)) < 0) return(-1);
    if ((FRQPr = GetDSPdata(fdser,yAddress,opGetDSP,ToM42,_opGetDSP,TRUE)) < 0) return(-1);
    if ((FRQPs = GetDSPdata(fdser,yAddress,opGetDSP,ToM43,_opGetDSP,TRUE)) < 0) return(-1);
    if ((FRQPt = GetDSPdata(fdser,yAddress,opGetDSP,ToM44,_opGetDSP,TRUE)) < 0) return(-1);

    note[0] = '\0';
    if (bColumns) {
        if (yCentral < 0 && GVPn == VBD) strcpy(note,"*\0");
        fprintf(outfp, "%11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  %11.6f%s  ",GVPn,note,GVPr,note,GVPs,note,GVPt,note,GCPr,note,GCPs,note,GCPt,note,FRQPr,note,FRQPs,note,FRQPt,note);
        bColOutput = TRUE;
    }
    else
    {
        if (yCentral < 0 && GVPn == VBD) strcpy(note," *\0");
        fprintf(outfp, "\n3-Phase DSP Reporting\n");
        if (GVPnImp) fprintf(outfp, "%-26s = %11.6f V %s\n",_ToM38,GVPn,note);
        fprintf(outfp, "%-26s = %11.6f V %s\n",_ToM61,GVPr,note);
        fprintf(outfp, "%-26s = %11.6f V %s\n",_ToM62,GVPs,note);
        fprintf(outfp, "%-26s = %11.6f V %s\n",_ToM63,GVPt,note);
        fprintf(outfp, "\n%-26s = %11.6f A %s\n",_ToM39,GCPr,note);
        fprintf(outfp, "%-26s = %11.6f A %s\n",_ToM40,GCPs,note);
        fprintf(outfp, "%-26s = %11.6f A %s\n",_ToM41,GCPt,note);
        fprintf(outfp, "\n%-26s = %11.6f Hz%s\n",_ToM42,FRQPr,note);
        fprintf(outfp, "%-26s = %11.6f Hz%s\n",_ToM43,FRQPs,note);
        fprintf(outfp, "%-26s = %11.6f Hz%s\n",_ToM44,FRQPt,note);
        if (yCentral < 0 && GVPn == VBD) fprintf(outfp, "(Note: * = May not be in this Inverter's firmware)\n");
    }

    return(0);
}

/*--------------------------------------------------------------------------
    GetDSPdata
----------------------------------------------------------------------------*/
float GetDSPdata(int fdser, int yAddress, int opcode, int param, char description[], BOOL TScheck)
{
    int nCnt;
    float paramValFloat = 0.0;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opcode;		/* set Measure request to the DSP opcode */
    szSerBuffer[cParam1] = invFunc = param;
    szSerBuffer[cParam1End] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && (! TScheck || TransState((int)szSerBuffer[aState],description,TScheck))) {
        if (! CmdNotImp)  paramValFloat = szCvrtFloat(szSerBuffer);
        if (bVerbose) fprintf(stderr, "value     %12.6f\n",paramValFloat);
        return(paramValFloat);
    }

    if (bVerbose) fprintf(stderr, "_ERROR_ (%i %i)\n",invOp,invFunc);
    return(_ERROR_);
}


/*--------------------------------------------------------------------------
    GetJoules
----------------------------------------------------------------------------*/
int GetJoules(int fdser, int yAddress)
{
    int nCnt;
    unsigned long Joules;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetEnergy10Sec;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetEnergy10Sec,TRUE)) {
        Joules = szCvrtShort(szSerBuffer);
        if (bVerbose) fprintf(stderr, "Joules       %12lu\n",(unsigned long)Joules);
        fprintf(outfp, "\nEnergy in the last 10 seconds (Joules) : %lu\n",(unsigned long)Joules);
        return(0);
   }
    return(-1);
}

/*--------------------------------------------------------------------------
    GetInvTime
----------------------------------------------------------------------------*/
time_t GetInvTime(int fdser, int yAddress)
{
    int nCnt = 0;
    time_t timeValLong = 0;
    long int gmtoff = 0;
    long int gmtoffTB = 0;
    struct tm tim;
    char curTime[24];

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));	/*clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;			/*set inverter address */
    szSerBuffer[cCommand] = invOp = opGetTime;
    szSerBuffer[cCommandEnd] = '\0';
    nCnt = Communicate(fdser, yAddress);

    if (nCnt > 0) {
        if (! TransState((int)szSerBuffer[aState],_opGetTime,TRUE)) return(-1);
        timeValLong = time(NULL);
        tim = *(localtime(&timeValLong));
        gmtoff = getGMToffset(timeValLong);
        timeValLong = (time_t)TimeBase;
        tim = *(localtime(&timeValLong));
        gmtoffTB = getGMToffset(timeValLong);
        if (bVerbose) {
            timeValLong = (time_t)TimeBase - gmtoffTB;
            fprintf(stderr, "\nTimeBase     %12lu\n",(time_t)TimeBase);
            fprintf(stderr, "gmtoffTB     %12li\n",gmtoffTB);
            tim = *(localtime(&timeValLong));
            strftime(curTime,sizeof(curTime),"%d-%b-%Y %H:%M:%S",&tim);
            curTime[sizeof(curTime)-1] = '\0';
            fprintf(stderr, "Base Inverter date/time: %s\n",curTime);
        }
        timeValLong = (time_t)szCvrtLong(szSerBuffer) & 0xffffffff;
        if (bVerbose) {
            fprintf(stderr, "timeValLong  %12lu\n",(time_t)timeValLong);
            fprintf(stderr, "gmtoff       %12li\n",gmtoff);
        }
        timeValLong += (time_t)TimeBase;
        timeValLong -= gmtoff;
        if (bVerbose) fprintf(stderr, "timeValLong  %12lu\n",(time_t)timeValLong);
    } else {
        if (bVerbose) fprintf(stderr, "GetInvTime no response nCnt  %i\n",nCnt);
        return(-1);
    }

    return(timeValLong);
}

/*--------------------------------------------------------------------------
    GetTime
----------------------------------------------------------------------------*/
int GetTime(int fdser, int yAddress)
{
    time_t timeValLong;
    struct tm tim;
    char curTime[24];
    char fromInv[10] = "Inverter \0";

    if (bGetInvTime) {
        if ((timeValLong = GetInvTime(fdser, yAddress)) < 0) return(-1); 
        if (bVerbose) fprintf(stderr, "GetTime: timeValLong  %12lu\n",(time_t)timeValLong);
    }

    if (bGetLocTime) {
        timeValLong = time(NULL);
        fromInv[0] = '\0';
    }

    if (timeValLong != 0 || bGetLocTime) {
        tim = *(localtime(&timeValLong));
        if (!bColumns || (yGetDSP < 0 && !bGetEnergy)) {
            fprintf(outfp, "Current %sdate/time: ",fromInv);
            strftime(curTime,sizeof(curTime),"%d-%b-%Y %H:%M:%S",&tim);
            if (bVerbose && !bGetLocTime) fprintf(stderr, "\n");
        } else {
            strftime(curTime,sizeof(curTime),"%Y%m%d-%H:%M:%S",&tim);
            bColOutput = TRUE;
        }
        curTime[sizeof(curTime)-1] = '\0';
        fprintf(outfp, "%s",curTime);
        return(0);
   }
    return(-1);
}


/*--------------------------------------------------------------------------
    CheckSetTime
    Check if the time on the inverter differs fro the computers time and 
    modify it as requested
----------------------------------------------------------------------------*/
int CheckSetTime(int fdser, int yAddress)
{

    time_t timeValLongInv;
    time_t timeValLongCur;
    time_t timeValLongMN;
    struct tm TS;
    long int gmtoffCur = 0;
    long int gmtoffInv = 0;
    long int gmtoffMN = 0;
    char DateTime[24];
    BOOL ChangeTime = FALSE;
    int rc = 0;

    if ((timeValLongInv = GetInvTime(fdser, yAddress)) < 0) return(-1);
    timeValLongCur = timeValLongMN = time(NULL);

    TS = *(localtime(&timeValLongCur));
    gmtoffCur = getGMToffset(timeValLongCur);
    if (bVerbose) {
        fprintf(stderr, "\ntimeValLongCur %lu timeValLongInv %li diff %li\n",timeValLongCur,timeValLongInv,timeValLongInv-timeValLongCur);
        strftime(DateTime,sizeof(DateTime),"%d-%b-%Y %H:%M:%S",&TS);
        DateTime[sizeof(DateTime)-1] = '\0';
        fprintf(stderr, "Current Time:  %s gmtoff: %li\n",DateTime,gmtoffCur);
        TS = *(localtime(&timeValLongInv));
        gmtoffInv = getGMToffset(timeValLongInv);
        strftime(DateTime,sizeof(DateTime),"%d-%b-%Y %H:%M:%S",&TS);
        fprintf(stderr, "Inverter Time: %s gmtoff: %li",DateTime,gmtoffInv);
        DateTime[sizeof(DateTime)-1] = '\0';
    }
    if (yCheckSetTime == 0) {
        TS = *(localtime(&timeValLongMN));
        TS.tm_sec = TS.tm_min = TS.tm_hour = 0;
        timeValLongMN = mktime(&TS);
        TS = *(localtime(&timeValLongMN));
        gmtoffMN = getGMToffset(timeValLongMN);
        timeValLongMN += (gmtoffCur - gmtoffMN);
        TS = *(localtime(&timeValLongMN));
        gmtoffMN = getGMToffset(timeValLongMN);
        if (gmtoffCur != gmtoffMN && abs(timeValLongCur-timeValLongInv) >= 900) ChangeTime = TRUE;
        if (bVerbose) {
            strftime(DateTime,sizeof(DateTime),"%d-%b-%Y %H:%M:%S",&TS);
            DateTime[sizeof(DateTime)-1] = '\0';
            fprintf(stderr, "\nYesterday:     %s gmtoff: %li",DateTime,gmtoffMN);
        }
    } else 
        if (abs(timeValLongInv-timeValLongCur) >= yCheckSetTime) ChangeTime = TRUE;
    if (bVerbose) fprintf(stderr, " ChangeTime: %s\n",ChangeTime ? "TRUE" : "FALSE");

    if (ChangeTime) rc = SetTime(fdser,yAddress,TRUE);

    return(rc);
}


/*--------------------------------------------------------------------------
    SetTime
----------------------------------------------------------------------------*/
int SetTime(int fdser, int yAddress, BOOL force)
{
    int nCnt = 0;
    int rc;
    int yTimeoutOrg;
    time_t timeValLong, timeValLongInv;
    struct tm tim;
    long int gmtoff = 0;
    char strTime[24];
    char answer;

    if (bVerbose && force) fprintf(stderr, "Force setting time\n");
    if (! force) {
        printf("\n**** WARNING *****   ***** WARNING *****   ***** WARNING ****\n");
        printf("Setting the Date and Time has been known to clear all History\n");
        printf("   Except \"Total Energy\" but no guarantees that it won't\n");
        printf("           (May not be enabled on all models)\n");
        printf("\nAre you sure your want to proceed? y/[n] : ");
        rc = scanf("%c", &answer);
        if (rc == EOF || (answer != 'y' && answer != 'Y')) return(1);
    }
 
    timeValLong = time(NULL);
    tim = *(localtime(&timeValLong));
    gmtoff = getGMToffset(timeValLong);
    if (bVerbose) {
	fprintf(stderr, "\ntimeValLong        %12lu\n",(time_t)timeValLong);
        strftime(strTime,sizeof(strTime),"%d-%b-%Y %H:%M:%S",&tim);
        strTime[sizeof(strTime)-1] = '\0';
        fprintf(stderr, "setting time to %s DST %i\n",strTime,tim.tm_isdst);
    }

    /* adjust time to Aurora's time base */
    timeValLong -= (long)TimeBase;
    timeValLong += gmtoff;
    /* adjust by 1 due to latency in setting the time */
    timeValLong++;
    if (bVerbose) fprintf(stderr, "timeValLong        %12lu\n",(time_t)timeValLong);

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opSetTime;
    szSerBuffer[cParam1] = (timeValLong >> 24) & 0xff;
    szSerBuffer[cParam2] = (timeValLong >> 16) & 0xff;
    szSerBuffer[cParam3] = (timeValLong >> 8) & 0xff;
    szSerBuffer[cParam4] = timeValLong & 0xff;
    szSerBuffer[cCommandEnd+4] = '\0';
    yTimeoutOrg = yTimeout;
    if (yTimeout == 0) yTimeout = 1000;
    nCnt = Communicate(fdser, yAddress);
    yTimeout = yTimeoutOrg;
    if (nCnt > 0 && ! TransState((int)szSerBuffer[aState],_opSetTime,TRUE)) return(-1);
    if (nCnt <= 0 || bVerbose) {
        timeValLong = time(NULL);
        if ((timeValLongInv = GetInvTime(fdser, yAddress)) < 0) return(-1);
        tim = *(localtime(&timeValLong));
        gmtoff = getGMToffset(timeValLong);
        if (bVerbose) {
            tim = *(localtime(&timeValLong));
            strftime(strTime,sizeof(strTime),"%d-%b-%Y %H:%M:%S",&tim);
            strTime[sizeof(strTime)-1] = '\0';
            fprintf(stderr, "\ntimeValLong     %12lu\t%s\n",(time_t)timeValLong,strTime);
            tim = *(localtime(&timeValLongInv));
            strftime(strTime,sizeof(strTime),"%d-%b-%Y %H:%M:%S",&tim);
            strTime[sizeof(strTime)-1] = '\0';
            fprintf(stderr, "timeValLongInv  %12lu\t%s\n",(time_t)timeValLongInv,strTime);
        }
    }
    if (nCnt > 0) {
        fprintf(stderr, "\nInverter date/time set (successful)\n");
        return(0);
    } else {
        if (bVerbose) fprintf(stderr, "no response, comparing time\n");
        if (abs(timeValLong - timeValLongInv) <= 2) {
            fprintf(stderr, "\nInverter date/time set (checked)\n");
            return(0);
        }
    }
    return(-1);
}


/*--------------------------------------------------------------------------
    GetCounters
----------------------------------------------------------------------------*/
int GetCounters(int fdser, int yAddress)
{
    char TRT[18], PRT[18], TGC[18], RPC[18];

    if ((yGetCount & 0x01) && GetCountersData(fdser, yAddress, cTotalRun, TRT) < 0) return(-1);
    if ((yGetCount & 0x02) && GetCountersData(fdser, yAddress, cPartialRun, PRT) < 0) return(-1);
    if ((yGetCount & 0x04) && GetCountersData(fdser, yAddress, cTotalGrid, TGC) < 0) return(-1);
    if ((yGetCount & 0x08) && GetCountersData(fdser, yAddress, cResetPartial, RPC) < 0) return(-1);

    fprintf(outfp, "\n%-34s   %18s\n","","yyy ddd hh:mm:ss");
    if (yGetCount & 0x01) fprintf(outfp, "%-34s : %18s\n",_cTotalRun,TRT);
    if (yGetCount & 0x02) fprintf(outfp, "%-34s : %18s\n",_cPartialRun,PRT);
    if (yGetCount & 0x04) fprintf(outfp, "%-34s : %18s\n",_cTotalGrid,TGC);
    if (yGetCount & 0x08) fprintf(outfp, "%-34s : %18s\n",_cResetPartial,RPC);

    return(0);
}


/*--------------------------------------------------------------------------
    GetCountersData
----------------------------------------------------------------------------*/
int GetCountersData(int fdser, int yAddress, int param, char *uptime)
{
    int nCnt;
    unsigned long paramValPtr, paramValLong;
    int years, days, hours, minutes, seconds;

    invFunc = -1;
    memcpy(szSerBuffer,_clearCMD,sizeof(szSerBuffer));		/* clear Aurora cmd string */
    szSerBuffer[cAddress] = yAddress;		/* set inverter address */
    szSerBuffer[cCommand] = invOp = opGetCounters;	/* set Measure request to the DSP opcode */
    szSerBuffer[cParam1] = invFunc = param;
    szSerBuffer[cParam1End] = '\0';
    nCnt = Communicate(fdser, yAddress);
    if (nCnt > 0 && TransState((int)szSerBuffer[aState],_opGetCounters,TRUE)) {
        paramValPtr = szCvrtLong(szSerBuffer);
        if (bVerbose) fprintf(stderr, "value        %12lu\n",(unsigned long)paramValPtr);
    }
    else
        return(-1);

    paramValLong = (unsigned long)paramValPtr;
    if (bVerbose) fprintf(stderr, "paramValLong %12lu\n",(unsigned long)paramValLong);
    years = paramValLong / SecsInYear;
    paramValLong -= (years * SecsInYear);
    if (bVerbose) fprintf(stderr, "paramValLong %12li\tyears   %3i\n",(unsigned long)paramValLong,years);
    days = paramValLong / SecsInDay;
    paramValLong -= (days * SecsInDay);
    if (bVerbose) fprintf(stderr, "paramValLong %12li\tdays    %3i\n",(unsigned long)paramValLong,days);
    hours = paramValLong / SecsInHour;
    paramValLong -= (hours * SecsInHour);
    if (bVerbose) fprintf(stderr, "paramValLong %12li\thours   %3i\n",(unsigned long)paramValLong,hours);
    minutes = paramValLong / SecsInMinute;
    paramValLong -= (minutes * SecsInMinute);
    if (bVerbose) fprintf(stderr, "paramValLong %12li\tminutes %3i\n",(unsigned long)paramValLong,minutes);
    seconds = paramValLong;
    if (bVerbose) fprintf(stderr, "paramValLong %12li\tseconds %3i\n",(unsigned long)paramValLong,seconds);
    
    sprintf(uptime,"%3d %3d %02d:%02d:%02d",years,days,hours,minutes,seconds);
    if (bVerbose) fprintf(stderr, "uptime: %s\n",uptime);

    return(0);
}

/*--------------------------------------------------------------------------
    exceededRT
----------------------------------------------------------------------------*/
BOOL exceededRT()
{
    time_t timeValue;

    if (yMaxRunTime > 0) {
        timeValue = time(NULL);
        if ((timeValue-startTimeValue) > yMaxRunTime) {
            fprintf(stderr, "\nMaximum %i second runtime exceeded (%luS)\n",yMaxRunTime,timeValue-startTimeValue);
            return(TRUE);
        }
    }
    return(FALSE);
}


/*--------------------------------------------------------------------------
    Communicate
----------------------------------------------------------------------------*/
int Communicate(int fdser, int yAddress)
{
    int i = 0;
    int nCnt;
    char szSerBufferSave[_szSerBufferLen];
    int CRCrc = -1;
    int attempts = 1;
    struct timeval curtv;
    long long int curtvusecs = 0, lastcommtvusecs = 0, elapsedtvusecs = 0;

    if (exceededRT()) return(-1);

    memcpy(szSerBufferSave,_clearCMD,sizeof(szSerBufferSave));
    memcpy(szSerBufferSave,szSerBuffer,_szSerBufferLen);
    gettimeofday(&curtv, NULL);
    while(CRCrc < 0 && attempts <= yMaxAttempts) {
        if (lastcommtv.tv_sec == 0 || lastcommtv.tv_usec == 0) {
            lastcommtv.tv_sec = curtv.tv_sec;
            lastcommtv.tv_usec = curtv.tv_usec;
        } else {
            curtvusecs = (curtv.tv_sec*1000000) + curtv.tv_usec;
            lastcommtvusecs = (lastcommtv.tv_sec*1000000) + lastcommtv.tv_usec;
            elapsedtvusecs = curtvusecs - lastcommtvusecs;
        }
        memcpy(szSerBuffer, szSerBufferSave,_szSerBufferLen);
        if (bVerbose) fprintf(stderr, "\nElapsed time since last comm %llu us\nAttempt %i",elapsedtvusecs,attempts);
        if (yCommPause > 0 && elapsedtvusecs < yCommPause) {
            if (bVerbose) fprintf(stderr, " Sleeping for %llu us",yCommPause-elapsedtvusecs);
            usleep(yCommPause-elapsedtvusecs);
        }
        if (bVerbose) fprintf(stderr, "\nClearing read buffer ");
        errno = 0;
        if (tcflush(fdser,TCIFLUSH))
            fprintf(stderr, "- Problem clearing buffer: (%i)\n%s\n",errno,strerror (errno));
        else
            if (bVerbose) fprintf(stderr, "Success!\n");
        if (bVerbose) {
            fprintf(stderr, "szSerBufferSave ");
            if (strcmp(szSerBuffer,szSerBufferSave) == 0)
                fprintf(stderr, "OK! ");
            else
                fprintf(stderr, "ERROR! ");
            for (i = 0; i < cSIZE; i++) {
                fprintf(stderr, "%02x ",(unsigned char)szSerBufferSave[i]);
            }
            fprintf(stderr, "\n");
        }
        crcValue = crc16(szSerBuffer, 8);
        szSerBuffer[cCRC_L] = LOBYTE(crcValue);
        szSerBuffer[cCRC_H] = HIBYTE(crcValue);
        szSerBuffer[cEND] = '\0';
        if (bVerbose) {
            fprintf(stderr, "command: ");
            for (i = 0; i < cSIZE; i++) {
                fprintf(stderr, "%02x ",(unsigned char)szSerBuffer[i]);
            }
            fprintf(stderr, "\nFlushing serial device buffer... ");
        }
        errno = 0;
        if (tcflush(fdser,TCIOFLUSH))
            fprintf(stderr, "Problem flushing before sending command: (%i) %s\n",errno,strerror (errno));
        else
            if (bVerbose) fprintf(stderr, "Success!\nSending command... ");
        nCnt = write(fdser, &szSerBuffer, cSIZE);		/* send it */
        if (bVerbose) fprintf(stderr, "sent %d characters\nDraining serial device buffer... ", nCnt);
        errno = 0;
        if (tcdrain(fdser))
            fprintf(stderr, "Problem draining command: (%i) %s\n",errno,strerror (errno));
        else
            if (bVerbose) fprintf(stderr, "Success!\n");
        memcpy(szSerBuffer, _clearCMD,sizeof(szSerBuffer));
        if (bVerbose) {
            fprintf(stderr, "Cleared data buffer: ");
            for (i = 0; i < cSIZE; i++) {
                fprintf(stderr, "%02x ",(unsigned char)szSerBuffer[i]);
            }
            fprintf(stderr, "\n");
        }
        tcflush(fdser,TCIFLUSH);
        if (yReadPause > 0) {
            if (bVerbose)
                fprintf(stderr, "Waiting %d milli-seconds before reading inverter response\n",yReadPause);
            else
                if (bRptReadPause) fprintf(stderr, "\n%s: %s: Waiting %d milli-seconds before reading inverter response",getCurTime(),ProgramName,yReadPause);
            usleep(yReadPause*1000);
        }
        nCnt = ReadToBuffer(fdser, szSerBuffer, aSIZE);
        if (bVerbose) {
            fprintf(stderr, "answer:  ");
            if (nCnt > 0) {
                for (i = 0; i < nCnt; i++) {
                    fprintf(stderr, "%02x ",(unsigned char)szSerBuffer[i]);
                }
                fprintf(stderr, "\nreceived %d characters\n", nCnt);
            } else
                fprintf(stderr, "Got %d characters\n", nCnt);
        }
        if (nCnt > 0) {
            crcValue = crc16(szSerBuffer, 6);
            if ((unsigned char)szSerBuffer[aCRC_L] != LOBYTE(crcValue) || (unsigned char)szSerBuffer[aCRC_H] != HIBYTE(crcValue)) {
                if (yMaxAttempts == 1 || attempts == yMaxAttempts)
                    if (!bCommCheck) {
                        if (! bVerbose && bRptReadPause) fprintf(stderr, "\n");
                        fprintf(stderr, "%s: CRC receive error (%i attempts made) %04x %02x %02x\n",getCurTime(),attempts,crcValue,(unsigned char)szSerBuffer[aCRC_H],(unsigned char)szSerBuffer[aCRC_L]);
                    }
            } else {
                if (bVerbose) fprintf(stderr, "CRC receive OK %04x\n",crcValue);
                CRCrc = 0;
            }
        }
        gettimeofday(&curtv, NULL);
        lastcommtv.tv_sec = curtv.tv_sec;
        lastcommtv.tv_usec = curtv.tv_usec;
        attempts++;
    }
    if (CRCrc < 0) return(-1);
    if (bRptRetries) {
        fprintf(stderr, "\n%s: %s: %i attempts made",getCurTime(),ProgramName,attempts-1);
        if (bVerbose) fprintf(stderr, "\n");
    } else
        if (bRptReadPause) fprintf(stderr, "\n");
    return(nCnt);
}

/*--------------------------------------------------------------------------
    szCvrtFloat
    Converts a 4 char string to a float.
----------------------------------------------------------------------------*/
float szCvrtFloat(char *Buffer)
{
    unsigned char cValue[4];
    float *value;

    if (! bSwapEndian) {
        cValue[0] = Buffer[aParam4];
        cValue[1] = Buffer[aParam3];
        cValue[2] = Buffer[aParam2];
        cValue[3] = Buffer[aParam1];
    } else {
        cValue[0] = Buffer[aParam1];
        cValue[1] = Buffer[aParam2];
        cValue[2] = Buffer[aParam3];
        cValue[3] = Buffer[aParam4];
    }

    value = (float *)cValue;
    if (bVerbose) fprintf(stderr, "szCvrtFloat %12.6f 0x%02x%02x%02x%02x\n",*value,cValue[3],cValue[2],cValue[1],cValue[0]);

    return(*value);
}

/*--------------------------------------------------------------------------
    szCvrtShort
    Converts a 2 char string to a short.
----------------------------------------------------------------------------*/
unsigned short szCvrtShort(char *Buffer)
{
    unsigned char cValue[2];
    unsigned short *value = 0;

    if (! bSwapEndian) {
        cValue[0] = Buffer[aParam2] & 0xff;
        cValue[1] = Buffer[aParam1] & 0xff;
    } else {
        cValue[0] = Buffer[aParam1] & 0xff;
        cValue[1] = Buffer[aParam2] & 0xff;
    }

    value = (unsigned short *)cValue;
    if (bVerbose) fprintf(stderr, "szCvrtShort  %12u 0x%02x%02x\n",*value & 0xffff,cValue[1],cValue[0]);

    return(*value & 0xffff);
}


/*--------------------------------------------------------------------------
    szCvrtLong
    Converts a 4 char string to a long.
----------------------------------------------------------------------------*/
unsigned long szCvrtLong(char *Buffer)
{
    unsigned char cValue[4];
    unsigned long *value = 0;

    if (! bSwapEndian) {
        cValue[0] = Buffer[aParam4] & 0xff;
        cValue[1] = Buffer[aParam3] & 0xff;
        cValue[2] = Buffer[aParam2] & 0xff;
        cValue[3] = Buffer[aParam1] & 0xff;
    } else {
        cValue[0] = Buffer[aParam1] & 0xff;
        cValue[1] = Buffer[aParam2] & 0xff;
        cValue[2] = Buffer[aParam3] & 0xff;
        cValue[3] = Buffer[aParam4] & 0xff;
    }

    value = (unsigned long *)cValue;
    if (bVerbose) fprintf(stderr, "szCvrtLong   %12lu 0x%02x%02x%02x%02x\n",*value & 0xffffffff,cValue[3],cValue[2],cValue[1],cValue[0]);

    return(*value & 0xffffffff);
}


/*--------------------------------------------------------------------------
    cvrtShort
    Converts a 2 char string to a short.
----------------------------------------------------------------------------*/
/* not used at this time
unsigned short cvrtShort(char *Buffer)
{
    unsigned char cValue[2];
    unsigned short *value = 0;

    if (! bSwapEndian) {
        cValue[0] = Buffer[1] & 0xff;
        cValue[1] = Buffer[0] & 0xff;
    } else {
        cValue[0] = Buffer[0] & 0xff;
        cValue[1] = Buffer[1] & 0xff;
    }

    value = (unsigned short *)cValue;
    if (bVerbose) fprintf(stderr, "cvrtShort    %12u 0x%02x%02x\n",*value & 0xffff,cValue[1],cValue[0]);

    return(*value & 0xffff);
}
*/


/*--------------------------------------------------------------------------
    cvrtLong
    Converts a 4 char string to a long.
----------------------------------------------------------------------------*/
unsigned long cvrtLong(char *Buffer)
{
    unsigned char cValue[4];
    unsigned long *value = 0;

    if (! bSwapEndian) {
        cValue[0] = Buffer[3] & 0xff;
        cValue[1] = Buffer[2] & 0xff;
        cValue[2] = Buffer[1] & 0xff;
        cValue[3] = Buffer[0] & 0xff;
    } else {
        cValue[0] = Buffer[0] & 0xff;
        cValue[1] = Buffer[1] & 0xff;
        cValue[2] = Buffer[2] & 0xff;
        cValue[3] = Buffer[3] & 0xff;
    }

    value = (unsigned long *)cValue;
    if (bVerbose) fprintf(stderr, "cvrtLong     %12lu 0x%02x%02x%02x%02x\n",*value & 0xffffffff,cValue[3],cValue[2],cValue[1],cValue[0]);

    return(*value & 0xffffffff);
}

/*--------------------------------------------------------------------------
    FindString
    Reads command line parameters.
----------------------------------------------------------------------------*/
char* FindString(WORD wRule, char *ptr)
{

    while(wRule--) {		/* walk thru the null terminators */
        while(*ptr++)
        ;
    }
    return ptr;
}
 

/*--------------------------------------------------------------------------
    ReadNextChar
    Reads the next character from the serial device. Returns zero if no
    character was available.
----------------------------------------------------------------------------*/
int ReadNextChar(int nfd, char *pChar, int timeout)
{
    int nResult = -1;
    long int eUsecs, sSecs, cSecs, sUsecs, cUsecs;
    struct timeval tv;

    if (exceededRT()) return(-1);

    errno = 0;

    sUsecs = cUsecs = 0;
    eUsecs = sSecs = cSecs = sUsecs = cUsecs = 0;
    memset (pChar, 0, sizeof (*pChar));
    eUsecs = 0;
    gettimeofday(&tv, NULL);
    sSecs = tv.tv_sec;
    sUsecs = tv.tv_usec;
    nResult = read(nfd, pChar, 1);
    gettimeofday(&tv, NULL);
    cSecs = tv.tv_sec;
    cUsecs = tv.tv_usec;
    eUsecs = ((cSecs-sSecs)*1000000) + (cUsecs-sUsecs);
    if (errno != 0)
        fprintf (stderr, "\naurora: (TO) Problem reading serial device, (nResult %i) (errno %i) %s.\n",nResult,errno,strerror (errno));
    if (nResult == -1) {
        if (errno == 0) perror("\naurora: (TO) Problem reading serial device. \n");
        RestorePort(nfd);
        ClrSerLock(PID);
        fprintf (stderr, "\n");
        exit(2);
    }
    if (bVerbose) {
        fprintf(stderr, "RC=%i (%02x) ",nResult,(unsigned char)*pChar);
        if ((cUsecs-sUsecs) > 0) fprintf(stderr, "waited/max %8i/%-8i uS ",(int)eUsecs,(timeout*1000)+(yDelay*1000000));
    }
    return nResult;

}

/*--------------------------------------------------------------------------
    ReadToBuffer
    Reads data to a buffer until no more characters are available. If
    the buffer overflows, returns -1. Otherwise, returns the number of
    characters read.
----------------------------------------------------------------------------*/
int ReadToBuffer(int nfd, char *pszBuffer, int nBufSize)
{
    int nPos = 0;		/* current character position */
    int attempts = 0;
    char *pBuf = pszBuffer;
    int rc = 0;
    int sanity = 0;

    while(nPos < nBufSize && sanity < MAX(1,yTimeout/100)) {
        if (bVerbose) {
            attempts++;
            fprintf(stderr, "Read char #%i ",nPos+1);
        }
        rc = ReadNextChar(nfd, pBuf, yTimeout);
        if (bVerbose && attempts > 1) fprintf(stderr, "attempts %i ",attempts);
        if (rc < 0)
            return nPos;	/* no character available */
        if (rc > 0) {
            pBuf++;
            nPos++;
            attempts = sanity = 0;
        } else 
            sanity += MAX(1,yDelay);
        if (bVerbose) fprintf(stderr, "\n");
    }
    if (nPos < nBufSize) return -1;	/* problem */
    return nPos;
}


/*--------------------------------------------------------------------------
    crc16
                                         16   12   5
    this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
    This is 0x1021 when x is 2, but the way the algorithm works
    we use 0x8408 (the reverse of the bit pattern).  The high
    bit is always assumed to be set, thus we only use 16 bits to
    represent the 17 bit value.
----------------------------------------------------------------------------*/

#define POLY 0x8408   /* 1021H bit reversed */

WORD crc16(char *data_p, unsigned short length)
{
      unsigned char i;
      unsigned int data;
      unsigned int crc = 0xffff;

      if (length == 0)
        return (~crc);
      do
      {
        for (i=0, data=(unsigned int)0xff & *data_p++;
         i < 8; 
         i++, data >>= 1)
        {
          if ((crc & 0x0001) ^ (data & 0x0001))
            crc = (crc >> 1) ^ POLY;
          else  crc >>= 1;
        }
      } while (--length);

      crc = ~crc;

      return (crc);
}

/*--------------------------------------------------------------------------
    Delay
    Delays by the number of seconds and microseconds.
----------------------------------------------------------------------------*/
void Delay(int secs, long microsecs)
{
    static struct timeval t1;

    t1.tv_sec = (long)secs;
    t1.tv_usec = microsecs;
    if ( select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &t1)  < 0 )
         perror("Internal error: error in select()");
    return;
}


