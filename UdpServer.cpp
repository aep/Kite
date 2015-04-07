#include "UdpServer.hpp"
#include <stdio.h>
#include <sstream>
#include <iostream>
#include "Timer.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory>
#include <strings.h>
#include <arpa/inet.h>
#include <fcntl.h>

using namespace Kite;

InternetAddress::InternetAddress(InternetAddress::SpecialAddress sa, uint16_t port)
    : p(new space)
{
    bzero(p.get(), sizeof(sockaddr_in));
    //TODO : v6
    sockaddr_in *p_a = (sockaddr_in*)p.get();
    p_a->sin_family = AF_INET;
    p_a->sin_port = htons(port);

    if (sa == InternetAddress::Any) {
        p_a->sin_addr.s_addr = htonl(INADDR_ANY);
    }

}

InternetAddress::InternetAddress(const std::string &address, uint16_t port)
    : p(new space)
{
    bzero(p.get(), sizeof(sockaddr_in));
    //TODO : v6
    sockaddr_in *p_a = (sockaddr_in*)p.get();
    p_a->sin_family = AF_INET;
    p_a->sin_port = htons(port);
    if (inet_pton(p_a->sin_family, address.c_str(), &p_a->sin_addr) != 1) {
        //TODO: throw
        std::cerr << "inet_aton failed" << std::endl;
    }
}
InternetAddress::~InternetAddress()
{
}

std::string InternetAddress::address() const
{
    sockaddr_in *p_a = (sockaddr_in*)p.get();
    char b[128];
    inet_ntop(p_a->sin_family, &p_a->sin_addr, b, 128);
    return std::string(b);
}

uint16_t InternetAddress::port() const
{
}

bool InternetAddress::operator==(const InternetAddress& other) const
{
    sockaddr_in *p_a = (sockaddr_in*)p.get();
    sockaddr_in *p_b = (sockaddr_in*)other.p.get();
    if (p_a->sin_addr.s_addr != p_b->sin_addr.s_addr)
        return false;
    if (p_a->sin_family != p_b->sin_family)
        return false;
    if (p_a->sin_port != p_b->sin_port)
        return false;

    return true;
}



UdpServer::UdpServer(std::weak_ptr<Kite::EventLoop> ev)
    : Evented(ev)
{

}


UdpServer::~UdpServer()
{
}

bool UdpServer::listen(const InternetAddress &address)
{
    sockaddr_in *p_a = (sockaddr_in*)address.p.get();
    struct sockaddr_in servaddr;
    p_fd = socket(p_a->sin_family, SOCK_DGRAM, 0);

    if (bind(p_fd,(struct sockaddr *)address.p.get(), sizeof(sockaddr_in)) != 0) {
        // TODO
        perror("listen failed");
        return false;
    }

    fcntl(p_fd, F_SETFL, O_NONBLOCK);
    evAdd(p_fd);

    return true;
}

int UdpServer::send (const char *data, int len, const InternetAddress &address)
{
    return ::sendto(p_fd, data , len, 0, (struct sockaddr *)address.p.get(), sizeof(sockaddr_in));
}
int UdpServer::receive (char *data, int len,  InternetAddress *address)
{
    bzero(address->p.get(), sizeof(sockaddr_in));
    socklen_t shit = sizeof(sockaddr_in);
    return recvfrom(p_fd, data, len, 0, (struct sockaddr *)address->p.get(), &shit);
}


