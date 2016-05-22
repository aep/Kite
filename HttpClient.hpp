#ifndef KITE_HTTP_HPP_WARP_kan
#define KITE_HTTP_HPP_WARP_kan

#include "SecureSocket.hpp"
#include "EventLoop.hpp"

namespace Kite {

class HttpClientPrivate;
class HttpClient : public Kite::SecureSocket
{
public:
    enum Status {
        Undefined,
        Connecting,
        Connected,
        StatusCompleted,
        HeaderCompleted,
        Completed
    };

    HttpClient(std::weak_ptr<Kite::EventLoop> ev);
    ~HttpClient();


    void setHeaders(std::map<std::string,std::string> headers);
    void get(const std::string &url);
    void post(const std::string &url, const std::string &body);

protected:
    virtual void onFinished(Status status, int responseCode, const std::string &body) = 0;
    virtual void onReadActivated();

    void onActivated (int) override final;
    void onConnected () override final;
    void onDisconnected(SocketState state) override final;

private:
    friend class HttpClientPrivate;
    HttpClientPrivate *p;
};

}
#endif
