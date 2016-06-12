#include <stdio.h>
#include <string>

#include "EventLoop.hpp"
#include "Stdio.hpp"
#include "Timer.hpp"

class Echo: public Kite::Stdio
{
public:
    Echo(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::Stdio(ev)
    {
    }

protected:
    virtual void onActivated(int) override
    {
        char buf[1024];
        int len = read(buf, 1024);
        write(buf, len);
        if (len == 0) {
            ev()->exit();
        }
    }
};

int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Echo>        clock(new Echo(ev));
    return ev->exec();
}
