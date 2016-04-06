#ifndef KITE_IO_HPP_SKMD
#define KITE_IO_HPP_SKMD

namespace Kite {
    class IO {
    public:
        virtual int read(char *buf, int len) = 0;
        virtual int write(const char *buf, int len) = 0;
        virtual inline bool getc(char &c) {return (read(&c, 1) == 1);}
    protected:
        virtual void onActivated (int) {}
    };
};

#endif
