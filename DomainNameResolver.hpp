#ifndef KITE_DOMAINNAMERESOLVER_HPP_asd
#define KITE_DOMAINNAMERESOLVER_HPP_asd

#include <string>
#include "EventLoop.hpp"
#include "File.hpp"
#include "Timer.hpp"
#include "Internet.hpp"
#include "Promises.hpp"

namespace Kite {
    enum DomainNameRecordType {
        UnknownRecord = 0,
        AddressRecord = 'A'
    };
    struct DomainNameRecord
    {
        std::string name;
        DomainNameRecordType type;
        int ttl;
        InternetAddress address;
    };

    class DomainNameResolver : public Kite::File, public Kite::Timer, public Kite::Scope, public NoCopy
    {
    public:
        enum MaybeError {
            OK      = 0,
            Invalid = 1,
            Timeout = 2
        };

        DomainNameResolver(std::weak_ptr<Kite::EventLoop>);
        ~DomainNameResolver();

        void setDomainServer(const std::string &ip);
        void setTimeout(int ms);

        Promise<std::vector<DomainNameRecord>>
            resolve(const std::string &name, DomainNameRecordType requestedType = AddressRecord);

        static Promise<std::vector<DomainNameRecord>> resolve(
                std::weak_ptr<Kite::EventLoop> ev,
                const std::string &domain, DomainNameRecordType requestedType = AddressRecord,
                const std::string &dnsServer = "8.8.8.8", int timeout = 5000);
    private:
        virtual void onActivated(int, int) override final;
        virtual bool onExpired() override final;

    protected:
        virtual void onFinished(MaybeError err, const std::vector<DomainNameRecord> &records){}

    private:
        std::string pDnsServer;
        int pTimeout;
        void *pDns;
        std::map<std::pair<std::string, DomainNameRecordType>, Promise<std::vector<DomainNameRecord>>> pQ;

    };
};

#endif
