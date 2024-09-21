// C library headers
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

static int serial_fd = 0;
int serialSetup()
{
    int serial_port;
    serial_port = open("/dev/ttyS3", O_RDWR | O_NOCTTY);
    if (serial_port < 0)
    {
        perror("open uart device error\n");
        return -1;
    }
    // Create new termios struct, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    struct termios tty;

    // Read in existing settings, and handle any error
    // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() overwise behaviour
    // is undefined
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }
    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;  // Clear all the size bits, then use one of the statements below
    tty.c_cflag |= CS8;     // 8 bits per byte (most common)
    // tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON;
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }
    return serial_port;
}

static int serialWrite(int fd, uint8_t command)
{
    int len;
    len = write(fd, &command, 1);
    if (len < 0)
        return 0;
    return 1;
}

#define SERIAL_CMD_READ_ADC 0x58
#define SERIAL_CMD_POWEROFF 0x6a
#define SERIAL_CMD_USBMODE_NORMAL 0x11
#define SERIAL_CMD_USBMODE_WIFI_CAM 0x12
#define SERIAL_CMD_USBMODE_DIRECT 0x13

int16_t PowerManager_getBatteryVoltage()
{
    char buf[2];
    int len;
    int16_t result = 0;
    serialWrite(serial_fd, SERIAL_CMD_READ_ADC);
    len = read(serial_fd, buf, 2);
    if (len < 0)
        return -1;
    result = buf[0];
    result <<= 8;
    result |= buf[1];
    return result;
}

void PowerManager_init()
{
    serial_fd = serialSetup();
}

#include <signal.h>
void PowerManager_powerOff()
{
    if (serial_fd <= 0)
        return;
    serialWrite(serial_fd, SERIAL_CMD_POWEROFF);
    system("echo 1 > /tmp/poweroff");
    system("poweroff");
    // stop here
    exit(0);
}
