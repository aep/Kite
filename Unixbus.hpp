#ifndef KITE_UNIXBUS_HPP_sa
#define KITE_UNIXBUS_HPP_sa

#include "IO.hpp"
#include "EventLoop.hpp"
#include "File.hpp"

namespace Kite {
    class Unixbus : public Kite::File {
    public:
        Unixbus(std::weak_ptr<EventLoop> ev);
        ~Unixbus();

        static bool invoke(const std::string &path, const std::string &data);

        bool bind    (const std::string &path);
        bool connect (const std::string &path);
        virtual void close() override;

        virtual bool sendBusMessage(const std::string &data, int address = 0);
    protected:
        virtual void onBusClosed();
        virtual void onBusMessage(const std::string &data, int address);
        virtual void onActivated(int fd, int e) override final;
    };
};


#endif


