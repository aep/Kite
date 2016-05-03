#include "Process.hpp"
#include "Timer.hpp"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>

using namespace Kite;

Process::Process(std::weak_ptr<EventLoop> ev)
    : Evented(ev)
{
}

void Process::popen(const std::string &cmd, const char *ch)
{
    d_f = ::popen(cmd.c_str(), ch);
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


class CollectorProcess : public Process
{
public:
    CollectorProcess(std::weak_ptr<EventLoop> ev)
        :Process(ev)
    {}

    std::string buffer;
protected:
    virtual void onActivated(int) override
    {
        char buf[1024];
        int len = read(buf, 1024);
        if (len == 0) {
            close();
            ev()->exit(0);
            return;
        }
        buffer += std::string(buf,len);
    }

};

std::string Process::shell(const std::string &cmd, int timeout)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);

    if (timeout > 0) {
        Kite::Timer::later(ev, [ev] () {
                ev->exit(9);
                return false;
                },timeout, "process::shell:timeout");
    }


    std::shared_ptr<CollectorProcess>  p(new CollectorProcess(ev));
    p->popen(cmd, "r");

    if (ev->exec() != 0)
        throw std::runtime_error("Process::shell timeout");

    return p->buffer;

}
