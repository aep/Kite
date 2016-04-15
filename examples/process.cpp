#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

#include "EventLoop.hpp"
#include "File.hpp"

class LS: public Kite::File
{
public:
    LS(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::File(ev)
    {
        FILE *f = popen("ls /", "r");
        int fd = fileno(f);
//        fcntl(fd, F_SETFL, O_NONBLOCK);
        setFile(fd);
    }

protected:
    virtual void onActivated(int) override
    {
        char buf[1024];
        int len = read(buf, 1024);
        std::cerr << len << std::endl;
        if (len == 0) {
            ev()->exit(0);
            return;
        }
        ::write(fileno(stdout), buf, len);
    }
};

int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<LS>         ls(new LS(ev));
    return ev->exec();
}
