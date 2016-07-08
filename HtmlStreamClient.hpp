#ifndef KITE_HtmlStreamClient_H
#define KITE_HtmlStreamClient_H

#include "HttpClient.hpp"
#include "EventLoop.hpp"

namespace Kite {

class HtmlStreamClient : public Kite::HttpClient
{
public:
    HtmlStreamClient(std::weak_ptr<Kite::EventLoop> ev);
    ~HtmlStreamClient();

    virtual void onHtmlOpenTag(const std::string &tag) {}
    virtual void onHtmlCloseTag(const std::string &tag) {}
    virtual void onHtmlAttribute(const std::string &key, const std::string &value) {}
    virtual void onHtmlText(const std::string &text) {}



protected:
    virtual void onReadActivated() override final;
    void *p;
};

}
#endif
