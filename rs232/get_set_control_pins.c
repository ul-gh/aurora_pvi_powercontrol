/*
* RS-232 Hardware Manipulation Examples
* Copyright 2003, 2015 by Floyd L. Davidson, fl...@apaflo.com
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE. See the GNU General Public License for details.
*
* serialreport.c -- report serial port status
*
* $Id: serialreport.c,v 1.1.0.2 2004/07/28 17:03:12 floyd Exp floyd $
*/

/*
* A program demonstrating how to monitor RS-232 control lines
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>
#include <sys/ioctl.h>

void show_serial(char *);

/*
* list of serial ports this program checks
*/
char devices[][30] = {
    "/dev/ttyS0",
    "/dev/ttyS1",
    "/dev/ttyS2",
    "/dev/ttyS3",
    "/dev/cua0",
    "/dev/cua1",
    "/dev/cua2",
    "/dev/cua3",
};

#define MAXDEVICES ((int)(sizeof(devices) / 30))

int main(void) {
    int i;

    if (getuid()) {
        fprintf(stderr, "You must be root to use this program.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < MAXDEVICES; ++i) {
        show_serial(devices[i]);
    }

    return EXIT_SUCCESS;
}

#if 0
/*
* These lists are from /usr/include/asm/termios.h on a Linux box.
* They might be different on other platforms, and are included here
* for instructional purposes only.
*/

/* line disciplines */
#define N_TTY 0
#define N_SLIP 1
#define N_MOUSE 2
#define N_PPP 3
#define N_STRIP 4
#define N_AX25 5
#define N_X25 6 /* X.25 async */
#define N_6PACK 7
#define N_MASC 8          /* Reserved for Mob ... [snipped] */
#define N_R3964 9         /* Reserved for Sim ... [snipped] */
#define N_PROFIBUS_FDL 10 /* Reserved for Pro ... [snipped] */
#define N_IRDA 11         /* Linux IR - http: ... [snipped] */
#define N_SMSBLOCK 12     /* SMS block mode - ... [snipped] */
#define N_HDLC 13         /* synchronous HDLC */
#endif

/********

Note: DCE (Data Comm Equip) is the modem.
DTE (Data Term Equip) is the computer.


IOCTL CMD IOCTL CMD DTE Input Signal
Name Value RS232 or Name
Pin Output

--- -- 1 --- Frame Ground
TIOCSBRK/TIOCCBRK -- 2 output TxD Transmit Signal from computer
--- -- 3 input RxD Receive Signal to computer
TIOCM_RTS 0x004 4 output Request To Send
TIOCM_CTS 0x020 5 input Clear To Send
TIOCM_DSR 0x100 6 input Data Set Ready (from DCE)
--- -- 7 --- Signal Ground
TIOCM_CAR 0x040 8 input Carrier detect
9
10
11
12 input Secondary carrier detect
13 input Secondary Clear To Send
14 output Secondary Transmit Data
15 input Transmitter Clock (from modem)
16 input Secondary Receive Data
17 input Receiver Clock (from modem)
18 output Local Loopback (Trigger Loopback at DCE)
19 output Secondary Request To Send
TIOCM_DTR 0x002 20 output Data Terminal Ready
21 output Remote Loopback (Trigger Loopback at remote modem)
TIOCM_RNG 0x080 22 input Ring Detect
23 either Data Signal Rate Selector
24 output External Transmit Clock (to modem)
25 input Test Mode (modem is in loopback)

Other ioctl command names and/or synonyms:

TIOCM_CD TIOCM_CAR
TIOCM_RI TIOCM_RNG
TIOCM_ST 0x008
TIOCM_SR 0x010
TIOCM_LE 0x001
TIOCM_OUT1 0x2000
TIOCM_OUT2 0x4000
TIOCM_LOOP 0x8000
TIOCM_MODEM_BITS TIOCM_OUT2

Chart of Typical 9 to 25 pin cable converter

Description Signal 9-pin DTE 25-pin DCE Source
Carrier Detect CD 1 8 Modem (DCE)
Receive Data RD 2 3 Modem
Transmit Data TD 3 2 Computer (DTE)
Data Terminal Ready DTR 4 20 Computer
Signal Ground SG 5 7 Modem
Data Set Ready DSR 6 6 Modem
Request to Send RTS 7 4 Computer
Clear to Send CTS 8 5 Modem
Ring Indicator RI 9 22 Modem

Typical 9-Pin Null Modem for Hardware Flow Control


RxD 2 >---+ +---< 2 RxD
\/
/\
TxD 3 <---+ +---> 3 TxD

RTS 7 >---+ +---< 7 RTS
\/
/\
CTS 8 <---+ +---> 8 CTS

DTR

CD 1 -----><----- 1 RI



RI 1 -----><----- 1 RI
*******/

void show_serial(char *device) {
    int valid;
    int rts;
    int openflag;
    int cntrlreg;
    int fd;
    int results;
    struct termios term;
    struct serial_struct serinfo;

    valid = 1;
    openflag = 0;

    if ((fd = open(device, O_RDWR | O_NONBLOCK | O_NOCTTY)) < 0) {
        valid = 0;
        printf("Device: %s <Cannot be opened>", device);
    } else {
        openflag = 1;
    }

    serinfo.reserved_char[0] = 0;

    if (valid && ioctl(fd, TIOCGSERIAL, &serinfo) < 0) {
        valid = 0; /* can't get serial info */
        printf("Device: %s <cannot get serial info struct>", device);
    }

    if (valid && ioctl(fd, TIOCMGET, &cntrlreg) < 0) {
        valid = 0; /* can't get control register */
        printf("Device: %s <cannot get control register>", device);
    }

    if (serinfo.line >= MAXDEVICES) {
        valid = 0; /* out of interesting range */
        printf("Device: %s <Device is out of range>", device);
    }

    if (valid) {
        printf("Device: %s: DCD = %s",
               device,
               cntrlreg & TIOCM_CAR ? "ON" : "OFF");
    }

    if (valid && tcgetattr(fd, &term) < 0) {
        valid = 0; /* can't get termios info */
        printf(" <Cannot get termios struct> ");
    }

    if (valid && B0 == cfgetospeed(&term)) {
        valid = 0; /* disabled if speed is 0 */
        printf(" [Speed is set to 0 (DTR cleared)] ");
    }

    if (valid && ioctl(fd, TIOCSERGETLSR, &results) < 0) {
        valid = 0; /* can't check line status */
        printf(" <Cannot get line status register> ");
    }

    if (valid &&
        results &&
        term.c_line != N_SLIP &&
        term.c_line != N_PPP &&
        term.c_line != N_TTY)
    {
        valid = 0; /* not a tty, ppp or slip line */
        printf(" [Not set for tty, slip or ppp line discipline] ");
    }

    rts = !(cntrlreg & TIOCM_RTS);

    if (valid) {
        printf(" rts status: %s", rts ? "ON" : "OFF");
    }

    if (openflag) {
        close(fd);
    }

    putchar('\n');

    return;
}

#if 1

/*
* Two different ways to set/clear modem control lines.
*/

/*
* Set or Clear DTR modem control line
*
* Note: TIOCMBIS: CoMmand BIt Set
* TIOCMBIC: CoMmand BIt Clear
*
*/
void setdtr(int fd, int on) {
    int controlbits = TIOCM_DTR;
    ioctl(fd, (on ? TIOCMBIS : TIOCMBIC), &controlbits);
}

/*
* Set or Clear RTS modem control line
*
* Note: TIOCMSET and TIOCMGET are POSIX
*
* the same things:
*
* TIOCMODS and TIOCMODG are BSD (4.3 ?)
* MCSETA and MCGETA are HPUX
*/
void setrts(int fd, int on) {
    int controlbits;

    ioctl(fd, TIOCMGET, &controlbits);
    if (on) {
        controlbits |= TIOCM_RTS;
    } else {
        controlbits &= ~TIOCM_RTS;
    }
    ioctl(fd, TIOCMSET, &controlbits);
}

#endif
