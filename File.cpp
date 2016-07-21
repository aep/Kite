#include "File.hpp"
#include <stdio.h>
#include <unistd.h>

using namespace Kite;

File::File(std::weak_ptr<EventLoop> ev)
    : Evented(ev)
    , d_fd(0)
{
}

File::~File()
{
    close();
}

void File::setFile(int fd)
{
    if (d_fd) {
        evRemove(d_fd);
    }
    d_fd = fd;
    if (d_fd) {
        evAdd(d_fd);
    }
}

int File::read(char *buf, int len)
{
    int r = ::read(d_fd, buf, len);
    if (r < 0) {
        if (errno != EAGAIN)
            close();
        return 0;
    }
}

int File::write(const char *buf, int len)
{
    return ::write(d_fd, buf, len);
}

void File::close()
{
    if (d_fd) {
        ::close(d_fd);
        evRemove(d_fd);
        d_fd = 0;
    }
}
