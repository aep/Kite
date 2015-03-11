#include "Stdio.hpp"
#include <stdio.h>
#include <unistd.h>

using namespace Kite;

Stdio::Stdio(std::weak_ptr<EventLoop> ev)
    : Evented(ev)
{
    evAdd(fileno(stdin));
}

int Stdio::read(char *buf, int len)
{
    return ::read(fileno(stdin), buf, len);
}

int Stdio::write(const char *buf, int len)
{
    return ::write(fileno(stdout), buf, len);
}

