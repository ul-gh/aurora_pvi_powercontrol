#include <stdio.h> // standard input / output functions
#include <stdlib.h>
#include <string.h>  // string function definitions
#include <unistd.h>  // UNIX standard function definitions
#include <fcntl.h>   // File control definitions
#include <errno.h>   // Error number definitions
#include <termios.h> // POSIX terminal control definitions
#include <sys/ioctl.h> // Linux ioctl definitions

// Linux termios subsystem configuration
//
// See: https://man7.org/linux/man-pages/man3/termios.3.html
//
int set_serial_raw_mode(
        int fd,
        speed_t baudrate = B9600,
        int char_size = 8, // Number of bits is 5, 6, 7 or 8
        int parity = 0, // 0: No parity. 1: Odd parity. 2: Even parity
        int stopbits = 1 // One or two
        ) {
    int err = 0;
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    // Get current configuration
    err = tcgetattr(fd, &tty);
    if (err) {
        fprintf(stderr, "Unable to get serial port attributes\n");
        return err;
    }

    /* Set Baud Rate */
    err |= cfsetospeed(&tty, baudrate);
    err |= cfsetispeed(&tty, baudrate);
    // Clear character size bits
    tty.c_cflag &= ~CSIZE;
    // Set new character size
    switch(char_size) {
        case 5: tty.c_cflag |= CS5; break;
        case 6: tty.c_cflag |= CS6; break;
        case 7: tty.c_cflag |= CS7; break;
        case 8: tty.c_cflag |= CS8; break;
        default: err |= 1;
    }
    // Set parity
    switch(parity) {
        case 0: tty.c_cflag &= ~PARENB; break; // No parity
        case 1: tty.c_cflag |= PARENB | PARODD; break; // Odd parity
        case 2: tty.c_cflag |= PARENB;
                tty.c_cflag &= ~PARODD; break; // Even parity
        default: err |= 1;
    }
    // Set number of stop bits
    switch(stopbits) {
        case 1: tty.c_cflag &= ~CSTOPB; break; // One stop bit
        case 2: tty.c_cflag |= CSTOPB; break; // Two stop bits
        default: err |= 1;
    }
    if (err) {
        fprintf(stderr, "Incorrect configuration settings, see man 3 termios\n");
        return err;
    }
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver and ignore HW modem ctrl lines
    //tty.c_cflag &= ~CRTSCTS; // no HW flow control

    tty.c_cc[VMIN] = 0; // Zero characters minimum read i.e. read doesn't block
    tty.c_cc[VTIME] = 0; // Indefinite timeout (if VMIN > 0)

    // Raw mode..
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
    tty.c_iflag |= IGNPAR; // Ignore characters with invalid parity

    tty.c_oflag &= ~OPOST; // No character post processing

    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN); // Raw mode, no echo, no signals

    // Flush Port
    err |= tcflush(fd, TCIFLUSH);

    // Apply attributes set in above struct termios
    err |= tcsetattr(fd, TCSANOW, &tty);
    if (err) {
        fprintf(stderr, "Problem setting port attributes\n");
    }
    return err;
}

class RawSerial {
public:
    int fd = -1;

    RawSerial(const char *device = "/dev/ttyUSB0",
              speed_t baudrate = B9600,
              int char_size = 8, // Number of bits is 5, 6, 7 or 8
              int parity = 0, // 0: No parity. 1: Odd parity. 2: Even parity
              int stopbits = 1 // One or two
              ) {
        //int USB = open(device, O_RDWR);
        fd = open(device, O_RDWR | O_NONBLOCK | O_NDELAY);
        if (fd < 0) {
            fprintf(stderr, "Unable to open port\n");
            exit(1);
        }
        set_serial_raw_mode(fd, baudrate, char_size, parity, stopbits);
    }
    virtual ~RawSerial() {
        close(fd);
    }

    int readchar(char *buf, int n_chars) {
        if (fd == -1) {
            fprintf(stderr, "Device not available\n");
            return 0;
        }
        auto n_available = 0;
        ioctl(fd, FIONREAD, &n_available);
        printf("Number of characters in input buffer: %d\n", n_available);
        return read(fd, buf, n_chars);
    }

};

// Linux TTY ioctl calls must be used for low-level operations like
// level testing or setting of the HW modem control lines.
//
// See: https://man7.org/linux/man-pages/man4/tty_ioctl.4.html
//
int main(void) {
    auto port = RawSerial{"/dev/ttyUSB0", B9600, 8, 0, 1};

    int n_chars;
    char c;
    while (1) {
        n_chars = port.readchar(&c, 1);
        if (n_chars > 0) printf("Character value %c  -  ", c);
        if (n_chars < 0) printf("Error");
        sleep(1);
    }
}
