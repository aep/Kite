#include "TcpServer.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory>
#include <strings.h>
#include <arpa/inet.h>
#include <fcntl.h>
using namespace Kite;

TcpServer::TcpServer(std::weak_ptr<Kite::EventLoop> ev, Factory factory)
    : Kite::Evented (ev)
    , p_fd(0)
    , factory(factory)
{
}

TcpServer::~TcpServer()
{
}

void TcpServer::onActivated(int, int)
{
    InternetAddress address;
    bzero(address.p.get(), sizeof(sockaddr_in));

    socklen_t len = sizeof(sockaddr_in);

    int nfd = accept(p_fd, (struct sockaddr *)(address.p.get()), &len);
    if (nfd < 0)  {
        perror("Kite::TcpServer::accept");
        return;
    }
    onNewConnection(nfd, address);
}

bool TcpServer::listen (const InternetAddress &address)
{
    sockaddr_in *p_a = (sockaddr_in*)address.p.get();
    struct sockaddr_in servaddr;
    p_fd = socket(p_a->sin_family, SOCK_STREAM, 0);

    int reuse = 1;
    if (setsockopt(p_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    if (::bind(p_fd,(struct sockaddr *)address.p.get(), sizeof(sockaddr_in)) != 0) {
        // TODO
        perror("listen failed");
        return false;
    }

    if (::listen(p_fd, 0)) {
        // TODO
        perror("listen failed");
        return false;
    }

    fcntl(p_fd, F_SETFL, O_NONBLOCK);
    evAdd(p_fd);

    return true;
}



void TcpServer::onNewConnection(int fd, const InternetAddress &address)
{
    if (factory) {
        factory(ev(), fd, address);
    }
}

