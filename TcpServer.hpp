#ifndef KITE_TCP_SERVER_HPP
#define KITE_TCP_SERVER_HPP

#include "File.hpp"
#include "Internet.hpp"
#include "EventLoop.hpp"

namespace Kite {
    class TcpConnection : public Kite::File {
    public:
        TcpConnection(std::weak_ptr<EventLoop> ev, int fd)
            : Kite::File(ev)
        {
            setFile(fd);
        }
        ~TcpConnection() {}
    };



    class TcpServer : public Kite::Evented {
    public:
        typedef std::function<void(
                std::weak_ptr<Kite::EventLoop> ev,
                int fd,
                const InternetAddress &address)> Factory;

        TcpServer(std::weak_ptr<Kite::EventLoop> ev, Factory = Factory());
        ~TcpServer();

        bool listen (const InternetAddress &address);

    protected:
        virtual void onActivated(int, int) override final;
        virtual void onNewConnection(int fd, const InternetAddress &address);
    private:
        int p_fd;
        Factory factory;
    };
};

#endif
