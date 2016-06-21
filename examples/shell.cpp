#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

#include "EventLoop.hpp"
#include "Process.hpp"
#include "Stdio.hpp"

class Shell: public Kite::Process
{
public:
    Shell(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::Process(ev)
    {
        popen("bash");
    }

protected:
    virtual void onReadActivated() override
    {
        char buf[1024];
        int len = read(buf, 1024);
        std::cerr << ":L: "<< len << std::endl;
        if (len == 0) {
            close();
            ev()->exit(0);
            return;
        }
        ::write(fileno(stdout), buf, len);
    }
};


std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
std::shared_ptr<Shell>        shell(new Shell(ev));

class IO: public Kite::Stdio
{
public:
    IO(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::Stdio(ev)
    {
    }

protected:
    virtual void onActivated(int,int) override
    {
        char buf[1024];
        int len = read(buf, 1024);
        if (len == 0) {
            ev()->exit(0);
            return;
        }
        shell->write(buf, len);
    }
};
std::shared_ptr<IO>  io(new IO(ev));

int main()
{
    return ev->exec();
}
