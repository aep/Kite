#include "DomainNameResolver.hpp"
using namespace Kite;
#include "3rdparty/dns/dns.c"

#include <iostream>

DomainNameResolver::DomainNameResolver(std::weak_ptr<Kite::EventLoop> ev)
    : Kite::File(ev)
    , Kite::Timer(ev)
    , Kite::Scope()
    , pDnsServer("8.8.8.8")
    , pTimeout(5000)
{
    pDns = 0;
}

DomainNameResolver::~DomainNameResolver()
{
    if (pDns) {
        dns_close((dns_t*)pDns);
        free(pDns);
    }
    for (auto q : pQ) {
        q.second.reject(std::underflow_error("canceled"));
    }
    pQ.clear();
}

Promise<std::vector<DomainNameRecord>>
DomainNameResolver::resolve(const std::string &name, DomainNameRecordType requestedType)
{
    if (!pDns) {
        pDns = malloc(sizeof(dns_t));
        memset(pDns, 0, sizeof(dns_t));
        dns_t *dns = (dns_t*)pDns;
        dns_init(dns, pDnsServer.c_str());
        setFile(dns->socket);
    }
    dns_t *dns = (dns_t*)pDns;
    if (dns_request(dns, name.c_str()) != 0) {
        std::cerr << "Kite::DomainNameResolver dns_request failed" << std::endl;
    }
    reset(pTimeout);

    Promise<std::vector<DomainNameRecord>> promise;
    pQ.insert(std::make_pair(std::pair<std::string, DomainNameRecordType>(std::string("22"), requestedType),promise));
    return promise;
}

void DomainNameResolver::setDomainServer(const std::string &ip)
{
    pDnsServer = ip;
}

void DomainNameResolver::setTimeout(int ms)
{
    pTimeout = ms;
}

void DomainNameResolver::onActivated(int, int)
{
    if (!pDns) {
        throw std::runtime_error("DomainNameResolver::onActivated without init");
    }
    dns_t *dns = (dns_t*)pDns;
    if (int e = dns_receive(dns)) {
        onFinished(MaybeError::Invalid, std::vector<DomainNameRecord>());
        for (auto q : pQ) {
            q.second.reject(std::domain_error(std::to_string(e)));
        }
        pQ.clear();
        return;
    }

    std::vector<DomainNameRecord> records;

    for(dns_record_t *r = dns->records; r->ttl != 0; r++) {
        DomainNameRecord rec;
        rec.type = AddressRecord;
        rec.ttl = r->ttl;

        char bb[24];
        sprintf(bb, "%hhu.%hhu.%hhu.%hhu",
                (r->address >> 0 ) & 0xFF,
                (r->address >> 8 ) & 0xFF,
                (r->address >> 16) & 0xFF,
                (r->address >> 24) & 0xFF);
        rec.address = InternetAddress(bb);

        records.push_back(rec);
    }

    //FIXME: we're basically only allowing one request at a time,
    //because we don't check here which request that answer was for
    onFinished(OK, records);
    for (auto q : pQ) {
        q.second.accept(records);
    }
    pQ.clear();
}

bool DomainNameResolver::onExpired()
{
    onFinished(MaybeError::Timeout, std::vector<DomainNameRecord>());
    for (auto q : pQ) {
        q.second.reject(std::underflow_error("timeout"));
    }
    pQ.clear();
}

Promise<std::vector<DomainNameRecord>> DomainNameResolver::resolve(
        std::weak_ptr<Kite::EventLoop> ev,
        const std::string &domain, DomainNameRecordType requestedType,
        const std::string &dnsServer, int timeout)
{
    auto p = new DomainNameResolver(ev);
    p->setDomainServer(dnsServer);
    p->setTimeout(timeout);
    auto q = p->resolve(domain, requestedType);
    q.finally([p,ev]() {
        ev.lock()->deleteLater(p);
    });
    return q;
}

