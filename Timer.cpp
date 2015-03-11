#include "Timer.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <algorithm>
#include <time.h>

using namespace Kite;



Timer::Timer(const std::weak_ptr<EventLoop> &ev, uint64_t exp)
    :p_ev(ev)
    ,p_expires(0)
    ,p_period_intent(0)
{
    reset(exp);
}
Timer::~Timer()
{
    reset(0);
}

uint64_t Timer::reset(uint64_t exp)
{
    p_period_intent = exp;
    p_expires = exp;
    uint64_t el = elapsed();

    struct timespec begin;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &begin)) {
        //TODO  Oops, getting clock time failed
        std::cerr << "Kite::Timer: no clock :(" << std::endl;
        p_start = 0;
        return el;
    }
    p_start = begin.tv_sec  * 1000
            + begin.tv_nsec * 1e-6;

    // add if was dead before and now active
    auto ev = p_ev.lock();
    if (ev) {
        if (p_expires > 0) {
            ev->p_timers.insert(this);
        } else {
            ev->p_timers.erase(ev->p_timers.find(this), ev->p_timers.end());
        }
    }
    return el;
}


uint64_t Timer::elapsed()
{
    struct timespec current;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &current)) {
        //TODO  Oops, getting clock time failed
        std::cerr << "Kite::Timer: no clock :(" << std::endl;
        return 1;
    }

    /* Start time in millis */
    uint64_t now = current.tv_sec  * 1000
                 + current.tv_nsec * 1e-6;
    return now - p_start;
}

uint64_t Timer::expires()
{
    if (p_expires == 0)
        return 0;

    uint64_t el = elapsed();
    if (el >= p_expires) {
        p_expires = 0;
        return 0;
    }
    return p_expires - el;
}


void Timer::doExpire()
{
    if (onExpired()) {
        reset(p_period_intent);
    } else {
        reset(0);
    }
}

class Once : public Timer
{
public:
    Once(const std::weak_ptr<Kite::EventLoop> &ev, const std::function<void()> &fn)
        : Timer(ev)
        , fn(fn)
    {
    }
private:
    std::function<void()> fn;
    virtual bool onExpired()
    {
        fn();
        delete this;
        return false;
    }
};


void Timer::later(const std::weak_ptr<Kite::EventLoop> &ev, const std::function<void()> &fn, uint64_t ms)
{
    (new Once(ev, fn))->reset(ms);
}

