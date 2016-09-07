#include <iostream>

#include "EventLoop.hpp"
#include "HttpClient.hpp"
#include "Stdio.hpp"


class MyClient : public Kite::HttpClient
{
public:
    MyClient(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::HttpClient(ev)
    {
    }
protected:
    int e = 0;
    virtual void onFinished(Status status, int responseCode, const std::string &body) override
    {
        std::cerr << errorMessage() << std::endl;
        std::cerr << responseCode << std::endl;
        std::cerr << body;

        ev().lock()->exit(9);
    }
};

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<MyClient>        client(new MyClient(ev));

    client->setCaFile("/etc/ssl/cert.pem");
    client->get("http://192.168.10.57/istatus.htm");

    return ev->exec();
}
