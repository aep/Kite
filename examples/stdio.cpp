#include <stdio.h>
#include <string>

#include "EventLoop.hpp"
#include "Stdio.hpp"
#include "Timer.hpp"

class Echo: public Kite::Stdio, public Kite::Timer
{
public:
    Echo(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::Stdio(ev)
        , Kite::Timer(ev)
    {
        reset(2000);
    }

protected:
    virtual bool onExpired() override
    {
        write("tick\n", 5);
        return true;
    }
    virtual void onActivated(int,int) override
    {
        char buf[1024];
        int len = read(buf, 1024);
        write(buf, len);
    }
};

int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Echo>        clock(new Echo(ev));
    return ev->exec();
}
