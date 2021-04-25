#include <stdio.h> // standard input / output functions
#include <stdlib.h>
#include <string.h>  // string function definitions
#include <unistd.h>  // UNIX standard function definitions
#include <fcntl.h>   // File control definitions
#include <errno.h>   // Error number definitions
#include <termios.h> // POSIX terminal control definitions
#include <sys/ioctl.h>

// Linux termios subsystem configuration
//
// See: https://man7.org/linux/man-pages/man3/termios.3.html
//
int set_serial_raw_mode(int fd) {
    int err = 0;
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    // Get current configuration
    err = tcgetattr(fd, &tty);
    if (err) {
        fprintf(stderr, "Unable to get serial port attributes\n");
        return err;
    }

    /* Set Baud Rate */
    err |= cfsetospeed(&tty, B115200);
    err |= cfsetispeed(&tty, B115200);

    // Set mode 8N1
    tty.c_cflag &= ~CSIZE; // Clear size bits
    tty.c_cflag |= CS8; // Set new character size

    tty.c_cflag &= ~PARENB; // Disable parity
    //tty.c_cflag &= ~PARODD; // If parity enabled, use even parity
    tty.c_cflag &= ~CSTOPB; // One stop bit; two if set

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

    RawSerial() {
        //int USB = open( "/dev/ttyUSB0", O_RDWR);
        fd = open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK | O_NDELAY);
        if (fd < 0) {
            fprintf(stderr, "Unable to open port\n");
            exit(1);
        }
        set_serial_raw_mode(fd);
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

// Linux TTY ioctl calls must be use for low-level operations like
// level testing or setting of the HW modem control lines.
//
// See: https://man7.org/linux/man-pages/man4/tty_ioctl.4.html
//
int main(void) {
    auto port = RawSerial{};

    int n_chars;
    char c;
    while (1) {
        n_chars = port.readchar(&c, 1);
        if (n_chars > 0) printf("Character value %c  -  ", c);
        if (n_chars < 0) printf("Error");
        sleep(1);
    }
}
