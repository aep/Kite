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

    k__debugName = "";
}
Timer::~Timer()
{
    reset(0);
}

uint64_t Timer::now()
{
    struct timespec begin;
    if (clock_gettime(CLOCK_REALTIME, &begin)) {
        throw std::runtime_error("Kite::Timer: no clock :(");
    }
    return begin.tv_sec  * 1000
         + begin.tv_nsec * 1e-6;
}

uint64_t Timer::reset(uint64_t exp)
{
    p_period_intent = exp;
    p_expires = exp;
    uint64_t el = elapsed();

    struct timespec begin;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &begin)) {
        throw std::runtime_error("Kite::Timer: no clock :(");
        p_start = 0;
        return el;
    }
    p_start = begin.tv_sec  * 1000
            + begin.tv_nsec * 1e-6;

    // add if was dead before and now active
    auto ev = p_ev.lock();
    if (ev) {
        if (p_expires > 0) {
            int cbefore = ev->p_timers.size();
            ev->p_timers.insert(this);
   //         std::cerr << "T " << this <<  " " <<  k__debugName << " insert " << exp << " " << cbefore << "->" << ev->p_timers.size() << std::endl;
        } else {
            auto fi = ev->p_timers.find(this);
            if (fi != ev->p_timers.end()) {
            int cbefore = ev->p_timers.size();
                ev->p_timers.erase(fi);
    //           std::cerr << "T " << this <<  " " <<  k__debugName << " erase " << " " << cbefore << "->" << ev->p_timers.size() << std::endl;
            }
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

class Kite::Once : public Timer
{
public:
    Once(const std::weak_ptr<Kite::EventLoop> &ev, const std::function<bool()> &fn)
        : Timer(ev)
        , fn(fn)
    {
    }
private:
    std::function<bool()> fn;
    //must be last on the callstack inside EventLoop because of delete this
    virtual void doExpire() override
    {
        if (fn()) {
            reset(p_period_intent);
        } else {
            reset(0);
            delete this;
        }
    }
};


void Timer::later(const std::weak_ptr<Kite::EventLoop> &ev, const std::function<bool()> &fn, uint64_t ms, const char *name)
{
    Once *o = new Once(ev, fn);
    o->k__debugName = name;
    o->reset(ms);

}

