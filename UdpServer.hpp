#ifndef KITE_UDPSERVER_HPP_KNMSS
#define KITE_UDPSERVER_HPP_KNMSS

#include <string>
#include <memory>
#include "EventLoop.hpp"
#include "Internet.hpp"

namespace Kite {

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
