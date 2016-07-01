#include "Serialport.hpp"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

using namespace Kite;

Serialport::Serialport(std::weak_ptr<EventLoop> ev)
    : Evented(ev)
    , IO()
    , d_fd(0)
{
}

static speed_t baud2speed_t(uint32_t bauds)
{
    switch (bauds) {
        case 50:      return B50;
        case 75:      return B75;
        case 110:     return B110;
        case 134:     return B134;
        case 150:     return B150;
        case 200:     return B200;
        case 300:     return B300;
        case 600:     return B600;
        case 1200:    return B1200;
        case 1800:    return B1800;
        case 2400:    return B2400;
        case 4800:    return B4800;
        case 9600:    return B9600;
        case 19200:   return B19200;
        case 38400:   return B38400;
        case 115200:  return B115200;
        case 230400:  return B230400;
        case 460800:  return B460800;
        case 500000:  return B500000;
        case 576000:  return B576000;
        case 921600:  return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
        default: return B9600;
        };
}
static int set_interface_attribs (int fd, int speed, int parity) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0) {
		perror("tcgetattr");
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		perror("tcsetattr");
		return -1;
	}
	return 0;
}

static void set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0) {
		perror("tcgetattr");
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		perror("tcsetattr");
		return;
	}
}

bool Serialport::open(const std::string &port, uint32_t speed)
{
	if (d_fd != 0)
		return false;

	d_fd = ::open (port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if (d_fd < 1) {
		d_fd = 0;
		perror("Serialport::open");
		return false;
	}
	set_interface_attribs (d_fd, baud2speed_t(speed), 0);
	set_blocking (d_fd, 0);

	evAdd(d_fd);
}

void Serialport::close()
{
	if (d_fd != 0) {
		::close(d_fd);
		d_fd = 0;
	}
}

int Serialport::read(char *buf, int len)
{
	if (d_fd == 0)
		return 0;
	return ::read(d_fd, buf, len);
}

int Serialport::write(const char *buf, int len)
{
	if (d_fd == 0)
		return 0;
	return ::write(d_fd, buf, len);
}

