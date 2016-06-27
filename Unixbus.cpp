#include "Unixbus.hpp"
#include "afunix_polyfill/afunix_polyfill.h"

using namespace Kite;

Unixbus::Unixbus(std::weak_ptr<EventLoop> ev)
    : File(ev)
{
    int fd = afunix_socket(0);
    setFile(fd);
}

Unixbus::~Unixbus()
{
    close();
}

bool Unixbus::invoke(const std::string &path, const std::string &data)
{
    int fd = afunix_socket(0);
    if (fd < 0) {
        perror("afunix_socket");
        return false;
    }
    if (afunix_connect(fd, path.c_str(), AFUNIX_PATH_CONVENTION) != 0) {
        perror("afunix_connect");
        return false;
    }
    if (::send(fd, data.data(), data.length(), 0) != data.length()) {
        perror("send");
        return false;
    }
    afunix_close(fd);
    return true;
}

bool Unixbus::bind (const std::string &path)
{
    return (afunix_bind(d_fd, path.c_str(), AFUNIX_PATH_CONVENTION | AFUNIX_MAKE_PATH) == 0);
}

bool Unixbus::connect (const std::string &path)
{
    return (afunix_connect(d_fd, path.c_str(), AFUNIX_PATH_CONVENTION) == 0);
}

void Unixbus::close()
{
    if (d_fd == 0)
        return;
    onClosed();
    afunix_close(d_fd);
    d_fd = 0;
}

void Unixbus::sendBusMessage(const std::string &data, int address)
{
    afunix_sendto(d_fd, (void*)data.data(), data.length(), 0, address);
}

void Unixbus::onClosed()
{
}

void Unixbus::onBusMessage(const std::string &data, int address)
{
}

void Unixbus::onActivated(int fd, int e)
{
    fprintf(stderr, "-------------------Unixbus::activated\n");
    char buf[AFUNIX_MAX_PACKAGE_SIZE];
    int address;
    int r = afunix_recvfrom(d_fd, &buf, AFUNIX_MAX_PACKAGE_SIZE, MSG_DONTWAIT, &address);
    if (r <  0) {
        if (errno != EAGAIN) {
            close();
        }
        return;
    }
    onBusMessage(std::string(buf, r), address);
}


