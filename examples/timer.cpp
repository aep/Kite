#include <stdio.h>
#include <string>

#include "EventLoop.hpp"
#include "Timer.hpp"

class Clock: public Kite::Timer
{
public:
    Clock(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::Timer(ev)
        , wall(ev)
    {
    }
    Kite::Timer wall;

protected:
    virtual bool onExpired() override
    {
        static int i = 0;
        if (++i > 5)
            ev()->exit();

        fprintf(stderr, "tick at %d ms\n", wall.elapsed());
        return true;
    }
};

int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Clock>        clock(new Clock(ev));
    clock->reset(500);
    return ev->exec();
}
