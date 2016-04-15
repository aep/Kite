#include "Process.hpp"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

using namespace Kite;

Process::Process(std::weak_ptr<EventLoop> ev)
    : Evented(ev)
{
}

void Process::popen(const char *cmd, const char *ch)
{
    d_f = ::popen(cmd, ch);
    fcntl(fileno(d_f), F_SETFL, O_NONBLOCK);
    evAdd(fileno(d_f));
}

void Process::close()
{
    evRemove(fileno(d_f));
    pclose(d_f);
    d_f = 0;
}

int Process::read(char *buf, int len)
{
    if (!d_f)
        return 0;
    return ::read(fileno(d_f), buf, len);
}

int Process::write(const char *buf, int len)
{
    if (!d_f)
        return 0;
    return ::write(fileno(d_f), buf, len);
}

