#ifndef KITE_ELAPSED_TIME_HPP_KLM
#define KITE_ELAPSED_TIME_HPP_KLM

#include <cstdint>
#include <memory>
#include "Scope.hpp"


/** Timer
 **
 ** supports two time operations:
 ** 1. calling onExpired() after a period of time (in ms)
 ** 2. getting the time elapsed since the last call to reset()
 **/



#define  KITE_TIMER_DEBUG_NAME(timer, name) (timer)->k__debugName = name;

namespace Kite {
    class EventLoop;
    class Once;
    class Timer {
    public:
        static uint64_t now();

        static void later(const std::weak_ptr<Kite::EventLoop> &ev,
                const std::function<bool()> &fn,
                uint64_t ms = 1,
                Scope *scope = 0,
                const char *name = "later");

        Timer (const std::weak_ptr<Kite::EventLoop> &ev, uint64_t expire = 0);
        ~Timer();
        uint64_t reset(uint64_t expire = 0);
        uint64_t elapsed();
        uint64_t expires();

        const char *k__debugName;

    protected:
        /* called periodically
         *
         * return false to stop the timer and true to continue
         */
        virtual bool onExpired(){return false;}

        inline std::weak_ptr<EventLoop> ev() const { return p_ev;}
    protected:
        uint64_t p_period_intent;
    private:
        virtual void doExpire();
        uint64_t p_start;
        uint64_t p_expires;
        std::weak_ptr<EventLoop> p_ev;
        friend class EventLoop;
        friend class Once;
    };
};
#endif
