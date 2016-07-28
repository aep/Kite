#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "dessert/dessert.hpp"

#include "EventLoop.hpp"
#include "Process.hpp"
#include "Timer.hpp"

desserts("stdio loopback") {
    static std::string output;
    class Test: public Kite::Process
    {
    public:
        Test(std::weak_ptr<Kite::EventLoop> ev)
            : Kite::Process(ev)
        {
            popen("./stdio.exe");
            Kite::Timer::later(ev, [this](){
                    write("herp\n", 5);
                    return false;
                    }, 100);
            Kite::Timer::later(ev, [this](){
                    write("derp\n", 5);
                    return false;
                    }, 200);
        }

    protected:
        virtual void onReadActivated() override
        {
            char buf[1024];
            int len = read(buf, 1024);
            if (len < 1) return;
            std::cerr << std::string(buf,len) << std::endl;
            output += std::string(buf,len);
            if (output.size() == 10)
                closeWrite();
        }
        virtual void onClosing()
        {
            ev()->exit(0);
        }
    };
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Test>         ls(new Test(ev));

    ev->exec();
    dessert (output == "herp\nderp\n");
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
            if (len < 1) return;
            output += std::string(buf,len);
        }
        virtual void onClosing()
        {
            ev()->exit(0);
        }
    };
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<LS>         ls(new LS(ev));
    ev->exec();
    dessert (output == "123\nabc\n");
}

desserts("alot of forks") {

    static int counter = 0;
    class UselessProcess : public Kite::Process , public Kite::Scope
    {
    public:
        UselessProcess(std::weak_ptr<Kite::EventLoop> ev)
            : Kite::Process(ev)
        {
            popen("sleep 1");
            std::cerr << ++counter << " children spawned " << std::endl;
        }
        virtual void onClosing()
        {
            ev()->deleteLater(this);
            std::cerr << --counter << " children left " << std::endl;
            if (counter == 0) {
                ev()->exit(0);
            }
        }
    };
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    for (int i = 0 ; i < 200 ; i++) {
        new UselessProcess(ev);
    }
    ev->exec();
}

desserts("shell") {
    std::string output = Kite::Process::shell("ls fixtures/testdir/");
    dessert (output == "123\nabc\n");
}

int main(){}
