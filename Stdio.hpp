#ifndef KITE_STDIO_HPP_SKMD
#define KITE_STDIO_HPP_SKMD

#include "IO.hpp"
#include "EventLoop.hpp"
#include <stdio.h>

namespace Kite {
    class Stdio  : public Kite::Evented, public Kite::IO {
    public:
        Stdio(std::weak_ptr<EventLoop> ev);

        virtual int read(char *buf, int len);
        virtual int write(const char *buf, int len);
    };
};


#endif


