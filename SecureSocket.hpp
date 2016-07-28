#ifndef KITE_SSLSOCKET_HPP_KNMS
#define KITE_SSLSOCKET_HPP_KNMS

#include <string>
#include "EventLoop.hpp"
#include "IO.hpp"
#include "Scope.hpp"

namespace Kite {
    class SecureSocketPrivate;
    class SecureSocket : public Kite::IO, public Kite::Scope
    {
    public:
        enum SocketState {
            Disconnected       = 1,
            Connecting         = 2,
            Connected          = 3,
            SecureSetupError      = 4,
            SecurePeerNotVerified = 5,
            SecureClientCertificateRequired = 6,
            TransportErrror    = 7
        };

        SecureSocket(std::weak_ptr<Kite::EventLoop>);
        ~SecureSocket();

        bool setCaDir (const std::string &path);
        bool setCaFile(const std::string &path);

        bool setClientCertificateFile(const std::string &path);
        bool setClientKeyFile(const std::string &path);

        SocketState state() const;
        void connect(const std::string &hostname,
                int port, uint64_t timeout = 0, bool tls = false);
        void disconnect();

        int write(const char *data, int len);
        int read (char *data, int len);
        void flush();

        const std::string &errorMessage() const;
    protected:
        virtual void onConnected(){}
        virtual void onDisconnected(SocketState state){}

        std::weak_ptr<EventLoop> ev();
    private:
        friend class SecureSocketPrivate;
        SecureSocketPrivate *p;
    };
};

#endif
