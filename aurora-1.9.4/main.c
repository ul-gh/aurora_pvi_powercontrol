/*
 * aurora - Communications with Magnetek Aurora Inverter
 *
 * Copyright (C) 2006-2020 Curtis J. Blank curt@curtronics.com
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
 * Starting with v1.5-0 this work based on Power One's 'Aurora Inverter
 * Series - Communication Protocol -' document, Rel. 4.6 25/02/09
 *
 * Special thanks to Tron Melzl at Magnetek for all his help including, but
 * by no means limited to, supplying the Communications Protocol document
 * And another thanks to Jonathan Duddington on the other side of the pond
 * for his contribution of items he gathered while reverse engineering the 
 * inverters communication some of which were incorporated beginning with 
 * verison 1.7.8
 *
 * modified 17-jul-2006 cjb	1. v1.2-6 Last 7 Days production value has been dropped 
 *				   in the v2.3 Communications Protocol doc
 * modified 27-jul-2006 cjb	1. add bVerbose in ClrSerLock
 * modified 13-oct-2006 cjb	1. make this v1.3-0
 * modified 25-apr-2007 cjb	1. take into account Daylight Savings Time when setting the Aurora's time
 * modified 29-dec-2008 cjb	1. correct an error in strftime that only may show up in the first or last
 * 				   week of the year (%G vs %Y)
 * modified 19-aug-2009 cjb	1. fix determining if serial port is in use
 * modified 22-aug-2009 cjb	1. disable XON/XOFF flow control on output
 * modified 29-sep-2009 cjb	1. don't set lckCNT = -1 when clearing stale serial port lock
 * modified 12-oct-2009 cjb	1. add -o option to output data to a file
 * modified 25-oct-2009 cjb	1. serial port configuration tweaks
 * modified 17-oct-2010 cjb	1. added -x option to enable XON/XOFF
 * modified 07-mar-2010 cjb	1. added -Y retries option and -y option to report the number of attempt made
 * modified 13-mar-2010 cjb	1. added -P option to delay read after sending command to inverter
 *				2. rename -P to -U and add -u to report it
 *				3. changed -U seconds to wait to milli-seconds to wait
 * modified 28-mar-2010 cjb	1. working on adding more DSP information
 * modified 27-jul-2010 cjb	1. added -P option to throttle commands sent to the inverter
 * modified 21-sep-2010 cjb	1. added -A option to read "Last four alarms"
 * modified 22-jan-2011 cjb	1. added -k option to get up to a years worth of daily generation history
 * modified 19-mar-2011 cjb	1. added -X option to enable RTS/CTS on the serial port
 * modified 13-aug-2001 cjb	1. increase the allowed values from 31 to 63 for inverter address (-a)
 *                                 as allowed in newer inverters
 * modified 22-aug-2011 cjb	1. clean up compile 'unused-but-set' messages
 * modified 05-nov-2011 cjb	1. remove size restriction on monetary identifier of the -C option
 *                              2. added -q --energy-sent option
 * modified 09-nov-2011 cjb	1. added -L --adjust-time option
 * modified 13-nov-2011 cjb     1. if a bad return code is returned from any function display a 
 *                                 message before exiting (used to just exit silently)
 *                              2. added -W option swap endian
 * modified 26-nov-2011 cjb     1. make sure the value passed to the -d option is numeric
 * modified 02-jan-2012 cjb	1. add bad return code tracking
 * modified 30-jan-2012 cjb     1. added -M option maximum amount of time in seconds that aurora will run
 * modified 18-feb-2012 cjb	1. changed --delay to --read-wait as the long option for -l so as to better 
 *                                 reflect it's function
 * modified 24-feb-2012 cjb	1. modify -i, --get-count option to accept a bitmask
 * modified 25-feb-2012 cjb	1. minor changes to help
 * modified 12-jun-2012 cjb	1. make sure errno is initialized
 * modified 18-aug-2015 cjb	1. improve device lock file handling - see RELEASENOTES
 * modified 19-jan-2016 cjb	1. on error when checking to print invFunc 0 (zero) is inclusive.
 * modified 20-jan-2016 cjb	1. lckCNT no longer used
 * modified 05-jun-2020 cjb	1. no longer define VersionC & VersionSHc here - causes a comiler problem
 *				   that first surfaced with gcc version 10.1.0
 *
 */

static char	VersionM[] = "1.9.4";

#include <syscall.h>
#include <unistd.h>
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
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <error.h>
#include "include/main.h"
#include "include/comm.h"
#include "include/names.h"

BOOL bVerbose = FALSE;
BOOL bColumns = FALSE;		/* Output data in columns */
int yGetDSP = -1;		/* Measure request to the DSP */
BOOL bGetDSPExtended = FALSE;	/* Measure request to the DSP more parameters */
BOOL bGetDSP3Phase = FALSE;	/* Measure request to the DSP three-phase parameter */
BOOL bHideDSP = FALSE;
BOOL bGetEnergy = FALSE;	/* Cumulated Energy Readings */
int yGetEnergyCen = -1;	/* Cumulated Energy Readings Central & TRIO only */
BOOL bGetInvTime = FALSE;	/* Display Inverter time flag */
BOOL bGetLocTime = FALSE;	/* Display computer time flag */
BOOL bCommCheck = FALSE;
BOOL bColOutput = FALSE;
BOOL bCalcGridPwr = FALSE;
BOOL bXonXoff = FALSE;
BOOL bRTSCTS = FALSE;
BOOL bRptRetries = FALSE;
BOOL bRptReadPause = FALSE;
BOOL bSwapEndian = FALSE;
BOOL bUseMultiplierQ = FALSE;
float yMultiplierK = 1.0;
int yTimeout = 0;		/* read timeout value in us */
long unsigned int PID;
int yMaxAttempts = 1;
int yReadPause = 0;
long int yCommPause = 0;
unsigned char yDelay;		/* Read wait time */
int yGetCount = 0;		/* Get Time Counters */
time_t startTimeValue;
int yCentral = -1;
int yMaxRunTime = 0;		/* max seconds aurora will run trying to get data from the inverter */
int yCheckSetTime = -1;		/* Check the time on the inverter and if different then system time set it */
struct timeval lastcommtv;

FILE *outfp;

float yCost = 0.0;
char *sCostType = NULL;

/* local Data */
static char VersionCHc[6] = VersionCH;
static char VersionMHc[6] = VersionMH;
static char VersionNHc[6] = VersionNH;
static int yAddress = 0;			/* Inverter address 1-63 */
static char *szttyDevice = NULL;
static char *outfile = NULL;
static BOOL bHelp = FALSE;			/* Print the help text */
static BOOL bGetPN = FALSE;			/* Get Inverter Part Number */
static BOOL bGetPNC = FALSE;			/* Get Inverter Part Number (Central Only) */
static BOOL bGetVer = FALSE;			/* Get version string */
static BOOL bGetVerFW = FALSE;			/* Get firmware version string */
static BOOL bGetSN = FALSE;			/* Get Serial Number */
static BOOL bGetMfg = FALSE;			/* Get Manufacturing Date */
static BOOL bGetConf = FALSE;			/* Get System Configuration */
static BOOL bGetState = FALSE;			/* State request */
static BOOL bGetJoules = FALSE;			/* Energy cumulated in the last 10 seconds */
static BOOL bSetTime = FALSE;			/* Set time flag */
static BOOL bVersion = FALSE;			/* Display program version */
static BOOL bGetTime;
static BOOL bGetLastAlarms = FALSE;		/* Get last four alarms */
static BOOL bFileError = FALSE;
static int yGetEnergyDaily = 0;
static int yGetEnergySent = 0;
static float yLockWait = 0.0;			/* Seconds to wait to lock serial port */
char *devLCKfile = NULL;
char *devLCKfileNew = NULL;
static struct termios oldtio;			/* current serial device configuration */

/* local functions */
static int GetParms(int argc, char *argv[]);
static void *getMemPtr(size_t mSize);
static void Version();
static int getPIDcmdLen(long unsigned int PID);
static void getPIDcmd(long unsigned int PID, char* COMMAND);
static int rnduSecs();

/*--------------------------------------------------------------------------
    getCurTime
----------------------------------------------------------------------------*/
char* getCurTime()
{
    time_t curTimeValue;
    struct tm timStruct;
    static char CurTime[18] = " ";

    curTimeValue = time(NULL);
    timStruct = *(localtime(&curTimeValue));
    strftime(CurTime,sizeof(CurTime),"%Y%m%d-%H:%M:%S",&timStruct);
    CurTime[sizeof(CurTime)-1] = '\0';

    return CurTime;
}

/*--------------------------------------------------------------------------
    getPIDcmdLen
----------------------------------------------------------------------------*/
int getPIDcmdLen(long unsigned int PID)
{
    FILE *fdserlck = NULL;
    int fLen = 0;
    char sPID[10];
    char *cmdFile = NULL;

    sPID[0] = '\0';
    sprintf(sPID,"%lu",PID);
    cmdFile = getMemPtr(strlen(sPID)+14+1);
    cmdFile[0] = '\0';
    sprintf(cmdFile,"/proc/%lu/cmdline",PID);
    cmdFile[strlen(cmdFile)] = '\0';
    fdserlck = fopen(cmdFile, "r");
    if (fdserlck != NULL) {
        while (fgetc(fdserlck) != EOF) fLen++;
        fclose(fdserlck);
        fdserlck = NULL;
    }
    if (cmdFile != NULL) {
        free(cmdFile);
        cmdFile = NULL;
    }
    return fLen;
}


/*--------------------------------------------------------------------------
    getPIDcmd
----------------------------------------------------------------------------*/
void getPIDcmd(long unsigned int PID, char* COMMAND)
{
    FILE *fdserlck = NULL;
    int bRead = 0;
    int fLen = 0;
    char sPID[10];
    char *cmdFile = NULL;

    sPID[0] = '\0';
    sprintf(sPID,"%lu",PID);
    cmdFile = getMemPtr(strlen(sPID)+14+1);
    cmdFile[0] = '\0';
    sprintf(cmdFile,"/proc/%lu/cmdline",PID);
    cmdFile[strlen(cmdFile)] = '\0';
    fLen = getPIDcmdLen(PID);
    if (fLen > 0) {
        fdserlck = fopen(cmdFile, "r");
        if (fdserlck != NULL) {
            COMMAND[0] = '\0';
            bRead = fscanf(fdserlck, "%s", COMMAND);
            if (bRead) COMMAND[strlen(COMMAND)] = '\0';
            fclose(fdserlck);
            fdserlck = NULL;
        }
    }
    if (cmdFile != NULL) {
        free(cmdFile);
        cmdFile = NULL;
    }
}


/*--------------------------------------------------------------------------
    main
----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int fdser;				/* serial device fd */
    FILE *fdserlck = NULL;
    struct termios newtio;		/* serial device configuration */
    int rc = 0;
    struct tm timStruct;
    time_t endTimeValue;
    char RunTime[18] = " ";
    char EndTime[18] = " ";
    long unsigned int LckPID;
    int bRead, bWrite;
    int errno_save = 0;
    int fLen = 0;
    int curChar = 0;
    int uSlckWait = 0;
    float LockWaitElapsed = 0.0;
    float LockWaitElapsedTot = 0.0;
    BOOL lckCNTbeg = TRUE;
    char *COMMAND = NULL;
    char *LckCOMMAND = NULL;
    char *LckPIDcommand = NULL;
    long L=1; 
    void *Ptr=&L; 
    char Endian=*(char*)Ptr;
    struct timeval startv;


    lastcommtv.tv_sec = 0;
    lastcommtv.tv_usec = 0;
    startTimeValue = time(NULL);
    gettimeofday(&startv, NULL);
    srand((startv.tv_usec & 0x7ffe) + 1);

    timStruct = *(localtime(&startTimeValue));
    strftime(RunTime,sizeof(RunTime),"%Y%m%d-%H:%M:%S",&timStruct);
    RunTime[sizeof(RunTime)-1] = '\0';

    errno = 0;
    PID = getpid();


    /* Get command line parms */
    if ((!GetParms(argc, argv) && !bVersion) || bHelp) {
        printf("\nSee http://www.curtronics.com/Solar/ for Solar Site example\n\n");
        printf("Usage: %s [Options] Device\t\t\tv%s\n\n",ProgramName,VersionM);
        printf("Options:\n");
        printf(" -A, --last-alarms              Get last four alarms (once displayed FIFO queue is cleared)\n");
        printf(" -b, --verbose                  Verbose mode. For maximum effectiveness should be the first option\n");
        printf("                                  on the command line\n");
        printf(" -C, --calc-value=<num[:$]>     Calculate monetary value using <num> * kWh. \":$\" is optional and if\n");
        printf("                                  included the character(s) represented by the \"$\" will be used to\n");
        printf("                                  denote monetary type in the output. Defaults to \"$\"\n");
        printf(" -c, --columnize                Output data in columns --> for -d, -e, -D, -3, -E, -t, & -T options\n");
        printf("                                  only will disable any other options -- if value ends with an\n");
        printf("                                  \"*\" reporting of that item may not be in inverters firmware\n");
        printf(" -d <num>, --get-dsp=<num>      Get DSP data <num> indicates string to get data for. 0 indicates both\n");
        printf("                                  1 for only string 1, 2 for only string 2. <num> is required for \n");
        printf("                                  short option and <num> is optional for long option and if omitted\n");
        printf("                                  for long option data for both input strings will be retrieved\n");
        printf(" -D, --get-dsp-extended         Get more DSP data\n");
        printf(" -3, --get-dsp-3phase           Get 3-Phase DSP data\n");
        printf(" -e, --get-energy               Get Cumulated Energy readings (Excluding Central)\n");
        printf(" -E <num>, --get-energy-central=<num>  Get Cumulated Float Energy readings (Central & TRIO only) <num> days\n");
        printf("                                  1 to 366 Default 366 <num> is required for short option and optional\n");
        printf("                                  for long option\n");
        printf(" -f, --firmware-ver             Query for Firmware Version string\n");
        printf(" -g, --mfg-date                 Query for Inverter Manufacturing Date\n");
        printf(" -h, --help                     This text\n");
        printf(" -i, --get-count=<bitmask>      Display Inverter Time Counters <bitmask> is required for short option\n");
        printf("                                  is optional for long option and if omitted for long option then all\n");
        printf("                                  counters will be displayed and Partial counter will be reset\n");
        printf("                                  1 = \"Total Run Time\"    4 = \"Grid Connection Time\"\n");
        printf("                                  2 = \"Partial Run Time\"  8 = \"Reset Partial Run Time\"\n");
        printf(" -j, --get-joules               Display Energy cumulated in the last 10 seconds\n");
        printf(" -K, --use-kmult=<num>          Adjust vaules reported by -k, --daily-kwh by \"<num>\" multiplier\n");
        printf(" -k <num>, --daily-kwh=<num>    Get past daily kWh for <num> days (1-366)\n");
        printf("                                  <num> is required for short option and optional for long option\n");
        printf("                                  and if omitted for long option then all 366 days or as many that\n");
        printf("                                  are found will be displayed\n");
        printf(" -L <num>, --adjust-time=<num>  Automatically adjust the inverter time if it differs from the \n");
        printf("                                  computer time. If <num> is 0 (zero) do a Daylight Savings Time\n");
        printf("                                  check. If <num> is >= 1 change the inverter time if it differs\n");
        printf("                                  by <num> or more seconds. See the README for more information\n");
        printf("                                  and constraints\n");
        printf(" -l <num>, --read-wait=<num>    Timeout for the read to the serial port. <num> in 1/10ths seconds.\n");
        printf("                                  Default is 1 (0.1 sec). See the README file for important\n");
        printf("                                  information on this option\n");
        printf(" -M <num>, --max-runtime=<num>  Maximum amount of time in seconds that aurora will run trying to\n");
        printf("                                  get data from the inverter\n");
        printf(" -m, --get-conf                 Query for Inverter System Configuration\n");
        printf(" -n, --serial-number            Query for Inverter Serial Number\n");
        printf(" -N <num>, --central=<num>      Indicates Aurora Central ** Experimental **\n");
        printf("                                  0 indicates Master, 1 indicates Slave\n");
        printf(" -o, --output-file=<filename>   Append data to file (Created if nonexistant) Note: If -c option is\n");
        printf("                                  used only -d, -e, -D, -3, -E, -t, & -T options will output to file\n");
        printf(" -O, --part-number-central      Query for Inverter Part Number (Central only)\n");
        printf(" -p, --part-number              Query for Inverter Part Number\n");
        printf(" -P <num>, --comm-pause=<num>   Wait <num> us between sending commands to inverter (1-1000000)\n");
        printf(" -Q, --use-qmult                Use inverter specific multiplier if known to adjust values reported\n");
        printf("                                  by -q, --energy-sent\n");
        printf(" -q <num>, --energy-sent=<num>  Get past energy delivered to the grid ** Experimental **\n");
        printf("                                  in 10 second intervals for <num> minutes (1-1440). <num> is\n");
        printf("                                  optional for long option and if omitted all data, ~24 hours\n");
        printf("                                  worth will be reported. It is suggested the -Y option be used\n");
        printf("                                  with this due to the extensive length of time this could take.\n");
        printf("                                  See the README file for important information on this option\n");
	printf(" -R <num>, --read-timeout=<num> Serial port read retry timeout value when reading a character from\n");
        printf("                                  the Inverter (mS - minimum 200) See the README file for\n");
        printf("                                  important information on this option\n");
        printf(" -r, --calc-grid-power          Calc Grid power using Grid Voltage * Grid Current, instead of\n");
        printf("                                  reporting the Inverter's value. --> for -d option only,\n");
        printf("                                  ignored when used with -c option (Inverter typically reports a\n");
        printf("                                  lower value. This affects Inverter conversion efficiency value.)\n");
        printf(" -S, --set-time                 Set Inverter Date/Time to system time\n");
        printf(" -s, --get-state                Get Inverter State\n");
        printf(" -T, --get-loctime              Display computer Date/Time\n");
        printf(" -t, --get-invtime              Display Inverter Date/Time\n");
        printf(" -V, --version                  Aurora communications program version\n");
        printf(" -v, --inv-version              Query for Version string\n");
	printf(" -U <num>, --read-pause=<num>   Pause <num> milli-seconds after sending command to inverter before\n");
        printf("                                  reading response from inverter (1-10000)\n");
        printf(" -u, --rpt-read-pause           Report when/that pausing before read\n");
        printf(" -W, --swap-endian              Swap Endianness\n");
        printf(" -w <num>, --lock-wait=<num>    Milliseconds to wait to lock serial port. (%.3f-30000.0 mS)\n",(float)(uSecsLCKWaitMin/1000.0));
        printf("                                  Default 0.0 mS meaning only one attempt to get the lock will be made.\n");
        printf(" -X, --rts-cts                  Enable RTS/CTS on the serial port.\n");
	printf(" -x, --xon-xoff                 Enable XON/XOFF on the serial port.\n");
        printf(" -Y <num>, --retries=<num>      Retry failed communications with inverter up to <num> times (1-100)\n");
        printf(" -y, --rpt-retries              Report the number of retires done\n\n");
        printf("               *** Required Parameters ***\n");
        printf(" -a <num>, --address=<num>      Inverter address. 1-31 on older inverters, 1-63 on newer inverters.\n");
        printf(" Device                         Serial Device.\n");
        printf("\n");
        if (bHelp) 
            exit(0);
        else
            exit(2);
    }

    if (bVerbose) {
        fprintf(stderr, "\nRunTime     %s v%-6s\nEndian    : %s\ntm_gmtoff : ",RunTime,VersionM,Endian ? "Little" : "Big");
#ifdef NO_TM_GMTOFF
        fprintf(stderr, "no\n");
#else
        fprintf(stderr, "yes\n");
#endif
    }

    if (yCost > 0.0 && sCostType == NULL) {
        sCostType = getMemPtr(2);
        strcpy(sCostType,"$");
        sCostType[1] = '\0';
        if (bVerbose) fprintf (stderr, "Monetary type \"%s\"\n",sCostType);
    }

    if (bVerbose) fprintf (stderr, "PID       : %lu\n", PID);

    if (bVersion) {
        Version();
        exit(0);
    }

    fLen = getPIDcmdLen(PID);
    COMMAND = getMemPtr(fLen+1);
    getPIDcmd(PID,COMMAND);

    if (bVerbose) fprintf(stderr, "\nAttempting to get lock on Serial Port %s...\n",szttyDevice);
    fdserlck = fopen((const char *)devLCKfile, "a");
    if (fdserlck == NULL) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "%s: %s: Problem locking serial device, can't open lock file: %s for write.\n\n",getCurTime(),ProgramName,devLCKfile);
        exit(2);
    }
    bWrite = fprintf(fdserlck, "%lu %s\n", PID, COMMAND);
    errno_save = errno;
    fclose(fdserlck);
    fdserlck = NULL;
    if (bWrite < 0 || errno_save != 0) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "%s: %s: Problem locking serial device, can't write lock file: %s.\n%s\n\n",getCurTime(),ProgramName,devLCKfile,strerror (errno_save));
        exit(2);
    }

    LckPID = 0;
    lckCNTbeg = TRUE;
    while(LckPID != PID && yLockWait >= 0.0) {
        if (bVerbose && lckCNTbeg) {
            fprintf(stderr, "Checking for lock\n");
            lckCNTbeg = FALSE;
        }
        fdserlck = fopen(devLCKfile, "r");
        if (fdserlck == NULL) {
            if (bVerbose) fprintf(stderr, "\n");
            fprintf(stderr, "%s: %s: Problem locking serial device, can't open lock file: %s for read.\n\n",getCurTime(),ProgramName,devLCKfile);
            exit(2);
        }

        fLen = 0;
        while ((curChar = fgetc(fdserlck)) != EOF && curChar != '\n' && curChar != ' ') fLen++;
        fLen = 0;
        while ((curChar = fgetc(fdserlck)) != EOF && curChar != '\n') fLen++;
        rewind(fdserlck);
        LckCOMMAND = getMemPtr(fLen+1);
        LckCOMMAND[0] = '\0';

        bRead = fscanf(fdserlck, "%lu %s", &LckPID, LckCOMMAND);
        errno_save = errno;
        fclose(fdserlck);
        if (bVerbose && LockWaitElapsed >= 1000.0) fprintf(stderr, "\nChecking process %lu (%s) for lock\n", LckPID, LckCOMMAND);
        fdserlck = NULL;
        if (bRead == EOF || errno_save != 0) {
            if (bVerbose) fprintf(stderr, "\n");
            fprintf(stderr, "%s: %s: Problem locking serial device, can't read lock file: %s.\n%s\n\n",getCurTime(),ProgramName,devLCKfile,strerror (errno_save));
            exit(2);
        }

        fLen = getPIDcmdLen(LckPID);
        LckPIDcommand = getMemPtr(fLen+1);
        getPIDcmd(LckPID,LckPIDcommand);

        if (bVerbose) {
            if (LckPID == PID)
                fprintf (stderr, "PID: %lu COMMAND: \"%s\" LckPID: %lu LckCOMMAND: \"%s\" LckPIDcommand \"%s\" = me\n", PID, COMMAND, LckPID, LckCOMMAND, LckPIDcommand);
            else if (LockWaitElapsed >= 1000.0)
                fprintf (stderr, "PID: %lu COMMAND: \"%s\" LckPID: %lu LckCOMMAND: \"%s\" LckPIDcommand \"%s\"\n", PID, COMMAND, LckPID, LckCOMMAND, LckPIDcommand);
        }

//      PID           - this process
//      LckPID        - PID from lock file
//	COMMAND       - this process
//	LckCOMMAND    - process command from lock file
//	LckPIDcommand - process command of process using PID from lock file
        if ((PID != LckPID && LckPIDcommand == NULL) || strcmp(LckCOMMAND,LckPIDcommand) != 0 || strcmp(LckPIDcommand,"") == 0) {
            if (bVerbose && uSlckWait != 0) fprintf (stderr, "\n");
            fprintf(stderr, "%s: %s: Clearing stale serial port lock. (%lu)\n",getCurTime(),ProgramName,LckPID);
            ClrSerLock(LckPID);
            LckPID = 0;
        }
        if (PID != LckPID && LckPID != 0 && yLockWait >= 0.0) {
            uSlckWait = rnduSecs();
            if (bVerbose && LockWaitElapsed >= 1000.0) {
                fprintf (stderr, "Waited: %.3f mS\n",LockWaitElapsed); 
                LockWaitElapsed = 0.0;
            }
            usleep(uSlckWait);
            yLockWait -= (float)(uSlckWait/1000);
            LockWaitElapsed += (float)(uSlckWait/1000);
            LockWaitElapsedTot += (float)(uSlckWait/1000);  
        }
        if (LckCOMMAND != NULL) {
            free(LckCOMMAND);
            LckCOMMAND = NULL;
        }
        if (LckPIDcommand != NULL) {
            free(LckPIDcommand);
            LckPIDcommand = NULL;
        }
    }
    free(COMMAND);
    if (bVerbose) {
        if (LockWaitElapsedTot > 0.0) fprintf(stderr, "\nLockWaitElapsedTot: %.3f mS\n",LockWaitElapsedTot);
        if (LckPID == PID) fprintf(stderr, "Appears we got the lock.\n");
    }
    if (LckPID != PID) {
        if (bVerbose) fprintf (stderr, "\n");
        fprintf(stderr, "%s: %s: Problem locking serial device %s, couldn't get the lock for %lu, locked by %lu.\n",getCurTime(),ProgramName,szttyDevice,PID,LckPID);
        ClrSerLock(PID);
        exit(2);
    }

    if (bVerbose) fprintf(stderr, "\nOpening Serial Port %s...  ", szttyDevice);
    fdser = open(szttyDevice, O_RDWR | O_NOCTTY );
    if (fdser < 0) {
        ClrSerLock(PID);
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "%s: %s: Problem opening serial device, check device name.\n\n",getCurTime(),ProgramName);
        exit(2);
    }
    if (bVerbose) fprintf(stderr, "Serial Port %s successfully opened.\n", szttyDevice);

    tcgetattr(fdser, &oldtio);      /* save previous port settings */

    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag &= ~PARENB;			/* no parity */
    newtio.c_cflag &= ~CSTOPB;			/* on stop bit */
    newtio.c_cflag &= ~CSIZE;			/* character size mask */
    newtio.c_cflag &= ~HUPCL;			/* no hangup */
    if (bRTSCTS)
        newtio.c_cflag |= ~CRTSCTS;             /* enable hardware flow control */
    else
        newtio.c_cflag &= ~CRTSCTS;             /* disable hardware flow control */
    newtio.c_cflag |= CS8 | CLOCAL | CREAD;	/* 8 bit - ignore modem control lines - enable receiver */
    if (bXonXoff)
        newtio.c_iflag |= IXON | IXOFF;		/* enable XON/XOFF flow control on output & input */
    else {
        newtio.c_iflag &= ~IXON;		/* disable XON/XOFF flow control on output */
        newtio.c_iflag &= ~IXOFF;		/* disable XON/XOFF flow control on input */
    }
    newtio.c_iflag |= IGNBRK | IGNPAR;		/* ignore BREAK condition on input & framing errors & parity errors */
    newtio.c_oflag = 0;	    			/* set serial device input mode (non-canonical, no echo,...) */
    newtio.c_oflag &= ~OPOST;			/* enable output processing */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = yDelay;		/* timeout in 1/10 sec intervals */
    newtio.c_cc[VMIN]     = 0;			/* block until char or timeout */

    if (cfsetospeed (&newtio, B19200)) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "\n%s: %s: Problem setting serial output speed.\n\n",getCurTime(),ProgramName);
        if (bVerbose) fprintf(stderr, "Closing Serial Port %s...",szttyDevice);
        if (close(fdser)) {
            if (bVerbose) fprintf(stderr, "\n");
            fprintf(stderr, "%s: %s: Problem closing serial device.\n",getCurTime(),ProgramName);
        }
        if (bVerbose) { fprintf(stderr, " Success!\n"); }
        ClrSerLock(PID);
        exit(2);
    }
    if (cfsetispeed (&newtio, B19200)) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "\n%s: %s: Problem setting serial input speed.\n\n",getCurTime(),ProgramName);
        if (bVerbose) fprintf(stderr, "Closing Serial Port %s...",szttyDevice);
        if (close(fdser)) {
            if (bVerbose) fprintf(stderr, "\n");
            fprintf(stderr, "%s: %s: Problem closing serial device.\n",getCurTime(),ProgramName);
        }
        if (bVerbose) { fprintf(stderr, " Success!\n"); }
        ClrSerLock(PID);
        exit(2);
    }

    if (bVerbose) { fprintf(stderr, "Configuring serial device... Flushing unread data first... "); }
    errno = 0;
    if (tcflush(fdser, TCIFLUSH)) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "\n%s: %s: Problem flushing serial device: (%i) %s\n\n",getCurTime(),ProgramName,errno,strerror(errno));
    }
    if (tcsetattr(fdser, TCSANOW, &newtio)) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "\n%s: %s: Problem configuring serial device.\n\n",getCurTime(),ProgramName);
        if (bVerbose) fprintf(stderr, "Closing Serial Port %s...",szttyDevice);
        if (close(fdser)) {
            if (bVerbose) fprintf(stderr, "\n");
            fprintf(stderr, "%s: %s: Problem closing serial device.\n",getCurTime(),ProgramName);
        }
        if (bVerbose) { fprintf(stderr, " Success!\n"); }
        ClrSerLock(PID);
        exit(2);
    }

    if (bVerbose) { fprintf(stderr, " Success!\nFlushing serial device buffer..."); }

    errno = 0;
    if (tcflush(fdser, TCIOFLUSH)) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "\n%s: %s: Problem flushing serial device: (%i) %s\n\n",getCurTime(),ProgramName,errno,strerror(errno));
        if (bVerbose) fprintf(stderr, "Closing Serial Port %s...",szttyDevice);
        if (close(fdser)) {
            if (bVerbose) fprintf(stderr, "\n");
            fprintf(stderr, "%s: %s: Problem closing serial device.\n",getCurTime(),ProgramName);
        }
        if (bVerbose) { fprintf(stderr, " Success!\n"); }
        ClrSerLock(PID);
        exit(2);
    }

    if (bVerbose) { fprintf(stderr, " Success!\n"); }

    bCommCheck = TRUE;
    if (bVerbose) { fprintf( stderr, "\nComm Check: Let's see if the Aurora is listening... "); }
    outfp = stderr;
    if (CommCheck(fdser,yAddress) >= 0) {
        if (bVerbose) fprintf(stderr, "Comm Check: OK\n");
    } else {
        if (bVerbose)
             fprintf(stderr, "Comm Check: Failure, aborting...\n");
        else {
             if (bRptReadPause) fprintf(stderr, "\n");
             fprintf(stderr, "%s: %s: No response after %i attempts\n",getCurTime(),ProgramName,yMaxAttempts);
        }
        rc = -1;
    }
    bCommCheck = FALSE;

    if (rc == 0 && bSetTime) {
        rc = SetTime(fdser,yAddress,FALSE);
    }

    if (rc == 0 && yCheckSetTime >= 0) {
        rc = CheckSetTime(fdser,yAddress);
    }

    if (outfile != NULL) {
        if (! (outfp = fopen(outfile, "a"))) {
            fprintf(stderr, "%s: %s: Problem opening output file %s\n",getCurTime(),ProgramName,outfile);
            bFileError = TRUE;
        }
    } else {
        outfp = stdout;
    }

    if (! bFileError) {

        bGetTime = bGetInvTime | bGetLocTime;

        if (rc == 0 && bGetTime) {
            if (!bColumns && !bSetTime) fprintf(outfp, "\n");
            rc |= GetTime(fdser,yAddress);
            if (!bColumns) fprintf(outfp, "\n");
        }

        if (!bColumns) {

            if (rc == 0 && bGetPN)
                rc |= GetPN(fdser,yAddress);

            if (rc == 0 && bGetPNC)
                rc |= GetPNC(fdser,yAddress);

            if (rc == 0 && bGetSN)
                rc |= GetSN(fdser,yAddress);

            if (rc == 0 && bGetVerFW)
                rc |= GetVerFW(fdser,yAddress);

            if (rc == 0 && bGetMfg)
                rc |= GetMfgDate(fdser,yAddress);

            if (rc == 0 && bGetVer)
                rc |= GetVer(fdser,yAddress);

            if (rc == 0 && bGetConf)
                rc |= GetConf(fdser,yAddress);

            if (rc == 0 && bGetJoules)
                rc |= GetJoules(fdser,yAddress);

            if (rc == 0 && bGetState)
                rc |= GetState(fdser,yAddress);

            if (rc == 0 && yGetCount > 0)
                rc |= GetCounters(fdser,yAddress);

            if (rc == 0 && bGetLastAlarms)
                rc |= GetLastAlarms(fdser,yAddress);

            if (rc == 0 && yGetEnergyDaily > 0)
                rc |= GetCEDaily(fdser,yAddress,yGetEnergyDaily);

            if (rc == 0 && yGetEnergySent > 0)
                rc |= GetCESent(fdser,yAddress,yGetEnergySent);

        }

        if (rc == 0 && yGetDSP >= 0)
            rc |= GetDSP(fdser,yAddress);

        if (rc == 0 && bGetEnergy)
            rc |= GetCE(fdser,yAddress);

        if (rc == 0 && bGetDSPExtended)
            rc |= GetDSPExtended(fdser,yAddress);

        if (rc == 0 && bGetDSP3Phase)
            rc |= GetDSP3Phase(fdser,yAddress);

        if (rc == 0 && yGetEnergyCen >= 1)
            rc |= GetCEC(fdser,yAddress,yGetEnergyCen);

    }

    if (rc >= 0) {
        if (bColumns && bColOutput)
            fprintf(outfp, "  OK\n");
        else
            fprintf(outfp, "\n");
        if (bVerbose) fprintf(stderr, "Complete.\n\n");
    } else if (bColumns && bColOutput)
        fprintf(outfp, "\n");

    if (bVerbose && rc >= 0) fprintf(stderr, "rc: %d\n\n",rc);

    if (outfile != NULL && ! bFileError) fclose(outfp);


    /* all done, exit */

    RestorePort(fdser);
    ClrSerLock(PID);

    if (rc < 0) {
        if (bRptRetries) fprintf(stderr, "\n");
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "%s: %s: ERROR: Received bad return code (%i %i",getCurTime(),ProgramName,rc,invOp);
        if (invFunc >= 0)
            fprintf(stderr, " %i)\n",invFunc);
        else
            fprintf(stderr, ")\n");
    }

    endTimeValue = time(NULL);
    timStruct = *(localtime(&endTimeValue));
    strftime(EndTime,sizeof(EndTime),"%Y%m%d-%H:%M:%S",&timStruct);
    EndTime[sizeof(EndTime)-1] = '\0';
    if (bVerbose) fprintf(stderr, "\nComplete %s\n\n",EndTime);

    if (rc >= 0) exit(0);
    exit(1);
}

/*--------------------------------------------------------------------------
    RestorePort
    Restore Serial Port Settings
----------------------------------------------------------------------------*/
int RestorePort(int fdser) {

    if (bVerbose) fprintf(stderr, "\nRestoring Serial Port settings %s...", szttyDevice);
    if (tcsetattr(fdser, TCSANOW, &oldtio)) {		/* restore previous port settings */
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "%s: %s: Problem restoring serial device settings.\n",getCurTime(),ProgramName);
        if (bVerbose) fprintf(stderr, "Closing Serial Port %s...",szttyDevice);
        if (close(fdser)) {
            if (bVerbose) fprintf(stderr, "\n");
            fprintf(stderr, "%s: %s: Problem closing serial device, check device name.\n",getCurTime(),ProgramName);
        }
        if (bVerbose) fprintf(stderr, " Success!\n");
        return 2;
    }

    if (bVerbose) { fprintf(stderr, " Success!\nFlushing serial device buffer..."); }

    errno = 0;
    if (tcflush(fdser, TCIOFLUSH)) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "\n%s: %s: Problem flushing serial device: (%i) %s\n\n",getCurTime(),ProgramName,errno,strerror(errno));
        if (bVerbose) fprintf(stderr, "Closing Serial Port %s...",szttyDevice);
        if (close(fdser)) {
            if (bVerbose) fprintf(stderr, "\n");
            fprintf(stderr, "%s: %s: Problem closing serial device.\n",getCurTime(),ProgramName);
        }
        if (bVerbose) { fprintf(stderr, " Success!\n"); }
        return 2;
    }

    if (bVerbose) fprintf(stderr, " Success!\nClosing Serial Port %s...", szttyDevice);

    if (close(fdser)) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "%s: %s: Problem closing serial device.\n",getCurTime(),ProgramName);
        return 2;
    }
    if (bVerbose) fprintf(stderr, " Success!\n");
    return 0;
}


/*--------------------------------------------------------------------------
    ClrSerLock
    Clear Serial Port lock.
----------------------------------------------------------------------------*/
int ClrSerLock(long unsigned int LckPID) {
    FILE *fdserlck, *fdserlcknew;
    long unsigned int PID;
    int bWrite, bRead;
    int errno_save = 0;
    int fLen = 0;
    int cmdLen = 0;
    int curChar = 0;
    char *COMMAND = NULL;

    errno = 0;
    if (bVerbose) fprintf(stderr, "\ndevLCKfile: <%s>\ndevLCKfileNew: <%s>\nClearing Serial Port Lock (%lu)...", devLCKfile, devLCKfileNew, LckPID);

    fdserlck = fopen(devLCKfile, "r");
    if (fdserlck == NULL) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "\n%s: %s: Problem opening serial device lock file to clear LckPID %lu: %s for read.\n\n",getCurTime(),ProgramName,LckPID,devLCKfile);
        return(0);
    }
    fdserlcknew = fopen(devLCKfileNew, "w");
    if (fdserlcknew == NULL) {
        if (bVerbose) fprintf(stderr, "\n");
        fprintf(stderr, "\n%s: %s: Problem opening new serial device lock file to clear LckPID %lu: %s for write.\n\n",getCurTime(),ProgramName,LckPID,devLCKfileNew);
        fclose(fdserlck);
        return(0);
    }

    curChar = 0;
    while (curChar != EOF) {
        fLen = 0;
        while ((curChar = fgetc(fdserlck)) != EOF && curChar != '\n' && curChar != ' ') fLen++;
        fLen = 0;
        while ((curChar = fgetc(fdserlck)) != EOF && curChar != '\n') fLen++;
        if (fLen > cmdLen) cmdLen = fLen;
    }
    rewind(fdserlck);

    COMMAND = getMemPtr(cmdLen+1);
    COMMAND[0] = '\0';
    bRead = fscanf(fdserlck, "%lu %s", &PID, COMMAND);

    while (bRead != EOF) {
        if (PID != LckPID) {
            errno = 0;
            bWrite = fprintf(fdserlcknew, "%lu %s\n", PID, COMMAND);
            errno_save = errno;
            if (bWrite < 0 || errno_save != 0) {
                fprintf(stderr, "\n%s: %s: Problem clearing serial device lock, can't write lock file: %s.\n%s\n\n",getCurTime(),ProgramName,devLCKfile,strerror (errno_save));
                fclose(fdserlcknew);
                return(0);
            }
        }
        bRead = fscanf(fdserlck, "%lu %s", &PID, COMMAND);
    }
    fclose(fdserlck);
    fclose(fdserlcknew);
    free(COMMAND);
    errno = 0;
    if (rename(devLCKfileNew,devLCKfile)) fprintf(stderr, "\n%s: %s: Problem clearing serial device lock, can't update lock file: %s.\n%s\n\n",getCurTime(),ProgramName,devLCKfile,strerror (errno));
    if (bVerbose) fprintf(stderr, " done.\n");

    return -1;
}


/*--------------------------------------------------------------------------
    isNumeric
----------------------------------------------------------------------------*/
BOOL isNumeric(char *p)
{
    int i;

    for (i = 0; i <= strlen(p); i++)
        if (p[i] != '\0' && ! isdigit(p[i])) return(FALSE);
    return(TRUE);
}

/*--------------------------------------------------------------------------
    GetParms
    Reads command line parameters.
----------------------------------------------------------------------------*/
int GetParms(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int c;
    int i = 0;
    BOOL b = FALSE;
    char *pos;
    char *SubStrPos = NULL;
    char sPID[10];
    static char *Cost = NULL;

    if (strpbrk(VersionM,"abcdefghijklmnopqurtsuvwxyz") != NULL) fprintf(stderr, "\n**** THIS IS EXPERIMENTAL CODE %-6s ****\n",VersionM);
    if (strcmp(VersionM,VersionC) != 0) {
        fprintf(stderr, "\n**** MODULE VERSION MISMATCH ****\n");
        fprintf(stderr, "      Main module : %-6s\n",VersionM);
        fprintf(stderr, "      Comm module : %-6s\n",VersionC);
        return 0;
    }

    while (! b && i < argc) if (strcmp(argv[i++],"-b") == 0) b = TRUE;

    if (b) {
        fprintf(stderr, "\n");
        i = 0;
        while (i < argc) fprintf(stderr, "%s ",argv[i++]);
        fprintf(stderr, "\n");
    }

     /* options descriptor */
    static struct option longopts[] = {
        { "address",			required_argument,	0,	'a' },
	{ "last-alarms",		no_argument,		0,	'A' },
        { "verbose",			no_argument,		0,	'b' },
	{ "calc-value",			required_argument,	0,	'C' },
        { "columnize",			no_argument,		0,	'c' },
        { "get-dsp",			optional_argument,	0,	'd' },
        { "get-dsp-extended",		no_argument,		0,	'D' },
        { "get-dsp-3phase",		no_argument,		0,	'3' },
        { "get-energy",			no_argument,		0,	'e' },
        { "get-energy-central",		optional_argument,	0,	'E' },
        { "firmware-ver",		no_argument,		0,	'f' },
        { "mfg-date",			no_argument,		0,	'g' },
        { "hide-dsp-msg",		no_argument,		0,	'H' },
        { "help",			no_argument,		0,	'h' },
        { "get-count",			optional_argument,	0,	'i' },
        { "get-joules",			no_argument,		0,	'j' },
        { "use-kmult",			required_argument,	0,	'K' },
        { "daily-kwh",			optional_argument,      0,      'k' },
        { "adjust-time",		required_argument,	0,	'L' },
	{ "read-wait",			required_argument,	0,	'l' },
        { "max-runtime",		required_argument,	0,	'M' },
        { "get-conf",			no_argument,		0,      'm' },
        { "central",			required_argument,	0,	'N' },
        { "serial-number",		no_argument,		0,	'n' },
        { "part-number-central",	no_argument,		0,	'O' },
        { "output-file",		required_argument,      0,      'o' },
        { "part-number",		no_argument,		0,	'p' },
        { "comm-pause",			required_argument,	0,	'P' },
        { "use-qmult",			no_argument,		0,	'Q' },
        { "energy-sent",		optional_argument,	0,	'q' },
        { "read-timeout",		required_argument,	0,	'R' },
        { "calc-grid-power",		no_argument,		0,	'r' },
        { "set-time",			no_argument,		0,	'S' },
        { "get-state",			no_argument,		0,	's' },
        { "get-invtime",		no_argument,		0,      't' },
        { "get-loctime",		no_argument,		0,	'T' },
        { "version",			no_argument,		0,	'V' },
        { "inv-version",		no_argument,		0,	'v' },
	{ "read-pause",			required_argument,	0,	'U' },
	{ "rpt-read-pause",		no_argument,		0,	'u' },
	{ "swap-endian",		no_argument,		0,	'W' },
        { "lock-wait",			required_argument,	0,	'w' },
        { "rts-cts",			no_argument,		0,	'X' },
	{ "xon-xoff",			no_argument,		0,	'x' },
	{ "retries",			required_argument,	0,	'Y' },
	{ "rpt-retries",		no_argument,		0,	'y' },
        { NULL,				0,				NULL,   0 }
    };

    /* Set command line defaults */
    yDelay = 1;

    if (argc == 1)
        return 0;				/* no parms at all */

    while ((c = getopt_long(argc, argv, "3a:AbC:cDd:E:efgHhi:jK:k:L:l:M:mN:nOo:P:pQq:R:rSsTtVvU:uWw:XxY:y", longopts, NULL )) != EOF) {
        switch (c) {
            case 'a':
                /* Inverter address */
                yAddress = atoi(optarg);
                break;
            case 'A': bGetLastAlarms     = TRUE; break;
            case 'b': bVerbose     = TRUE; break;
            case 'C':
                SubStrPos = strstr(optarg, ":");
                if (SubStrPos == NULL)
                    yCost = atof(optarg);
                else {
                    Cost = getMemPtr(SubStrPos-optarg);
                    strncpy(Cost,optarg,SubStrPos-optarg);
                    Cost[strlen(Cost)] = '\0';
                    yCost = atof(Cost);
                    sCostType = getMemPtr(strlen(optarg)-(SubStrPos-optarg));
                    strcpy(sCostType,SubStrPos+1);
                    sCostType[strlen(sCostType)] = '\0';
                    free(Cost);
                    Cost = NULL;
                }
                if (bVerbose) fprintf(stderr, "\nCost: %f Type: \"%s\"\n",yCost,sCostType);
                break;
            case 'c': bColumns     = TRUE; break;
            case 'd':
                if (optarg != NULL && ! isNumeric(optarg)) {
                    fprintf(stderr, "\n%s: %s: -d value is not numeric, 0 to 2.\n",getCurTime(),ProgramName);
                    return 0;
                }
                if (optarg == NULL) {
                    yGetDSP = 0;
                } else {
                    i = atoi(optarg);
                    if (i < 0 || i > 2) {
                        fprintf(stderr, "\n%s: %s: -d value out of range, 0 to 2.\n",getCurTime(),ProgramName);
                        return 0;
                    }
                    yGetDSP = i;
                }
                break;
            case 'D': bGetDSPExtended = TRUE; break;
            case '3': bGetDSP3Phase   = TRUE; break;
            case 'e': bGetEnergy      = TRUE; break;
            case 'E':
                if (optarg != NULL && ! isNumeric(optarg)) {
                    fprintf(stderr, "\n%s: %s: -d value is not numeric, 1 to 366.\n",getCurTime(),ProgramName);
                    return 0;
                }
                if (optarg == NULL) {
                    yGetDSP = 366;
                } else {
                    i = atoi(optarg);
                    if (i < 1 || i > 366) {
                        fprintf(stderr, "\n%s: %s: -d value out of range, 1 to 366.\n",getCurTime(),ProgramName);
                        return 0;
                    }
                    yGetEnergyCen = i;
                }
                break;
            case 'f': bGetVerFW       = TRUE; break;
            case 'g': bGetMfg         = TRUE; break;
            case 'H': bHideDSP        = TRUE; break;
            case 'h': bHelp           = TRUE; break;
            case 'i': 
                if (optarg == NULL) {
                    yGetCount = 0x0f;
                } else {
                    i = atoi(optarg);
                    if (i < 1 || i > 15) {
                        fprintf(stderr, "\n%s: %s: -i value out of range, 1 (0x01) to 15 (0x0f).\n",getCurTime(),ProgramName);
                        return 0;
                    }
                    yGetCount = i;
                }

                break;
            case 'j': bGetJoules   = TRUE; break;
            case 'K': yMultiplierK = atof(optarg);
                if (yMultiplierK <= 0) {
                    fprintf(stderr, "\n%s: %s: -K value out of range, > 0.\n",getCurTime(),ProgramName);
                    return 0;
                }
                break;
            case 'k':
                /* Get ECC */
                if (optarg == NULL) {
                    yGetEnergyDaily = 366;
                } else {
                    i = atoi(optarg);
                    if (i < 1 || i > 366) {
                        fprintf(stderr, "\n%s: %s: -k value out of range, 1 to 366.\n",getCurTime(),ProgramName);
                        return 0;
                    }
                    yGetEnergyDaily = i;
                }
                break;
            case 'L':
                i = atoi(optarg);
                if (i < 0) {
                    fprintf(stderr, "\n%s: %s: -L value out of range, 0 or >=1 0 = check for DST change >=1 = Reconcile with computer time\n",getCurTime(),ProgramName);
                    return 0;
                }
                yCheckSetTime = i;
                break;
            case 'l':
                /* Get delay time */
                i = atoi(optarg);
                if (i < 0 || i > 255) {
                    fprintf(stderr, "\n%s: %s: -l Illegal read wait specified.\n",getCurTime(),ProgramName);
                    return 0;
                }
                yDelay = (unsigned char)i;
                break;
            case 'M':
                i = atoi(optarg);
                if (i < 0) {
                    fprintf(stderr, "\n%s: %s: -l Illegal max runtime  specified.\n",getCurTime(),ProgramName);
                    return 0;
                }
                yMaxRunTime = i;
                break;
            case 'm': bGetConf     = TRUE; break;
            case 'n': bGetSN       = TRUE; break;
            case 'N':
                i = atoi(optarg);
                if (i < 0 || i > 1) {
                    fprintf(stderr, "\n%s: %s: -l Illegal Master/Slave  specified.\n",getCurTime(),ProgramName);
                    return 0;
                }
                yCentral = i;
                break;
            case 'O': bGetPNC      = TRUE; break;
            case 'o':
                outfile = getMemPtr(strlen(optarg)+1); 
                strcpy(outfile, optarg);
                outfile[strlen(outfile)] = '\0';
                break;
            case 'p': bGetPN       = TRUE; break;
            case 'P':
                yCommPause = atoi(optarg);
                if (yCommPause <= 0 || yCommPause > 1000000) {
                    fprintf(stderr, "\n%s: %s: -P Comm Pause micro-seconds (%li) out of range, 1-1000000.\n",getCurTime(),ProgramName,yCommPause);
                    return 0;
                }
                break;
            case 'Q': bUseMultiplierQ = TRUE; break;
            case 'q':
                /* Get Energy Sent */
                if (optarg == NULL) {
                    yGetEnergySent = 4320;
                } else {
                    i = atoi(optarg);
                    if (i < 1 || i > 1440) {
                        fprintf(stderr, "\n%s: %s: -k value out of range, 1 to 1440.\n",getCurTime(),ProgramName);
                        return 0;
                    }
                    yGetEnergySent = i*3;
                }
                break;
            case 'R':
                /* read timeout value in mS */
                i = atoi(optarg);
                if (i < 200) {
                    fprintf(stderr, "\n%s: %s: -R value out of range, minimum 200.\n",getCurTime(),ProgramName);
                    return 0;
                }
                yTimeout = i;
                break;
            case 'r': bCalcGridPwr = TRUE; break;
            case 'S': bSetTime     = TRUE; break;
            case 's': bGetState    = TRUE; break;
            case 'T': bGetLocTime  = TRUE; break;
            case 't': bGetInvTime  = TRUE; break;
            case 'U':
                yReadPause = atoi(optarg);
                if (yReadPause < 1 || yReadPause > 10000) {
                    fprintf(stderr, "\n%s: %s: -U Read Pause milli-seconds (%d) out of range, 1-10000.\n",getCurTime(),ProgramName,yReadPause);
                    return 0;
                }
                break;
            case 'u': bRptReadPause	= TRUE; break;
            case 'W': bSwapEndian	= TRUE; break;
            case 'w':
                yLockWait = atof(optarg);
                if (yLockWait < (uSecsLCKWaitMin/1000.0) || yLockWait > 30000.0) {
                    fprintf(stderr, "\n%s: %s: -w Lock Wait milliseconds (%.3f) out of range, %.3f-30000.0 mS.\n",getCurTime(),ProgramName,yLockWait,(float)(uSecsLCKWaitMin/1000.0));
                    return 0;
                }
                break;
            case 'X': bRTSCTS      = TRUE; break;
            case 'x': bXonXoff     = TRUE; break;
            case 'Y':
                yMaxAttempts = atoi(optarg);
                if (yMaxAttempts < 1 || yMaxAttempts > 100) {
                    fprintf(stderr, "\n%s: %s: -Y Retries (%d) out of range, 1-100.\n",getCurTime(),ProgramName,yMaxAttempts);
                    return 0;
                }
                break;
            case 'y': bRptRetries   = TRUE; break;
            case 'V': bVersion     = TRUE; break;
            case 'v': bGetVer      = TRUE; break;

            case '?': /* user entered unknown option */
            case ':': /* user entered option without required value */
                return 0;

            default:
                break;
        }
    }
    if (optind < argc) { 			/* get serial device name */
        szttyDevice = getMemPtr(strlen(argv[optind])+1);
        strcpy(szttyDevice, argv[optind]);
        szttyDevice[strlen(szttyDevice)] = '\0';
     } else {
        if (!bVersion && ! bHelp) fprintf(stderr, "\n%s: %s: No serial device specified\n",getCurTime(),ProgramName);
        return 0;
    }
    pos = strrchr(szttyDevice, '/');
    if (pos > 0) {
        pos++;
        devLCKfile = getMemPtr(strlen(ttyLCKloc)+(strlen(szttyDevice)-(pos-szttyDevice))+1);
        devLCKfile[0] = '\0';
        strcpy(devLCKfile,ttyLCKloc);
        strcat(devLCKfile, pos);
        devLCKfile[strlen(devLCKfile)] = '\0';
        sprintf(sPID,"%lu",PID);
        devLCKfileNew = getMemPtr(strlen(devLCKfile)+strlen(sPID)+2);	/* dot & terminator */
        devLCKfileNew[0] = '\0';
        strcpy(devLCKfileNew,devLCKfile);
        sprintf(devLCKfileNew,"%s.%lu",devLCKfile,PID);
        devLCKfileNew[strlen(devLCKfileNew)] = '\0';
    } else {
        devLCKfile = NULL;
    }
    if (bVerbose) {
        fprintf(stderr, "\nszttyDevice: %s\nyDelay:     %i\nyTimeout    %i mS\nyMaxRunTime ",szttyDevice,yDelay,yTimeout);
        yMaxRunTime ? fprintf(stderr, "%i S",yMaxRunTime) : fprintf(stderr, "~");
        fprintf(stderr, "\ndevLCKfile: <%s>\ndevLCKfileNew: <%s>\n",devLCKfile,devLCKfileNew);
    }
    if (bSetTime && yCheckSetTime >= 0) {
        fprintf(stderr, "\n%s: %s: -L and -S are mutually exclusive.\n",getCurTime(),ProgramName);
        return 0;
    }
    if (yAddress < 1 || yAddress > 63) {
        fprintf(stderr, "\n%s: %s: Illegal address (%d) specified.\n",getCurTime(),ProgramName,yAddress);
        return 0;
    }
    if (bVerbose) fprintf(stderr, "Got Params\n");
    return -1;
}


/*--------------------------------------------------------------------------
    rnduSecs
    randomize lock wait time
----------------------------------------------------------------------------*/
int rnduSecs() {
    float rnumf = 0.0;
    int waitusecs = 0;

    rnumf = (float)rand();
    while (rnumf > 1) rnumf = rnumf/10.0;
    waitusecs = uSecsLCKWaitMax * rnumf;;
    if (waitusecs <= 1) waitusecs = uSecsLCKWaitMin;
    while (waitusecs < uSecsLCKWaitMin) waitusecs = waitusecs * 1.29;

    return(waitusecs);
}


/*--------------------------------------------------------------------------
        getMemPtr
----------------------------------------------------------------------------*/
void *getMemPtr(size_t mSize)
{
    void *ptr;
    char *cptr;
    int i;

    ptr = malloc(mSize);
    if (!ptr) {
        fprintf(stderr, "\nvproweather: malloc failed\n");
        exit(2);
    }
    cptr = (char *)ptr;
    for (i = 0; i < mSize; i++) cptr[i] = '\0';
    return ptr;
}


/*--------------------------------------------------------------------------
    Version
    Display program component versions
----------------------------------------------------------------------------*/
void Version()
{
    printf("\nAurora module versions:\n");
    printf("Main module : %-6s\n",VersionM);
    printf("Comm module : %-6s\n",VersionC);
    printf("main.h      : %-6s\n",VersionMHc);
    printf("comm.h      : %-6s\n",VersionCHc);
    printf("names.h     : %-6s\n",VersionNHc);
    printf("states.h    : %-6s\n\n",VersionSHc);
}
 
