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

#define	VersionNH	"1.8.7"


/* # 50 State Request */

#define	opGetState	50
#define _opGetState "State Request"


/* # 52 P/N Reading */

#define opGetPN		52
#define _opGetPN "P/N Reading"

#define	aChar6	0
#define aChar5  1
#define aChar4  2
#define aChar3  3
#define aChar2  4
#define aChar1  5


/* # 58 Version Reading */

#define opGetVer	58
#define _opGetVer "Version Reading"

#define aPar1	2
#define aPar1i	"PVI-2000"
#define aPar1o	"PVI-2000-OUTD"
#define aPar1I	"PVI-3600"
#define aPar1O	"PVI-3.6-OUTD"
#define aPar15	"PVI-5000-OUTD"
#define aPar16	"PVI-6000-OUTD"
#define aPar1P	"3-phase interface (3G74)"
#define aPar1C	"PVI-CENTRAL-50 module"
#define aPar14	"PVI-4.2-OUTD"
#define aPar13	"PVI-3.6-OUTD"
#define aPar12	"PVI-3.3-OUTD"
#define aPar11	"PVI-3.0-OUTD"
#define aPar1D	"PVI-12.5-OUTD"
#define aPar1X	"PVI-10.0-OUTD"
#define aPar1H	"PVI-4.6-I-OUTD"
#define aPar1h	"PVI-3.8-I-OUTD"
#define aPar1T	"PVI-12.0-I-OUTD (output 480 VAC)"
#define aPar1t	"PVI-10.0-I-OUTD (output 480 VAC)"
#define aPar1U	"PVI-12.0-I-OUTD (output 208 VAC)"
#define aPar1u	"PVI-10.0-I-OUTD (output 208 VAC)"
#define aPar1V	"PVI-12.0-I-OUTD (output 380 VAC)"
#define aPar1v	"PVI-10.0-I-OUTD (output 380 VAC)"
#define aPar1Z	"PVI-12.0-I-OUTD (output 600 VAC)"
#define aPar1z	"PVI-10.0-I-OUTD (output 600 VAC)"
#define aPar1M	"PVI-CENTRAL-250"
#define aPar1w	"PVI-10.0-I-OUTD (output 480 VAC current limit 12 A)"
#define aPar1Y	"TRIO-27.6-TL-OUTD"
#define aPar1y	"TRIO-20-TL"
#define aPar1g	"UNO-2.0-I"
#define aPar1G	"UNO-2.5-I"
#define aPar1L	"PVI-CENTRAL-350 Liquid Cooled (control board)"
#define aPar1B	"PVI-CENTRAL-350 Liquid Cooled (display board)"
#define aPar1A	"PVI-CENTRAL-350 Liquid Cooled (AC gathering)"

#define aPar2   3
#define aPar2A	"USA - UL1741"
#define aPar2E	"Germany - VDE0126"
#define aPar2S	"Spain - DR 1663/2000"
#define aPar2I	"Italy - ENEL DK 5950"
#define aPar2U	"UK - UK G83"
#define aPar2K	"Australia - AS 4777"
#define aPar2F	"France - VDE French Model"
#define aPar2R	"Ireland - EN50438"
#define aPar2B	"Belgium - VDE Belgium Model"
#define aPar2O	"Korea"
#define aPar2G	"Greece - VDE Greece Model"
#define aPar2T	"Taiwan"
#define aPar2C	"Czech Republic"
#define aPar2Q	"People’s Republic of China"
#define aPar2a	"USA - UL1741 Vout = 208 single phase"
#define aPar2b	"USA - UL1741 Vout = 240 single phase"
#define aPar2c	"USA - UL1741 Vout = 277 single phase"
#define aPar2X	"Debug Standard 1"
#define aPar2x	"Debug Standard 2"
#define aPar2u	"UK – UK-G59"
#define aPar2k	"Israel – Derived from AS 4777"
#define aPar2W	"Germany – BDEW"
#define aPar2H	"Hungary"
#define aPar2o	"Corsica"
#define aPar2P	"Portugal"
#define aPar2e	"VDE AR-N-4105"

#define aPar3   4
#define aPar3T	"Transformer Version"
#define aPar3N	"Transformerless Version"
#define aPar3t	"Transformer HF version"
#define aPar3X	"Dummy transformer type"

#define aPar4   5
#define aPar4W	"Wind Version"
#define aPar4N	"Photovoltaic Version"
#define aPar4X "Dummy inverter type"

/* # 59 Measure request to the DSP - Type of Measure */

#define	opGetDSP	59
#define _opGetDSP "Measure request to the DSP"

#define	ToM1	1		/* Grid Voltage (All) */
#define ToM2	2		/* Grid Currrent (All) */
#define ToM3	3		/* Grid Power (All) */
#define ToM4	4		/* Frequency All) */
#define ToM5	5		/* VBulk was Ileak (Dc/Dc) reading All) */
#define ToM6	6		/* Ileak (Dc/Dc) was Ileak (Inverter) Reading*/
#define ToM7	7		/* Ileak (Inverter) */
#define ToM8	8		/* Pin 1 (All) */
#define ToM9	9		/* Pin 2 */


#define ToM21	21		/* Inverter Temperature (Grid-Tied) */
#define ToM22	22		/* Booster Temperatuer (Grid-Tied) */
#define ToM23	23		/* Input 1 Voltage */
#define ToM25	25		/* Input 1 Current (All) */
#define ToM26	26		/* Input 2 Voltage (Grid-Tied) */
#define ToM27	27		/* Input 2 Current (Grid-Tied) */
#define ToM28	28		/* Grid Voltage (Dc/Dc) (Grid-Tied) */
#define ToM29	29		/* Grid Frequency (Dc/Dc) (Grid-Tied) */
#define ToM30	30		/* Isolation Resistance (Riso) (All) */
#define ToM31	31		/* Vbulk (Dc/Dc) (Grid-Tied) */
#define ToM32	32		/* Average Grid Voltage (VgridAvg) (Grid-Tied) */
#define ToM33	33		/* Vbulk Mid (Grid-Tied) */
#define ToM34	34		/* Power Peak (All) */
#define ToM35	35		/* Power Peak Today (All) */
#define ToM36	36		/* Grid Voltage neutral (Grid-Tied) */
#define ToM37	37		/* Wind Generator Frequency */
#define ToM38	38		/* Grid Voltage neutral-phase (Central) */
#define ToM39	39		/* Grid Current phase r (Central & 3 Phase) */
#define ToM40	40		/* Grid Current phase s (Central & 3 Phase) */
#define ToM41	41		/* Grid Current phase t (Central & 3 Phase) */
#define ToM42	42		/* Frequency phase r (Central & 3 Phase) */
#define ToM43	43		/* Frequency phase s (Central & 3 Phase) */
#define ToM44	44		/* Frequency phase t (Central & 3 Phase) */
#define ToM45	45		/* Vbulk + (Central & 3 Phase) */
#define ToM46	46		/* Vbulk - (Central) */
#define ToM47	47		/* Supervisor Temperature (Central) */
#define ToM48	48		/* Alim Temperature (Central) */
#define ToM49	49		/* Heak Sink Temperature (Central) */
#define ToM50	50		/* Temperature 1 (Central) */
#define ToM51	51		/* Temperature 2 (Central) */
#define ToM52	52		/* Temperature 3 (Central) */
#define ToM53	53		/* Fan 1 Speed (Central) */
#define ToM54	54		/* Fan 2 Speed (Central) */
#define ToM55	55		/* Fan 3 Speed (Central) */
#define ToM56	56		/* Fan 4 Speed (Central) */
#define ToM57	57		/* Fan 5 Speed (Central) */
#define ToM58	58		/* Power Saturation limit (Der.) (Central) */
#define ToM59	59		/* Reference Ring Bulk (Central) */
#define ToM60	60		/* Vpanel micro (Central) */
#define ToM61	61		/* Grid Voltage phase r (Central & 3 Phase) */
#define ToM62	62		/* Grid Voltage phase s (Central & 3 Phase) */
#define ToM63	63		/* Grid Voltage phase t (Central & 3 Phase) */
#define ToM95	95		/* Fan 1 Speed (rpm) (Central) */
#define ToM96	96		/* Fan 2 Speed (rpm) (Central) */
#define ToM97	97		/* Fan 3 Speed (rpm) (Central) */
#define ToM98	98		/* Fan 4 Speed (rpm) (Central) */
#define ToM99	99		/* Fan 5 Speed (rpm) (Central) */
#define ToM100	100		/* Fan 6 Speed (rpm) (Central) */
#define ToM101	101		/* Fan 7 Speed (rpm) (Central) */


#define _ToM1	"Grid Voltage Reading "		/* Global */
#define _ToM2	"Grid Current Reading "		/* Global */
#define _ToM3	"Grid Power Reading "		/* Global */
#define _ToM3C	"Grid Power Calculated"
#define _ToM4	"Frequency Reading"
#define _ToM5	"Vbulk"				/* was"Ileak (Dc/Dc) Reading" */
#define _ToM6	"Ileak (Dc/Dc) Reading"		/* was "Ileak (Inverter) Reading" */
#define _ToM7	"Ileak (Inverter) Reading"
#define _ToM8	"Pin 1 "			/* Global */
#define _ToM9	"Pin 2"

#define _ToM21	"Inverter Temperature"
#define _ToM22	"Booster Temperature"		/* was Environment Temperature */
#define _ToM23	"Input 1 Voltage"		/* Global was "West String Voltage" */
#define _ToM25	"Input 1 Current"		/* was "West String Current" */
#define _ToM26	"Input 2 Voltage"		/* was "East String Voltage" */
#define _ToM27	"Input 2 Current"		/* was "East String Current" */
#define _ToM28	"Grid Voltage (Dc/Dc)"
#define _ToM29	"Grid Frequency (Dc/Dc)"
#define _ToM30	"Isolation Resistance (Riso)"
#define _ToM31	"Vbulk (Dc/Dc)"
#define _ToM32	"Grid Voltage Average (VgridAvg)"
#define _ToM33	"Vbulk Mid"
#define _ToM34	"Power Peak"
#define _ToM35	"Power Peak Today"
#define _ToM36	"Grid Voltage neutral"
#define _ToM37	"Wind Generator Frequency"
#define _ToM38	"Grid Voltage neutral-phase"
#define _ToM39	"Grid Current phase r"
#define _ToM40	"Grid Current phase s"
#define _ToM41	"Grid Current phase t"
#define _ToM42	"Frequency phase r"
#define _ToM43	"Frequency phase s"
#define _ToM44	"Frequency phase t"
#define _ToM45	"Vbulk +"
#define _ToM46	"Vbulk -"
#define _ToM47	"Supervisor Temperature"
#define _ToM48	"Alim Temperature"
#define _ToM49	"Heak Sink Temperature"
#define _ToM50	"Temperature 1"
#define _ToM51	"Temperature 2"
#define _ToM52	"Temperature 3"
#define _ToM53	"Fan 1 Speed"
#define _ToM54	"Fan 2 Speed"
#define _ToM55	"Fan 3 Speed"
#define _ToM56	"Fan 4 Speed"
#define _ToM57	"Fan 5 Speed"
#define _ToM58	"Power Saturation limit (Der.)"
#define _ToM59	"Reference Ring Bulk"
#define _ToM60	"Vpanel micro"
#define _ToM61	"Grid Voltage phase r"
#define _ToM62	"Grid Voltage phase s"
#define _ToM63	"Grid Voltage phase t"
#define _ToM95  "Fan 1 RPM"
#define _ToM96  "Fan 2 RPM"
#define _ToM97  "Fan 3 RPM"
#define _ToM98  "Fan 4 RPM"
#define _ToM99  "Fan 5 RPM"
#define _ToM100 "Fan 6 RPM"
#define _ToM101 "Fan 7 RPM"

#define _Str1P	"Input 1 Power"			/* was "West String Power" */
#define _Str2P  "Input 2 Power"			/* was "East String Power" */

#define _DcAcEff	"DC/AC Conversion Efficiency"


/* # 63 Serial Number */

#define	opGetSN		63
#define _opGetSN "Serial Number"


/* # 65 Manufacturing Week and Year */

#define opGetMfg	65
#define _opGetMfg "Manufacturing Week and Year"

#define	aWeekH	2
#define aWeekL	3
#define aYearH	4
#define aYearL	5


/* # 68 Cumulated Float Energy (Central & TRIO only)*/

#define opGetCEC         68
#define _opGetCEC "Cumulated Float Energy"

#define CECpar1  1               /* Daily Energy */
#define CECpar2  2               /* Weekly Energy */
#define CECpar3  3               /* Monthly Energy */
#define CECpar4  4               /* Yearly Energy */
#define CECpar5  5               /* nDays Energy */
#define CECpar6  6               /* Total Energy (total lifetime) */
#define CECpar7  7               /* Partial Energy (cumulated since reset) */

#define _CECpar1  "Daily Energy"
#define _CECpar2  "Weekly Energy"
#define _CECpar3  "Monthly Energy"
#define _CECpar4  "Yearly Energy"
#define _CECpar5  "Days Energy"
#define _CECpar6  "Total Energy"
#define _CECpar7  "Partial Energy"


/* # 70 0x46 Get Time/Date */

#define opGetTime	70
#define _opGetTime "Get Time/Date"


/* # 71 Set Time/Date */

#define opSetTime	71
#define _opSetTime "Set Time/Date"


/* # 72 Firmware Release */

#define opGetVerFW	72
#define _opGetVerFW "Firmware Release"

#define aRel3	2
#define aRel2   3
#define aRel1   4
#define aRel0   5


/* # 75 Energy delivred to grip every 10 seconds ** Experimental ** */

#define opGetCESent	0x4b01  /* 75 01 */
#define _opGetCESent "Energy Sent To Grid"

#define aCESMemAdd	0x000a
#define aCESMaxCnt	4320


/* # 75 Energy cumulated each day address ** Experimental ** */

#define opGetCEAdd	0x4b02	/* 75 02 */
#define _opGetCEAdd "Daily Cumulated Energy Address"

#define aCEAddH		2
#define aCEAddL		3


/* # 76 Energy cumulated in the last 10 seconds */

#define	opGetEnergy10Sec	76
#define _opGetEnergy10Sec "Energy cumulated in the last 10 seconds"

#define	aEnergyH	2
#define	aEnergyL	3


/* # 77 System Configuration */

#define opGetConfig	77
#define _opGetConfig "System Configuration"

#define	aConfCode	2

#define	ConfCode0	0
#define ConfCode1	1
#define ConfCode2	2

#define _ConfCode0	"System operating with both strings."
#define	_ConfCode1	"String 1 connected, String 2 disconnected."
#define	_ConfCode2	"String 2 connected, String 1 disconnected."


/* # 78 Cumulated Energy */

#define	opGetCE		78
#define _opGetCE "Cumulated Energy"

#define CEpar0	0		/* Daily Energy */
#define CEpar1  1		/* Weekly Energy */
#define CEpar2  2		/* Energy of last 7 days */
#define CEpar3  3		/* Monthly Energy */
#define CEpar4  4		/* Yearly Energy */
#define CEpar5  5		/* Total Energy (total lifetime) */
#define CEpar6  6		/* Partial Energy (cumulated since reset) */

#define _CEpar0  "Daily Energy"
#define _CEpar1  "Weekly Energy"
#define _CEpar2  "Energy last 7 days"
#define _CEpar3  "Monthly Energy"
#define _CEpar4  "Yearly Energy"
#define _CEpar5  "Total Energy"
#define _CEpar6  "Partial Energy"


/* # 79 Daily Cumulated Energy ** Experimental ** */

#define opGetCEValue	79
#define _opGetCEValue "Cumulated Energy"

#define cCEDailyAddH	2
#define cCEDailyAddL	3
#define cCEDailyEnd	4

#define aCEDailyDaysH	2
#define aCEDailyDaysL	3
#define aCEDailyValH	4
#define aCEDailyValL	5


/* # 80 Time Counter */

#define	opGetCounters	80
#define _opGetCounters "Time Counter"

#define	cTotalRun	0
#define cPartialRun	1
#define cTotalGrid	2
#define cResetPartial	3

#define _cTotalRun	"Total Running Time (Lifetime)"
#define _cPartialRun	"Partial Running Time (since reset)"
#define _cTotalGrid	"Total Time With Grid Connection"
#define _cResetPartial	"Reset of Partial (Time & Energy)"


/* # 86 Last four alarms */

#define opGetLastAlarms	86
#define _opGetLastAlarms "Last four alarms"

/* return codes same as alarms from # 50 State request */


/* # 105 P/N Reading */

#define opGetPNC	105
#define _opGetPNC "P/N Reading (Central)"

#define aChar6  0
#define aChar5  1
#define aChar4  2
#define aChar3  3
#define aChar2  4
#define aChar1  5


/* # 56 0x38 unknown */

#define opGet0x38	56
#define _opGet0x38	"0x38 unknown"


/* # 82 0x52 unknown */

#define opGet0x52	82
#define _opGet0x52      "0x52 unknown"


/* # 83 0x53 unknown */

#define opGet0x53	83
#define _opGet0x53      "0x53 unknown"


