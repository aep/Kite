#ifndef KITE_FILE_HPP_SKMD
#define KITE_FILE_HPP_SKMD

#include "IO.hpp"
#include "EventLoop.hpp"
#include <stdio.h>

namespace Kite {
    class Process : public Kite::Evented, public Kite::IO {
    public:
        Process(std::weak_ptr<EventLoop> ev);

        void popen(const char *cmd, const char *ch);
        void close();

        virtual int read(char *buf, int len);
        virtual int write(const char *buf, int len);

    private:
        FILE *d_f;

    };
};


#endif


