#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "dessert/dessert.hpp"

#include "EventLoop.hpp"
#include "Process.hpp"

desserts("stdio loopback") {
    static std::string output;
    class Test: public Kite::Process
    {
    public:
        Test(std::weak_ptr<Kite::EventLoop> ev)
            : Kite::Process(ev)
        {
            popen("./stdio.exe");
            write("derp\n", 5);
        }

    protected:
        virtual void onReadActivated() override
        {
            char buf[1024];
            int len = read(buf, 1024);
            if (len == 0) {
                close();
                ev()->exit(0);
                return;
            }
            output += std::string(buf,len);
            closeWrite();
        }
    };
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Test>         ls(new Test(ev));

    ev->exec();
    dessert (output == "derp\n");
}

desserts("ls") {
    static std::string output;
    class LS: public Kite::Process
    {
    public:
        LS(std::weak_ptr<Kite::EventLoop> ev)
            : Kite::Process(ev)
        {
            popen("ls fixtures/testdir/");
        }

    protected:
        virtual void onReadActivated() override
        {
            char buf[1024];
            int len = read(buf, 1024);
            if (len == 0) {
                close();
                ev()->exit(0);
                return;
            }
            output += std::string(buf,len);
        }
    };
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<LS>         ls(new LS(ev));
    ev->exec();
    dessert (output == "123\nabc\n");
}

desserts("shell") {
    std::string output = Kite::Process::shell("ls fixtures/testdir/");
    dessert (output == "123\nabc\n");
}

int main(){}
