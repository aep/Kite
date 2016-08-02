#ifndef KITE_INTERNET_HPP_KNMSS
#define KITE_INTERNET_HPP_KNMSS

#include <string>
#include <memory>

namespace Kite {
    class UdpServer;
    class TcpServer;
    class TcpConnection;
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
        bool operator<(const Kite::InternetAddress &other) const;
    private:
        struct space {char space[60];};
        std::shared_ptr <space> p;
        friend class UdpServer;
        friend class TcpConnection;
        friend class TcpServer;
    };
};

#endif
