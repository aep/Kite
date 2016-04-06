#ifndef KITE_UDPSERVER_HPP_KNMSS
#define KITE_UDPSERVER_HPP_KNMSS

#include <string>
#include <memory>
#include "EventLoop.hpp"

namespace Kite {

    class UdpServer;
    class InternetAddress
    {
    public:

        enum SpecialAddress
        {
            Any
        };

        InternetAddress(SpecialAddress sp = Any, uint16_t port = 0);
        InternetAddress(const std::string &str, uint16_t port = 0);
        ~InternetAddress();

        std::string address() const;
        uint16_t port() const;

        bool operator==(const InternetAddress& other) const;
    private:
        struct space {char space[60];};
        std::shared_ptr <space> p;
        friend class UdpServer;
    };

    class UdpServer : public Kite::Evented
    {
    public:
        UdpServer(std::weak_ptr<Kite::EventLoop>);
        ~UdpServer();

        bool listen (const InternetAddress &address);

        int send    (const char *data, int len, const InternetAddress &address);
        int receive (char *data, int len,  InternetAddress *address);

    private:
        int p_af;
        int p_fd;
    };

};

#endif
