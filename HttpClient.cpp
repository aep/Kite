#include "HttpClient.hpp"
#include "Timer.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

using namespace Kite;


class Kite::HttpClientPrivate
{
public:
    HttpClientPrivate()
    {
        state = 0;
        responseCode = 999;
    }

    void setUrl(const std::string &url, const std::string &verb)
    {
        d_path = "/";
        d_host.clear();
        d_verb = verb;
        p_buf.clear();

        std::istringstream ss(url);
        std::string token;

        int at = 0;

        while (std::getline(ss, token, '/')) {

            if (at == 0) {
                if (token == "https:") {
                    d_is_https = true;
                } else if (token == "http:") {
                    d_is_https = false;
                } else {
                    throw std::invalid_argument("unsupported url schema " + token);
                }
            } else if (at == 2) {
                std::istringstream s2(token);
                std::getline(s2, d_host, ':');
                std::getline(s2, token,  ':');
                try {
                    d_port = std::stoi(token);
                } catch (std::invalid_argument&) {
                    d_port = d_is_https ? 443 : 80;
                }
            } else if (at > 2) {
                d_path += "/" + token;
            }
            ++at;
        }
    }

    void onLine(const std::string &line)
    {
        if (state == Kite::HttpClient::Connected) {
            std::istringstream ss(line);
            std::string httpd;
            ss >> httpd;
            ss >> responseCode;
            state = Kite::HttpClient::StatusCompleted;
        } else {
            if (line == "\r\n") {
                state = HttpClient::HeaderCompleted;
            }
        }
    }

    std::map<std::string,std::string> headers;

    int responseCode;
    HttpClient::Status status;
    int state;

    std::string d_verb;
    bool        d_is_https;
    std::string d_host;
    int         d_port;
    std::string d_path;
    std::string d_post_body;
    friend class HttpClient;

    std::string p_buf;


    HttpClient *p;
};


///////////////////////////////////////////////////////////////////////////////


HttpClient::HttpClient(std::weak_ptr<Kite::EventLoop> ev)
    : Kite::SecureSocket(ev)
    , p(new HttpClientPrivate)
{
    p->p = this;
}

HttpClient::~HttpClient()
{
    delete p;
}

void HttpClient::get(const std::string &url)
{
    p->setUrl(url, "GET");
    p->status = Kite::HttpClient::Connecting;

    connect(p->d_host, p->d_port, 5000, p->d_is_https);
}

void HttpClient::post(const std::string &url, const std::string &body)
{
    p->setUrl(url, "POST");
    p->status = Kite::HttpClient::Connecting;
    p->d_post_body = body;

    connect(p->d_host, p->d_port, 5000, p->d_is_https);
}

void HttpClient::onActivated(int)
{
    if (p->state < HttpClient::HeaderCompleted) {
        if (p->p_buf.length() >= 4048) {
            throw std::runtime_error("overflow");
        }
        char c;
        int len = read(&c, 1);
        if (len != 1) {
            disconnect();
            return;
        }

        p->p_buf += c;
        if (c == '\n') {
            p->onLine(p->p_buf);
            p->p_buf.clear();
        }
        //return get activated again.
        //TODO overhead ok for header?
    } else {
        onReadActivated();
    }
}


void HttpClient::onReadActivated()
{
    char buf[1024];
    int len = read(buf, 1024);
    if (len < 1) {
        disconnect();
        return;
    }
    if (p->p_buf.length() + len >= 4048) {
        throw std::runtime_error("overflow");
    }
    p->p_buf += std::string(buf, len);
}

void HttpClient::onDisconnected(SocketState state)
{
    onFinished(p->status, p->responseCode, p->p_buf);
}

void HttpClient::onConnected() {
    p->state =  Kite::HttpClient::Connected;

    std::stringstream ss;

    ss << p->d_verb << " " << p->d_path << " HTTP/1.1\r\n";
    ss << "Host: " << p->d_host << "\r\n";

    for (auto it = p->headers.begin(); it != p->headers.end(); ++it) {
        ss << it->first << ": " << it->second << "\r\n";
    }

    if (p->d_verb == "POST") {
        ss << "Content-Length: " << p->d_post_body.length() << "\r\n";
    }
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << p->d_post_body;

    write(ss.str().c_str(), ss.str().length());
}


void HttpClient::setHeaders(std::map<std::string,std::string> headers)
{
    p->headers = headers;
}


