#ifndef KITE_SERIALPORT_HPP_dd
#define KITE_SERIALPORT_HPP_dd

#include "IO.hpp"
#include "EventLoop.hpp"
#include <stdio.h>

namespace Kite {
    class Serialport: public Kite::Evented, public Kite::IO {
    public:
        Serialport(std::weak_ptr<EventLoop> ev);

        virtual int read(char *buf, int len);
        virtual int write(const char *buf, int len);

        bool open(const std::string &port, uint32_t speed);
        void close();
    private:
        int d_fd;
    };
};


#endif


