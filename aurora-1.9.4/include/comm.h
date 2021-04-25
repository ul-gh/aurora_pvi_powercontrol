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

#define	VersionCH	"1.8.7"

extern int CommCheck(int fdser, int yAddress);
extern int GetState(int fdser, int yAddress);
extern int GetPN(int fdser, int yAddress);
extern int GetPNC(int fdser, int yAddress);
extern int GetVer(int fdser, int yAddress);
extern int GetSN(int fdser, int yAddress);
extern int GetVerFW(int fdser, int yAddress);
extern int GetMfgDate(int fdser, int yAddress);
extern int GetConf(int fdser, int yAddress);
extern int GetDSP(int fdser, int yAddress);
extern int GetDSPExtended(int fdser, int yAddress);
extern int GetDSP3Phase(int fdser, int yAddress);
extern int GetCE(int fdser, int yAddress);
extern int GetCEC(int fdser, int yAddress, int yGetEnergyCen);
extern int GetCESent(int fdser, int yAddress, int yGetEnergySent);
extern int GetCEDaily(int fdser, int yAddress, int yGetEnergyDaily);
extern int GetJoules(int fdser, int yAddress);
extern int GetCounters(int fdser, int yAddress);
extern int GetTime(int fdser, int yAddress);
extern int SetTime(int fdser, int yAddress, BOOL force);
extern int CheckSetTime(int fdser, int yAddress);
extern int GetLastAlarms(int fdser, int yAddress);

extern char     VersionC[];
extern char	VersionSHc[6];

extern int invOp;
extern int invFunc;

#define _clearCMD       "\0\0\0\0\0\0\0\0\0\0\0"
#define _clrAttemps	1000

#define _szSerBufferLen	11

#define TimeBase	946684800

#define SecsInMinute	60
#define SecsInHour	(60*SecsInMinute)
#define SecsInDay	(24*SecsInHour)
#define	SecsInYear	(365*SecsInDay)

