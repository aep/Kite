#include <stdio.h>
#include <string>
#include "dessert/dessert.hpp"

#include "EventLoop.hpp"
#include "Timer.hpp"


desserts("clock ticks") {
    static int interval = 500;
    static int ticks = 0;
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
            int ee = wall.elapsed();


            if (++ticks > 5)
                ev()->exit();

            int expected_interval = ticks * interval;
            dessert(abs(expected_interval - ee) < 2) << "drift";

            fprintf(stderr, "tick at %d ms\n", ee);
            return true;
        }
    };
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Clock>        clock(new Clock(ev));
    clock->reset(interval);
    ev->exec();
    dessert(ticks == 6);
}

int main(){}
