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

#define	VersionSH	"1.8.6"

/* Transmission States */
static char szTransStates[] =
"Everything is OK\0"				/* #  0 */
"Unknown Transmission State 1\0"		/* #  1 These are all placeholders */
"Unknown Transmission State 2\0"		/* #  2 */
"Unknown Transmission State 3\0"		/* #  3 */
"Unknown Transmission State 4\0"		/* #  4 */
"Unknown Transmission State 5\0"		/* #  5 */
"Unknown Transmission State 6\0"		/* #  6 */
"Unknown Transmission State 7\0"		/* #  7 */
"Unknown Transmission State 8\0"		/* #  8 */
"Unknown Transmission State 9\0"		/* #  9 */
"Unknown Transmission State 10\0"		/* # 10 */
"Unknown Transmission State 11\0"		/* # 11 */
"Unknown Transmission State 12\0"		/* # 12 */
"Unknown Transmission State 13\0"		/* # 13 */
"Unknown Transmission State 14\0"		/* # 14 */
"Unknown Transmission State 15\0"		/* # 15 */
"Unknown Transmission State 16\0"		/* # 16 */
"Unknown Transmission State 17\0"		/* # 17 */
"Unknown Transmission State 18\0"		/* # 18 */
"Unknown Transmission State 19\0"		/* # 19 */
"Unknown Transmission State 20\0"		/* # 20 */
"Unknown Transmission State 21\0"		/* # 21 */
"Unknown Transmission State 22\0"		/* # 22 */
"Unknown Transmission State 23\0"		/* # 23 */
"Unknown Transmission State 24\0"		/* # 24 */
"Unknown Transmission State 25\0"		/* # 25 */
"Unknown Transmission State 26\0"		/* # 26 */
"Unknown Transmission State 27\0"		/* # 27 */
"Unknown Transmission State 28\0"		/* # 28 */
"Unknown Transmission State 29\0"		/* # 29 */
"Unknown Transmission State 30\0"		/* # 30 */
"Unknown Transmission State 31\0"		/* # 31 */
"Unknown Transmission State 32\0"		/* # 32 */
"Unknown Transmission State 33\0"		/* # 33 */
"Unknown Transmission State 34\0"		/* # 34 */
"Unknown Transmission State 35\0"		/* # 35 */
"Unknown Transmission State 36\0"		/* # 36 */
"Unknown Transmission State 37\0"		/* # 37 */
"Unknown Transmission State 38\0"		/* # 38 */
"Unknown Transmission State 39\0"		/* # 39 */
"Unknown Transmission State 40\0"		/* # 40 */
"Unknown Transmission State 41\0"		/* # 41 */
"Unknown Transmission State 42\0"		/* # 42 */
"Unknown Transmission State 43\0"		/* # 43 */
"Unknown Transmission State 44\0"		/* # 44 */
"Unknown Transmission State 45\0"		/* # 45 */
"Unknown Transmission State 46\0"		/* # 46 */
"Unknown Transmission State 47\0"		/* # 47 */
"Unknown Transmission State 48\0"		/* # 48 */
"Unknown Transmission State 49\0"		/* # 49 */
"Unknown Transmission State 50\0"		/* # 50 */
"Command is not implemented\0"			/* # 51 */
"Variable does not exist\0"			/* # 52 */
"Variable value is out of range\0"		/* # 53 */
"EEprom not accessible\0"			/* # 54 */
"Not Toggled Service Mode\0"			/* # 55 */
"Cannot send the command to internal micro\0"	/* # 56 */
"Command not Executed\0"			/* # 57 */
"The variable is not available, retry\0";	/* # 58 */


/* Global States */
static char szGlobalStates[] =
"Sending Parameters\0"				/* #  0 */
"Wait Sun/Grid\0"				/* #  1 */
"Checking Grid\0"				/* #  2 */
"Measuring Riso\0"				/* #  3 */
"DcDc Start\0"					/* #  4 */
"Inverter Turn-On\0"				/* #  5 */
"Run\0"						/* #  6 */
"Recovery\0"					/* #  7 */
"Pause\0"					/* #  8 */
"Ground Fault\0"				/* #  9 */
"OTH Fault\0"					/* # 10 */
"Address Setting\0"				/* # 11 */
"Self Test\0"					/* # 12 */
"Self Test Fail\0"				/* # 13 */
"Sensor Test + Measure Riso\0"			/* # 14 */
"Leak Fault\0"					/* # 15 */
"Waiting for manual reset\0"			/* # 16 */
"Internal Error E026\0"				/* # 17 */
"Internal Error E027\0"				/* # 18 */
"Internal Error E028\0"				/* # 19 */
"Internal Error E029\0"				/* # 20 */
"Internal Error E030\0"				/* # 21 */
"Sending Wind Table\0"				/* # 22 */
"Failed Sending Table\0"			/* # 23 */
"UTH Fault\0"					/* # 24 */
"Remote Off\0"					/* # 25 */
"Interlock Fail\0"				/* # 26 */
"Executing Autotest\0"				/* # 27 */
"Unknown Global State 28\0"			/* # 28 Placeholder */
"Unknown Global State 29\0"			/* # 29 Placeholder */
"Waiting Sun\0"					/* # 30 */
"Temperature Fault\0"				/* # 31 */
"Fan Staucked\0"				/* # 32 */
"Int. Com. Fail\0"				/* # 33 */
"Slave Insertion\0"				/* # 34 */
"DC Switch Open\0"				/* # 35 */
"TRAS Switch Open\0"				/* # 36 */
"MASTER Exclusion\0"				/* # 37 */
"Auto Exclusion\0"				/* # 38 */
"Unknown Global State 39\0"			/* # 39 These are all placeholders for the last 4 */
"Unknown Global State 40\0"			/* # 40 */
"Unknown Global State 41\0"			/* # 41 */
"Unknown Global State 42\0"			/* # 42 */
"Unknown Global State 43\0"			/* # 43 */
"Unknown Global State 44\0"			/* # 44 */
"Unknown Global State 45\0"			/* # 45 */
"Unknown Global State 46\0"			/* # 46 */
"Unknown Global State 47\0"			/* # 47 */
"Unknown Global State 48\0"			/* # 48 */
"Unknown Global State 49\0"			/* # 49 */
"Unknown Global State 50\0"			/* # 50 */
"Unknown Global State 51\0"			/* # 51 */
"Unknown Global State 52\0"			/* # 52 */
"Unknown Global State 53\0"			/* # 53 */
"Unknown Global State 54\0"			/* # 54 */
"Unknown Global State 55\0"			/* # 55 */
"Unknown Global State 56\0"			/* # 56 */
"Unknown Global State 57\0"			/* # 57 */
"Unknown Global State 58\0"			/* # 58 */
"Unknown Global State 59\0"			/* # 59 */
"Unknown Global State 60\0"			/* # 60 */
"Unknown Global State 61\0"			/* # 61 */
"Unknown Global State 62\0"			/* # 62 */
"Unknown Global State 63\0"			/* # 36 */
"Unknown Global State 64\0"			/* # 64 */
"Unknown Global State 65\0"			/* # 65 */
"Unknown Global State 66\0"			/* # 66 */
"Unknown Global State 67\0"			/* # 67 */
"Unknown Global State 68\0"			/* # 68 */
"Unknown Global State 69\0"			/* # 69 */
"Unknown Global State 70\0"			/* # 70 */
"Unknown Global State 71\0"			/* # 71 */
"Unknown Global State 72\0"			/* # 72 */
"Unknown Global State 73\0"			/* # 73 */
"Unknown Global State 74\0"			/* # 74 */
"Unknown Global State 75\0"			/* # 75 */
"Unknown Global State 76\0"			/* # 76 */
"Unknown Global State 77\0"			/* # 77 */
"Unknown Global State 78\0"			/* # 78 */
"Unknown Global State 79\0"			/* # 79 */
"Unknown Global State 80\0"			/* # 80 */
"Unknown Global State 81\0"			/* # 81 */
"Unknown Global State 82\0"			/* # 82 */
"Unknown Global State 83\0"			/* # 83 */
"Unknown Global State 84\0"			/* # 84 */
"Unknown Global State 85\0"			/* # 85 */
"Unknown Global State 86\0"			/* # 86 */
"Unknown Global State 87\0"			/* # 87 */
"Unknown Global State 88\0"			/* # 88 */
"Unknown Global State 89\0"			/* # 89 */
"Unknown Global State 90\0"			/* # 90 */
"Unknown Global State 91\0"			/* # 91 */
"Unknown Global State 92\0"			/* # 92 */
"Unknown Global State 93\0"			/* # 93 */
"Unknown Global State 94\0"			/* # 94 */
"Unknown Global State 95\0"			/* # 95 */
"Unknown Global State 96\0"			/* # 96 */
"Unknown Global State 97\0"			/* # 97 */
"Erasing Internal EEprom\0"			/* # 98 */
"Erasing External EEprom\0"			/* # 99 was Erasing EEPROM */
"Counting EEprom\0"				/* # 100 */
"Freeze\0" ;					/* # 101 */


/* DcDc Status */
static char szDcDcStatus[] =
"DcDc OFF\0"					/* #  0 */
"Ramp Start\0"					/* #  1 */
"MPPT\0"					/* #  2 */
"not used\0"					/* #  3 */
"Input Over Current\0"				/* #  4 */
"Input Under Voltage\0"				/* #  5 */
"Input Over Voltage\0"				/* #  6 */
"Input Low\0"					/* #  7 */
"No Parameters\0"				/* #  8 */
"Bulk Over Voltage\0"				/* #  9 */
"Communication Error\0"				/* # 10 */
"Ramp Fail\0"					/* # 11 */
"Internal Error\0"				/* # 12 */
"Input mode Error\0"				/* # 13 */
"Ground Fault\0"				/* # 14 */
"Inverter Fail\0"				/* # 15 */
"DcDc IGBT Sat\0"				/* # 16 */
"DcDc ILEAK Fail\0"				/* # 17 */
"DcDc Grid Fail\0"				/* # 18 */
"DcDc Comm. Error\0" ;				/* # 19 */


/* Inverter State */
static char szInverterState[] =
"Stand By\0"					/* #  0 */
"Checking Grid\0"				/* #  1 */
"Run\0"						/* #  2 */
"Bulk Over Voltage\0"				/* #  3 */
"Out Over Current\0"				/* #  4 */
"IGBT Sat\0"					/* #  5 */
"Bulk Under Voltage\0"				/* #  6 */
"Degauss Error\0"				/* #  7 */
"No Parameters\0"				/* #  8 */
"Bulk Low\0"					/* #  9 */
"Grid Over Voltage\0"				/* # 10 */
"Communication Error\0"				/* # 11 */
"Degaussing\0"					/* # 12 */
"Starting\0"					/* # 13 */
"Bulk Cap Fail\0"				/* # 14 */
"Leak Fail\0"					/* # 15 */
"DcDc Fail\0"					/* # 16 */
"Ileak Sensor Fail\0"				/* # 17 */
"SelfTest: relay inverter\0"			/* # 18 */
"SelfTest: wait for sensor test\0"		/* # 19 */
"SelfTest: test relay DcDc + sensor\0"		/* # 20 */
"SelfTest: relay inverter fail\0"		/* # 21 */
"SelfTest: timeout fail\0"			/* # 22 */
"SelfTest: relay DcDc fail\0"			/* # 23 */
"Self Test 1\0"					/* # 24 */
"Waiting self test start\0"			/* # 25 */
"Dc Injection\0"				/* # 26 */
"Self Test 2\0"					/* # 27 */
"Self Test 3\0"					/* # 28 */
"Self Test 4\0"					/* # 29 */
"Internal Error (30)\0"				/* # 30 */
"Internal Error (31)\0"				/* # 31 */
"Unknown Inverter State 32\0"			/* # 32 These are all placeholders */
"Unknown Inverter State 33\0"			/* # 33 */
"Unknown Inverter State 34\0"			/* # 34 */
"Unknown Inverter State 35\0"			/* # 35 */
"Unknown Inverter State 36\0"			/* # 36 */
"Unknown Inverter State 37\0"			/* # 37 */
"Unknown Inverter State 38\0"			/* # 38 */
"Unknown Inverter State 39\0"			/* # 39 */
"Forbidden State\0"				/* # 40 */
"Input UC\0"					/* # 41 */
"Zero Power\0"					/* # 42 */
"Grid Not Present\0"				/* # 43 */
"Waiting Start\0"				/* # 44 */
"MPPT\0"					/* # 45 */
"Grid Fail\0"					/* # 46 */
"Input OC\0" ;					/* # 47 */


/* Alarm State */
static char szAlarmState[] =
"No Alarm\0"					/* #  0 */
"Sun Low W001 (1)\0"				/* #  1 */
"Input Over Current E001\0"			/* #  2 */
"Input Under Voltage W002\0"			/* #  3 */
"Input Over Voltage E002\0"			/* #  4 */
"Sun Low W001 (5)\0"				/* #  5 */
"No Parameters E003\0"				/* #  6 was Internal error E003 */
"Bulk Over Voltage E004\0"			/* #  7 */
"Comm. Error E005\0"				/* #  8 was Internal error E005 */
"Output Over Current E006\0"			/* #  9 */
"IGBT Sat E007\0"				/* # 10 was Internal error E007 */
"Bulk UV W011\0"				/* # 11 was Internal error E008 */
"Internal error E009\0"				/* # 12 was Internal error E009 */
"Grid Fail W003\0"				/* # 13 */
"Bulk Low E010\0"				/* # 14 was Internal error E010 */
"Ramp Fail E011\0"				/* # 15 was Internal error E011 */
"Dc/Dc Fail E012\0"				/* # 16 */
"Wrong Mode E013\0"				/* # 17 */
"Ground Fault (18)\0"				/* # 18 */
"Over Temp. E014\0"				/* # 19 */
"Bulk Cap Fail E015\0"				/* # 20 */
"Inverter Fail E016\0"				/* # 21 */
"Start Timeout E017\0"				/* # 22 was Internal error E017 */
"Ground Fault E018 (23)\0"			/* # 23 */
"Degauss error (24)\0"				/* # 24 */
"Ileak Sens. fail E019\0"			/* # 25 was Internal error E019 */
"DcDc Fail E012\0"				/* # 26 */
"Self  Test Error 1 E020\0"			/* # 27 was Internal error E020 */
"Self  Test Error 2 E021\0"			/* # 28 was Internal error E021 */
"Self  Test Error 3 E019\0"			/* # 29 was Internal error E019 */
"Self  Test Error 4 E022\0"			/* # 30 was Internal error E022 */
"DC inj error E023\0"				/* # 31 was Internal error E023 */
"Grid Over Voltage W004\0"			/* # 32 */
"Grid Under Voltage W005\0"			/* # 33 */
"Grid OF W006\0"				/* # 34 */
"Grid UF W007\0"				/* # 35 */
"Z grid Hi W008\0"				/* # 36 */
"Internal error E024\0"				/* # 37 */
"Riso Low E025\0"				/* # 38 */
"Vref Error E026\0"				/* # 39 was Internal error E026 */
"Error Meas V E027\0"				/* # 40 was Internal error E027 */
"Error Meas F E028\0"				/* # 41 was Internal error E028 */
"Error Meas I E029\0"				/* # 42 was Internal error E029 */
"Error Meas Ileak E030\0"			/* # 43 was Internal error E030 */
"Read Error V E031\0"				/* # 44 was Internal error E031 */
"Read Error I E032\0"				/* # 45 was Internal error E032 */
"Table fail W009\0"				/* # 46 was Empty Wind Table W009 */
"Fan Fail W010\0"				/* # 47 */
"UTH E033\0"					/* # 48 was Internal error E033 */
"Interlock Fail\0"				/* # 49 */
"Remote Off\0"					/* # 50 */
"Vout Avg error\0"				/* # 51 */
"Battery low\0"					/* # 52 */
"Clk fail\0"					/* # 53 */
"Input UC\0"					/* # 54 */
"Zero Power\0"					/* # 55 */
"Fan Stucked\0"					/* # 56 */
"DC Switch Open\0"				/* # 57 */
"Tras Switch Open\0"				/* # 58 */
"AC Switch Open\0"				/* # 59 */
"Bulk UV\0"					/* # 60 */
"Autoexclusion\0"				/* # 61 */
"Grid df/dt\0"					/* # 62 */
"Den switch Open\0"				/* # 63 */
"Jbox fail\0" ;					/* # 64 */

#define _ModelNameLen   55
struct model {
    int ID;
    float multipler;
    char name[_ModelNameLen];
};

static struct model model_names[] = {
    {'1', 0.0970019, "PVI-3.0-OUTD"},
    {'2', 0.1617742, "PVI-3.3-OUTD"},
    {'3', 0.1617742, "PVI-3.6-OUTD"},
    {'4', 0.1617742, "PVI-4.2-OUTD"},
    {'5', 0.1617742, "PVI-5000-OUTD"},
    {'6', 0.1617742, "PVI-6000-OUTD"},
    {'A', 0.1617742, "PVI-CENTRAL-350 Liquid Cooled (AC gathering)"},
    {'B', 0.1617742, "PVI-CENTRAL-350 Liquid Cooled (display board)"},
    {'C', 0.1617742, "PVI-CENTRAL-50 module"},
    {'D', 0.5320955, "PVI-12.5-OUTD"},
    {'G', -1, "UNO-2.5-I"},
    {'g', -1, "UNO-2.0-I"},
    {'H', -1, "PVI-4.6-I-OUTD"},
    {'h', -1, "PVI-3.8-I-OUTD"},
    {'I', 0.1004004, "PVI-3600"},
    {'i', 0.0557842, "PVI-2000"},
    {'L', 0.1617742, "PVI-CENTRAL-350 Liquid Cooled (control board)"},
    {'M', 0.5320955, "PVI-CENTRAL-250"},
    {'O', 0.1004004, "PVI-3.6-OUTD"},
    {'o', 0.0557842, "PVI-2000-OUTD"},
    {'P', 0.1617742, "3-phase interface (3G74)"},
    {'T', -1, "PVI-12.0-I-OUTD (output 480 VAC)"},
    {'t', -1, "PVI-10.0-I-OUTD (output 480 VAC)"},
    {'U', -1, "PVI-12.0-I-OUTD (output 208 VAC)"},
    {'u', -1, "PVI-10.0-I-OUTD (output 208 VAC)"},
    {'V', -1, "PVI-12.0-I-OUTD (output 380 VAC)"},
    {'v', -1, "PVI-10.0-I-OUTD (output 380 VAC)"},
    {'w', -1, "PVI-10.0-I-OUTD (output 480 VAC current limit 12 A)"},
    {'X', 0.5320955, "PVI-10.0-OUTD"},
    {'Y', -1, "TRIO-27.6-TL-OUTD"},
    {'y', -1, "TRIO-20-TL"},
    {'Z', -1, "PVI-12.0-I-OUTD (output 600 VAC)"},
    {'z', -1, "PVI-10.0-I-OUTD (output 600 VAC)"},
    {-1,  -1,        "unknown\0"}
};

