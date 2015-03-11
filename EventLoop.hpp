#ifndef KITE_EVENTLOOP_HPP_KNMS
#define KITE_EVENTLOOP_HPP_KNMS

#include <map>
#include <memory>
#include <functional>
#include <queue>
#include <unordered_set>

namespace Kite  {

    class Timer;
    class EventLoop;
    class Evented {
    public:
        Evented(std::weak_ptr<EventLoop> ev);
        virtual ~Evented();

    protected:
        void evAdd(int);
        void evRemove(int);

        inline std::shared_ptr<EventLoop> ev() const { return p_Ev.lock();}
    private:
        friend class EventLoop;
        virtual void onActivated(int) = 0;
        std::weak_ptr<EventLoop> p_Ev;
    };

    class EventLoop : public std::enable_shared_from_this<EventLoop> {
    public:
        EventLoop();
        ~EventLoop();
        int exec();
        void exit(int exitCode = 0);
    private:
        friend class Timer;
        friend class Evented;
        std::map <int, Evented*> p_evs;
        std::unordered_set<Timer* > p_timers;
        bool p_running;
        int  p_exitCode;
        int  p_intp[2];
    };
}

#endif
