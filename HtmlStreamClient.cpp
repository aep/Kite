#include "HtmlStreamClient.hpp"
#include "3rdparty/htmlstream/htmlstream.h"

using namespace Kite;

HtmlStreamClient::HtmlStreamClient(std::weak_ptr<Kite::EventLoop> ev)
    : Kite::HttpClient(ev)
    , p(new htmlstream_t)
{
    htmlstream_t *ht = (htmlstream_t*)p;
    htmlstream_init(ht);
    ht->parg = this;
    ht->open_cb = [](void *parg, const char *tag, int len){
        ((HtmlStreamClient*)parg)->onHtmlOpenTag(std::string(tag,len));
    };
    ht->close_cb = [](void *parg, const char *tag, int len){
        ((HtmlStreamClient*)parg)->onHtmlCloseTag(std::string(tag,len));
    };
    ht->attr_cb = [](void *parg, const char *key, int key_l, const char *val, int val_l ){
        ((HtmlStreamClient*)parg)->onHtmlAttribute(std::string(key,key_l), std::string(val,val_l));
    };
    ht->text_cb = [](void *parg, const char *text, int len){
        ((HtmlStreamClient*)parg)->onHtmlText(std::string(text,len));
    };
}


HtmlStreamClient::~HtmlStreamClient()
{
    delete (htmlstream_t*)p;
}

void HtmlStreamClient::onReadActivated()
{
    char buf[1024];
    int len = read(buf, 1024);
    htmlstream_t *ht = (htmlstream_t*)p;
    if (len < 1) {
        disconnect();
        return;
    }
    htmlstream_feed(ht, buf, len);
}
