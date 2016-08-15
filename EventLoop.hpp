#ifndef KITE_EVENTLOOP_HPP_KNMS
#define KITE_EVENTLOOP_HPP_KNMS

#include <map>
#include <memory>
#include <functional>
#include <queue>
#include <unordered_set>

#include "Scope.hpp"

namespace Kite  {

    class Timer;
    class EventLoop;
    class Evented {
    public:
        enum EventTypes {
            Read      = 0x1,
            Write     = 0x2,
            Exception = 0x4
        };
        Evented(std::weak_ptr<EventLoop> ev);
        virtual ~Evented();

        static void later(
                const std::weak_ptr<Kite::EventLoop> &ev,
                int fd, EventTypes types,
                const std::function<bool()> &fn,
                Scope *scope,
                const char *name = "later");

        inline std::weak_ptr<EventLoop> ev() const { return p_Ev;}

    protected:
        void evAdd(int fd, int events = Read);
        void evRemove(int);

        void evAddSignal(int signal);
        void evRemoveSignal(int signal);

    private:
        friend class EventLoop;
        virtual void onActivated(int fd, int events) = 0;
        virtual void onSignal(int signal, int fd) {};
        std::weak_ptr<EventLoop> p_Ev;
    };


    class EventLoop : public std::enable_shared_from_this<EventLoop>, public Scope {
    public:
        EventLoop();
        virtual ~EventLoop();
        int exec();
        void exit(int exitCode = 0);

        void deleteLater(Scope *);

    private:
        friend class Timer;
        friend class Evented;
        std::unordered_set <Evented*> p_signalevs;
        std::map <int, std::pair<Evented*, int> > p_evs;
        std::unordered_set<Timer* > p_timers;
        std::vector<ScopePtr<Scope> > p_deleteme;
        bool p_running;
        int  p_exitCode;
        int  p_intp[2];
        int  p_signalfd;
    };
}

#endif
