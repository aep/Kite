#ifndef KITE_FILE_HPP_SKMD
#define KITE_FILE_HPP_SKMD

#include "IO.hpp"
#include "EventLoop.hpp"
#include <stdio.h>

namespace Kite {
    class File : public Kite::Evented, public Kite::IO {
    public:
        File(std::weak_ptr<EventLoop> ev);
        virtual ~File();

        virtual int read(char *buf, int len);
        virtual int write(const char *buf, int len);

        virtual void close();
    protected:

        virtual void onClosing() {}

        void setFile(int fd);
        int d_fd;



    };
};


#endif


