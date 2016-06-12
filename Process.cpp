#include "Process.hpp"
#include "Timer.hpp"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "forkfd.h"

#include <iostream>

using namespace Kite;


Process::Process(std::weak_ptr<EventLoop> ev)
    : Evented(ev)
{
    d_pid = 0;
}

void Process::popen(const std::string &cmd)
{
    pipe2(d_pipein,     O_CLOEXEC);
    pipe2(d_pipeout,    O_CLOEXEC);
    fcntl(d_pipeout[0], F_SETFL, O_NONBLOCK);
    evAdd(d_pipeout[0]);

    d_forkfd = ::forkfd(FFD_CLOEXEC, &d_pid);
    evAdd(d_forkfd);

    if (d_pid == 0) {

        ::close(d_pipein[1]);
        ::close(d_pipeout[0]);

        dup2(d_pipein[0], STDIN_FILENO);
        dup2(d_pipeout[1], STDOUT_FILENO);
        dup2(d_pipeout[1], STDERR_FILENO);

        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)NULL);
        exit(1);
    }

}

void Process::onActivated(int e)
{
    if (e == d_forkfd){
        close();
    }
    onReadActivated();
}

void Process::close()
{
    onClosing();
    evRemove(d_pipeout[0]);
    evRemove(d_forkfd);
    ::close(d_pipein[1]);
    ::close(d_pipeout[0]);
    forkfd_close(d_forkfd);
    //kill(d_pid, SIGTERM);
    d_pid = 0;
}

void Process::closeRead()
{
    ::close(d_pipeout[0]);
}

void Process::closeWrite()
{
    ::close(d_pipein[1]);
}

int Process::read(char *buf, int len)
{
    if (d_pid == 0)
        return 0;
    return ::read(d_pipeout[0], buf, len);
}

int Process::write(const char *buf, int len)
{
    if (d_pid == 0)
        return 0;
    return ::write(d_pipein[1], buf, len);
}


class CollectorProcess : public Process
{
public:
    CollectorProcess(std::weak_ptr<EventLoop> ev)
        :Process(ev)
    {}

    std::string buffer;
protected:
    virtual void onReadActivated() override
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
    p->popen(cmd);

    if (ev->exec() != 0)
        throw std::runtime_error("Process::shell timeout");

    return p->buffer;

}
