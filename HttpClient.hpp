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

    void get(const std::string &url);

protected:
    virtual void onFinished(Status status, int responseCode, const std::string &body) = 0;



    void onActivated (int) override;
    void onConnected () override;
    void onDisconnected(SocketState state) override;

private:
    friend class HttpClientPrivate;
    HttpClientPrivate *p;
};

}
#endif
