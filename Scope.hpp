#ifndef KITE_CAPTURESCOPE_HPP_KNMS
#define KITE_CAPTURESCOPE_HPP_KNMS

#include <vector>
#include <algorithm>
#include <cassert>

/*
 * TODO: revalidate 2017 if C++ finally fixed this
 * hint: weak_from_this won't do it, because it'll still just throw if there is no shared_ptr holding it
 */

namespace Kite  {

    class NoCopy {
    public:
        NoCopy() {}
    private:
        NoCopy(const NoCopy&o) { throw "NOCOPY"; }
    };

    class Scope;
    class DeathNotificationReceiver {
    public:
        DeathNotificationReceiver();
        DeathNotificationReceiver(Scope *c);
        DeathNotificationReceiver(const DeathNotificationReceiver& a);
        ~DeathNotificationReceiver();
        bool isDead() const;
    protected:
        void setNotificationScope(Scope *);
        virtual void onDeathNotify(Scope *) {}
    private:
        friend class Scope;
        bool dd_scope_captured;
        Scope *dd_cscope;
        void doDeathNotify(Scope *);
    };

    class Scope {
    public:
        virtual ~Scope();
        void addDeathNotificationReceiver(DeathNotificationReceiver *c);
        void removeDeathNotificationReceiver(DeathNotificationReceiver *c);
    private:
        std::vector<DeathNotificationReceiver*> dd_notifications;
    };



    template <class R>
    class ScopePtr : public DeathNotificationReceiver {
    public:
        ScopePtr(R*);
        R* operator ->();
        R* operator *();
    private:
        R *d__r;
    };

    template <class R>
        ScopePtr<R>::ScopePtr(R* r)
        : DeathNotificationReceiver(r)
        , d__r(r)
    {
    }

    template <class R>
        R* ScopePtr<R>::operator ->()
        {
            assert(!isDead());
            return d__r;
        }

    template <class R>
        R* ScopePtr<R>::operator *()
        {
            assert(!isDead());
            return d__r;
        }
}

#endif
