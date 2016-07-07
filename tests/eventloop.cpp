#include <iostream>

#include "EventLoop.hpp"
#include "Timer.hpp"
#include "dessert/dessert.hpp"

std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
std::shared_ptr<Kite::Timer>     t1;
std::shared_ptr<Kite::Timer>     t2;

desserts("ev delete then expire") {
    class T2: public Kite::Timer
    {
    public:
        T2(std::weak_ptr<Kite::EventLoop> ev)
            : Kite::Timer(ev)
        {
        }
    protected:
        virtual bool onExpired() override
        {
            dessert(false);
            return false;
        }
    };
    class T1: public Kite::Timer
    {
    public:
        T1(std::weak_ptr<Kite::EventLoop> ev)
            : Kite::Timer(ev)
        {
        }
    protected:
        virtual bool onExpired() override
        {
            t2.reset();
            return false;
        }
    };

    t1.reset(new T1(ev));
    t2.reset(new T2(ev));

    t1->reset(10);
    t2->reset(12);

    Kite::Timer::later(ev, [](){ev->exit(); return false;}, 20);
    ev->exec();
}

int main(){}
