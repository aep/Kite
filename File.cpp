#include "File.hpp"
#include <stdio.h>
#include <unistd.h>

using namespace Kite;

File::File(std::weak_ptr<EventLoop> ev)
    : Evented(ev)
{
}

File::~File()
{
    close();
}

void File::setFile(int fd)
{
    d_fd = fd;
    evAdd(d_fd);
}

int File::read(char *buf, int len)
{
    return ::read(d_fd, buf, len);
}

int File::write(const char *buf, int len)
{
    return ::write(d_fd, buf, len);
}

void File::close()
{
    if (d_fd) {
        ::close(d_fd);
    }
}
