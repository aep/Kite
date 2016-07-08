#include <iostream>
#include "EventLoop.hpp"
#include "HtmlStreamClient.hpp"
#include <unistd.h>
#include <stdio.h>

class Crawler : public Kite::HtmlStreamClient
{
    bool is_title;
    bool is_a;
    std::string title;
    std::string href;
public:
    Crawler(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::HtmlStreamClient(ev)
    {
        is_title = false;
        is_a = false;
    }
    virtual void onHtmlOpenTag(const std::string &tag)
    {
        title.clear();
        if (tag == "title")
            is_title = true;
        else if (tag == "a")
            is_a = true;
    }

    virtual void onHtmlCloseTag(const std::string &tag)
    {
        if (tag == "title") {
            std::cout << title << std::endl;
        } else if (tag == "a") {
            std::cout << "  " << title << " -> " << href << std::endl;
        }
        title.clear();
    }

    virtual void onHtmlText(const std::string &t)
    {
        if (is_title || is_a) {
            title += t;
        }
    }
    virtual void onHtmlAttribute(const std::string &key, const std::string &value) {
        if (key == "href")
            href = value;
    }

    virtual void onFinished(Kite::HttpClient::Status , int , const std::string&) override
    {
        ev()->exit(0);
    }

};

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Crawler>    client(new Crawler(ev));

    client->get(argv[1]);
    return ev->exec();
}
