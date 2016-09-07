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
    void post(const std::string &url, size_t io_len);
    void setBodyBufferSize(int size);

    int writeBody(const char *data, int len);


    typedef std::function<void(Status status, int responseCode, const std::string &body)> CompletedCallback;
    static void post(std::weak_ptr<Kite::EventLoop> ev, const std::string &url, const std::string &body, CompletedCallback = CompletedCallback());
    static void get (std::weak_ptr<Kite::EventLoop> ev, const std::string &url, CompletedCallback = CompletedCallback());

protected:
    virtual void onHeadersReady(int responseCode, const std::map<std::string,std::string> &responseHeaders) {}
    virtual void onFinished(Status status, int responseCode, const std::string &body){};
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
