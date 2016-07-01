#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

#include "EventLoop.hpp"
#include "Serialport.hpp"
#include "Stdio.hpp"

class Miniterm: public Kite::Serialport
{
public:
    Miniterm(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::Serialport(ev)
    {
        open("/dev/ttyUSB0", 115200);
        write("ls\n", 3);
    }

protected:
    virtual void onActivated(int,int) override
    {
        char buf[1024];
        int len = read(buf, 1024);
        if (len == 0) {
            close();
            ev()->exit(0);
            return;
        }
        ::write(fileno(stdout), buf, len);
    }
};

std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
std::shared_ptr<Miniterm>         s(new Miniterm(ev));

class Stdio: public Kite::Stdio
{
public:
	Stdio(std::weak_ptr<Kite::EventLoop> ev)
		: Kite::Stdio(ev)
	{}
protected:
	virtual void onActivated(int,int) override
	{
		char buf[1024];
		int len = read(buf, 1024);
		s->write(buf, len);
	}
};

std::shared_ptr<Stdio>         io(new Stdio(ev));

int main()
{
    return ev->exec();
}
