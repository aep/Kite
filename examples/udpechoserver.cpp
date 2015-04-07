#include <stdio.h>
#include <string>

#include "EventLoop.hpp"
#include "UdpServer.hpp"

class Echo: public Kite::UdpServer
{
public:
    Echo(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::UdpServer(ev)
    {
    }

protected:
    virtual void onActivated(int) override
    {
        Kite::InternetAddress src;
        char buf[1024];
        int len = receive(buf, 1024, &src);

        fprintf(stdout, "%s:%d =>  %.*s\n", src.address().c_str(), src.port(), len, buf);

        send(buf, len, src);
    }
};

int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Echo>        echo(new Echo(ev));

    echo->listen(Kite::InternetAddress(Kite::InternetAddress::Any, 1234));

    return ev->exec();
}
