#include "Scope.hpp"

using namespace Kite;


bool DeathNotificationReceiver::isDead() const
{
    return dd_scope_captured && (dd_cscope == nullptr);
}

DeathNotificationReceiver::DeathNotificationReceiver(Scope *c)
{
    dd_scope_captured = (c != nullptr);
    dd_cscope = c;
    if (dd_cscope) {
        dd_cscope->addDeathNotificationReceiver(this);
    }
}

DeathNotificationReceiver::~DeathNotificationReceiver()
{
    if (dd_cscope) {
        dd_cscope->removeDeathNotificationReceiver(this);
    }
}

void DeathNotificationReceiver::doDeathNotify(const void *t)
{
    onDeathNotify(t);
    dd_cscope = nullptr;
}

Scope::~Scope()
{
    for (auto o : dd_notifications) { o->doDeathNotify(this);}
}
void Scope::addDeathNotificationReceiver(DeathNotificationReceiver *c)
{
    dd_notifications.push_back(c);
}

void Scope::removeDeathNotificationReceiver(DeathNotificationReceiver *c)
{
    dd_notifications.erase(std::remove(dd_notifications.begin(), dd_notifications.end(),
                c), dd_notifications.end());

}

namespace Kite {
template <> ScopePtr<Scope>::ScopePtr(Scope* r)
    : DeathNotificationReceiver(r)
      , d__r(r)
{
}
}

