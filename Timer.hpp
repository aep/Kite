#ifndef KITE_ELAPSED_TIME_HPP_KLM
#define KITE_ELAPSED_TIME_HPP_KLM

#include <cstdint>
#include <memory>


/** Timer
 **
 ** supports two time operations:
 ** 1. calling onExpired() after a period of time (in ms)
 ** 2. getting the time elapsed since the last call to reset()
 **/

namespace Kite {
    class EventLoop;
    class Timer {
    public:
        static void later(const std::weak_ptr<Kite::EventLoop> &ev, const std::function<void()> &fn, uint64_t ms = 1);

        Timer (const std::weak_ptr<Kite::EventLoop> &ev = std::weak_ptr<Kite::EventLoop>(), uint64_t expire = 0);
        ~Timer();
        uint64_t reset(uint64_t expire = 0);
        uint64_t elapsed();
        uint64_t expires();
    protected:
        virtual bool onExpired(){}

        inline std::shared_ptr<EventLoop> ev() const { return p_ev.lock();}
    private:
        void doExpire();
        uint64_t p_period_intent;
        uint64_t p_start;
        uint64_t p_expires;
        std::weak_ptr<EventLoop> p_ev;
        friend class EventLoop;
    };
};
#endif
