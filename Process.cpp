#include "Process.hpp"
#include "Timer.hpp"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/signalfd.h>
#include <sys/wait.h>
#include <system_error>
#include <iostream>

using namespace Kite;




class SigchildService : public Evented
{
public:
    SigchildService(std::weak_ptr<EventLoop> ev)
        : Evented(ev)
    {

    }
};


Process::Process(std::weak_ptr<EventLoop> ev)
    : Evented(ev)
{
    d_pid = 0;
}

void Process::popen(const std::string &cmd)
{
    if (pipe2(d_pipein, O_CLOEXEC) != 0) {
        throw std::system_error(errno, std::system_category());
    }
    if (pipe2(d_pipeout, O_CLOEXEC) != 0) {
        throw std::system_error(errno, std::system_category());
    }
    fcntl(d_pipeout[0], F_SETFL, O_NONBLOCK);
    evAdd(d_pipeout[0]);
    evAddSignal(SIGCHLD);

    //fork here
    d_pid = fork();
    if (d_pid < 0) {
        perror("fork");
        return;
    }

    if (d_pid == 0) {

        //unblock all signals in the new child, because we use signalfd,
        //which must have blocked signals
        sigset_t signal_set;
        sigfillset(&signal_set);
        sigprocmask(SIG_UNBLOCK, &signal_set, NULL);

        ::close(d_pipein[1]);
        ::close(d_pipeout[0]);

        dup2(d_pipein[0], STDIN_FILENO);
        dup2(d_pipeout[1], STDOUT_FILENO);
        dup2(d_pipeout[1], STDERR_FILENO);

        prctl(PR_SET_PDEATHSIG, SIGKILL);

        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)NULL);
        exit(1);
    }
    ::close(d_pipein[0]);
    ::close(d_pipeout[1]);
}

void Process::onSignal(int,int)
{
    int status;
    pid_t pid = waitpid(d_pid, &status, WNOHANG);
    if (pid == d_pid) {
        close();
    }
}

void Process::onActivated(int fd, int e)
{
    onReadActivated();
}

void Process::close()
{
    onClosing();
    evRemove(d_pipeout[0]);
    ::close(d_pipein[1]);
    ::close(d_pipeout[0]);

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
        if (len >  0) {
            buffer += std::string(buf,len);
        }
    }
    virtual void onClosing() override
    {
        //read all leftover
        for (;;) {
            char buf[1024];
            int len = read(buf, 1024);
            if (len < 1)
                break;
            buffer += std::string(buf,len);
        }

        ev().lock()->exit(0);
        return;
    }

};

std::string Process::shell(const std::string &cmd, int timeout)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::weak_ptr<Kite::EventLoop>  wev(ev);

    if (timeout > 0) {
        Kite::Timer::later(ev, [wev] () {
                wev.lock()->exit(9);
                return false;
                }, timeout, ev.get(), "process::shell:timeout");
    }

    std::shared_ptr<CollectorProcess>  p(new CollectorProcess(ev));
    p->popen(cmd);

    if (ev->exec() != 0)
        throw std::runtime_error("Process::shell timeout: " + cmd);

    return p->buffer;
}
