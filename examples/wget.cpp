#include <iostream>

#include "EventLoop.hpp"
#include "HttpClient.hpp"
#include <unistd.h>


class MyClient : public Kite::HttpClient
{
public:
    MyClient(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::HttpClient(ev)
    {
    }
protected:

    virtual void onReadActivated() override
    {
        char buf[1024];
        int len = read(buf, 1024);
        if (len < 1) {
            disconnect();
            return;
        }
        ::write(fileno(stdout), buf, len);
    }
    virtual void onFinished(Kite::HttpClient::Status status, int exitCode, const std::string& body) override
    {
        ev()->exit(0);
    }
};

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<MyClient>        client(new MyClient(ev));

    client->get(argv[1]);
    return ev->exec();
}
