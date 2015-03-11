#include <stdio.h>
#include <string>

#include "EventLoop.hpp"
#include "SecureSocket.hpp"

class MySocket: public Kite::SecureSocket
{
public:
    MySocket(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::SecureSocket(ev)
    {
    }

protected:
    virtual void onActivated(int) override
    {
        char buf [333];
        int r = read(buf, 100);
        buf[r] = 0;
        fprintf(stderr, "%s", buf);
        if (r == 0) {
            fprintf(stderr, "EOF\n");
            ev()->exit(0);
        }
    }
    virtual void onConnected() {
        fprintf(stderr, "connected!\n");
    }
    virtual void onDisconnected(SocketState state) {
        fprintf(stderr, "disconnected: %i %s \n", state, errorMessage().c_str());
        ev()->exit(9);
    }
};

int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<MySocket>        sock(new MySocket(ev));

    sock->setCaFile("/etc/x509/ca.crt") || fprintf(stderr, "it no werk!\n");
    sock->setClientCertificateFile("/etc/x509/host.crt") || fprintf(stderr, "it no werk!\n");
    sock->setClientKeyFile("/etc/x509/host.key")         || fprintf(stderr, "it no werk!\n");
    sock->connect("b1.broker.airspot.airfy.com", 443);

    return ev->exec();
}
