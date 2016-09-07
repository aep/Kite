#include <iostream>

#include "EventLoop.hpp"
#include "HttpClient.hpp"
#include "Stdio.hpp"
#include "dessert/dessert.hpp"


desserts("google") {

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

            if (e++ == 0) {
                std::map<std::string, std::string> headers;
                headers["X-DERP"]=  "funnies";
                setHeaders(headers);
                post("http://localhost:8000/", "turbo");
            } else {
                ev().lock()->exit(9);
            }
        }
    };

    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);

    //will timeout
    Kite::HttpClient::get(ev, "http://localhost:4000", [ev](Kite::HttpClient::Status status, int responseCode, const std::string &body){

            std::cerr << "1 " << responseCode << std::endl;

            MyClient *client(new MyClient(ev));
            client->setCaFile("/etc/ssl/cert.pem");
            client->get("https://google.com/stulle/hans");
    });


    std::cerr << "ev::exec" << std::endl;

    ev->exec();
}

int main(){}
