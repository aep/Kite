#ifndef KITE_CAPTURESCOPE_HPP_KNMS
#define KITE_CAPTURESCOPE_HPP_KNMS

#include <vector>
#include <algorithm>

namespace Kite  {
    class Scope;
    class DeathNotificationReceiver {
    public:
        DeathNotificationReceiver(Scope *c);
        ~DeathNotificationReceiver();
    protected:
        virtual void onDeathNotify(const void *) {}
        bool isDead() const;
    private:
        friend class Scope;
        bool dd_scope_captured;
        Scope *dd_cscope;
        void doDeathNotify(const void *);
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
